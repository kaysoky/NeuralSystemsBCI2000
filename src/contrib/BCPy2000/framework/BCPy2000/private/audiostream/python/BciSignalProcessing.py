import numpy
import scipy.signal
import SigTools
import WavTools
import BCI2000Tools.DataFiles  # adds self.load and self.dump methods

class BciTrapSequence(SigTools.TrapSequence):
	
	def onreset(self, discard=None, remember=None, persistence=None, detrend=None, bci=None):
		if discard == None: discard = getattr(self, 'discard', 0)
		if remember == None: remember = getattr(self, 'remember', 10)
		if persistence == None: persistence = getattr(self, 'persistence', 1.0)
		if detrend == None: detrend = getattr(self, 'detrend', None)
		if bci == None: bci = getattr(self, 'bci', None)
				
		self.discard = discard
		self.remember = remember   # keep how many individual epochs in memory at any one time?
		self.persistence = persistence  # for running mean
		self.bci = bci
		self.detrend = detrend  # None,  'constant' or 'linear'

		self.avg = SigTools.running_mean(persistence=self.persistence)
		self.ndelivered = 0
		self.recent = []
		
	def oncollect(self, x, n):
		if self.detrend != None: x = scipy.signal.detrend(x, axis=1, type=self.detrend)		
		self.ndelivered += 1
		if self.ndelivered > self.discard: self.avg += x   # keep a running average
		self.recent.append(x)
		while len(self.recent) > self.remember: self.recent.pop(0)
		if self.bci != None: self.bci.UpdatePrediction()

#################################################################
#################################################################

class BciSignalProcessing(BciGenericSignalProcessing):	

	#############################################################

	def Description(self):
		return "binary classification of difference of L-R ERPs triggered by stereo AUX channels"

	#############################################################

	def Construct(self):
		parameters = [
			"PythonSig         int       CheckNumberOfEpochs=                      1                      1     0 1 // verify that the correct number of epochs has been trapped (boolean)",
			"PythonSig:Epoch   float     EpochDurationMsec=                      600                    600   100 % // ",
			"PythonSig:Epoch   floatlist EpochLowerBoundMsec=                2   100     100            100     0 % // after springing, each ERP trap will not spring again for this many milliseconds",
			"PythonSig:Epoch   list      TriggerChannels=                    2  LAUD    RAUD              %     % % // ",
			"PythonSig:Epoch   floatlist TriggerThreshold=                   2     0.1     0.1            %     0 % // ",
			"PythonSig:Epoch   int       PyAudioMicTrigger=                        0                      1     0 1 // replace trigger signal with pyaudio sound recording (boolean)",
			"PythonSig:Epoch   float     TriggerHPCutoff=                          0.0                    0.0   0 % // ",
			"PythonSig:Epoch   floatlist ERPFilterFreqHz=                    2     0.1     8              %     0 % // lower and upper frequencies of bandpass filter for ERP feature set",
			"PythonSig:Epoch   int       ERPFilterOrder=                           8                      8     0 % // order of bandpass filter for ERP feature set",
			"PythonSig:Epoch   int       DetrendEpochs=                            2                      2     0 2 // Detrend data? 0: no, 1: mean, 2: linear (enumeration)",
			"PythonSig:Epoch   float     ERPClassifierBias=                        0.0                    0.0   % % // ",
			"PythonSig:Epoch   matrix    ERPClassifierWeights=             0 0                            %     % % // ",
			"PythonSig:Epoch   int       DiffFeatureSets=                          1                      1     0 1 // for 2-stream designs, whether to use the difference between the two feature sets (boolean)",
			"PythonSig:Epoch   intlist   DiscardEpochs=                      2     2       2              2     0 % // for classification, discard this many epochs at the beginning",
			
			"PythonSig:Control float     EpochAveragingPersistence=                1.0                    1.0   0 % // persistence parameter for the running average of ERPs",
			"PythonSig:Control int       ContinuousOutput=                         0                      0     0 1 // continuous output rather than trial-based feedback at the end of a defined period (boolean)",
			"PythonSig:Control float     ControlFilterCutoffHz=                    0                      0     0 % // output low-pass cutoff in Hz (0 to disable)",
			"PythonSig:Control int       NumberOfStreams=                          2                      2     2 % // ",
		]
		states = [
		]
		return (parameters, states)

	#############################################################

	def Preflight(self, sigprops):
		self.eegfs = self.nominal['SamplesPerSecond']
		self.nstreams = self.params['NumberOfStreams'].val
		self.epoch_samples = SigTools.msec2samples(self.params['EpochDurationMsec'], self.eegfs)
		self.takediff = self.params['DiffFeatureSets'].val
		self.discard = self.params['DiscardEpochs'].val
		if self.takediff and self.nstreams != 2: raise EndUserError, "DiffFeatureSets parameter cannot be set unless NumberOfStreams is 2"
		
		band  = self.params['ERPFilterFreqHz'].val
		order = self.params['ERPFilterOrder'].val
		if len(band) == 0 or order == 0: self.bandpass = None
		else: self.bandpass = SigTools.causalfilter(type='bandpass', order=order, freq_hz=band, samplingfreq_hz=self.eegfs)
		
		trigHP = self.params['TriggerHPCutoff'].val
		if trigHP: self.triggerfilter = SigTools.causalfilter(type='highpass', order=4, freq_hz=trigHP, samplingfreq_hz=self.eegfs)
		else: self.triggerfilter = None
		
		# check length = number of streams
		for paramname in ['EpochLowerBoundMsec', 'TriggerChannels', 'TriggerThreshold', 'DiscardEpochs']:
			v = self.params[paramname].val
			if len(v) != self.nstreams: raise EndUserError, "number of elements of %s parameter must match NumberOfStreams (=%d)" % (paramname, self.nstreams)

		self.trapgap = [SigTools.msec2samples(x, self.eegfs) for x in self.params['EpochLowerBoundMsec'].val]
				
		chn = self.inchannels()
		trigch = self.params['TriggerChannels'].val
		if False in [isinstance(x, int) for x in trigch]:
			nf = filter(lambda x: not str(x) in chn, trigch) 
			if len(nf): raise EndUserError, "failed to find %s in module's list of input channel names" % str(nf)
			self.trigchan = [chn.index(str(x)) for x in trigch]
		else:
			nf = filter(lambda x: x < 1 or x > len(chn), trigch) 
			if len(nf): raise EndUserError, "illegal channel(s): %s" % str(nf)
			self.trigchan = [x-1 for x in trigch]
		
		self.otherchan = [chn.index(x) for x in ['VMRK'] if x in chn]
		self.sigchan = list(set(range(len(chn))).difference(self.trigchan + self.otherchan))
		self.trigthresh = numpy.asarray(self.params['TriggerThreshold'].val)
		
		#self.trigthresh *= float(self.params.get('SystemMasterVolume', 1.0))
		#if not int(self.params.get('SurroundSoundTrigger', 1)):
		#	if 'ChannelVolumesDB' in self.params:
		#		vc = 10.0**(numpy.array(self.params['ChannelVolumesDB'].val)/20.0)
		#		if vc.shape == self.trigthresh.shape:
		#			self.trigthresh[:] *= vc[:]
		#			print "scaled TriggerThreshold by " + ' '.join(["%.3g"%x for x in vc.flat]) + " according to ChannelVolumesDB"
		#		else:
		#			print "failed to use ChannelVolumesDB to scale TriggerThreshold"

		self.weights = self.params['ERPClassifierWeights'].val
		self.weights2 = numpy.multiply(self.weights, self.weights)
		if len(self.weights) == 0:
			self.weights = self.weights2 = None
		else:
			if self.weights.shape[0] != len(chn): raise EndUserError, "ERPClassifierWeights should have %d rows to match the number of channels" % len(chn)
			if self.weights.shape[1] != self.epoch_samples: raise EndUserError, "ERPClassifierWeights should have %d cols to match the number of samples in an epoch" % self.epoch_samples
		self.bias = float(self.params['ERPClassifierBias'])
		
		for istream in xrange(self.nstreams):
			sn = 'Stream%d'%(istream+1)
			if not sn in self.states: raise EndUserError, "state %s is not defined"%sn   # ZZZ
		
		if 1:  # turn back off in order to visualize temporally-filtered output
			self.out_signal_props['ChannelLabels']         =  ['prediction']
			self.out_signal_props['ElementLabels']         =  ['1']
			self.out_signal_props['ElementUnit']['Gain']   =  self.nominal['SecondsPerPacket']
			self.out_signal_props['ElementUnit']['RawMin'] =  0.0
			self.out_signal_props['ElementUnit']['RawMax'] =  self.nominal['PacketsPerSecond'] * 10.0 - 1.0
			self.out_signal_props['ValueUnit']['Gain']     =  1.0
			self.out_signal_props['ValueUnit']['RawMin']   = -2.0
			self.out_signal_props['ValueUnit']['RawMax']   = +2.0
		
		self.persistence = float(self.params['EpochAveragingPersistence'])
		self.continuous = int(self.params['ContinuousOutput'])
		ctrllp = float(self.params['ControlFilterCutoffHz'])
		if ctrllp: self.controlfilter = SigTools.causalfilter(type='lowpass', order=8, freq_hz=ctrllp, samplingfreq_hz=self.eegfs)
		else: self.controlfilter = None

		
	#############################################################

	def Initialize(self, indim, outdim):
		
		self.seq = []
		for istream in xrange(self.nstreams):
			s = BciTrapSequence(
					nsamp=self.epoch_samples,
					mingap=self.trapgap[istream],
					trigger_channel=self.trigchan[istream],
					trigger_threshold=self.trigthresh[istream],
					trigger_processing=self.ProcessTrigger,
					discard=self.discard[istream],
					remember=10,
					persistence=self.persistence,
					detrend={0:None, 1:'constant', 2:'linear'}.get(int(self.params['DetrendEpochs'])),
					bci=self,
			)
			self.seq.append(s)

		self.prediction, self.prediction_se = 0.0, 1.0
		
		self.Last10SecondsTrigger = SigTools.Buffering.trap(nsamples=SigTools.msec2samples(10000, self.eegfs), nchannels=2, leaky=True)
		self.LastPacketMic        = SigTools.Buffering.trap(nsamples=1+int(self.nominal.SecondsPerPacket*44100), nchannels=2, leaky=True)
		if int(self.params['PyAudioMicTrigger']):
			self.recorder = WavTools.recorder(seconds=0, fs=44100, nchan=2, callback=self.HandleMicData)
		else:
			self.recorder = None
			
		self.saving = int(self.params['CheckNumberOfEpochs']) != 0       # are we gathering and dumping preprocessed data?
		
	#############################################################
	
	def HandleMicData(self, packet):
		if WavTools.across_samples == 0: packet = packet.T
		self.LastPacketMic.process(numpy.abs(packet))
		
	#############################################################
	
	def StartRun(self):
		self.x = []
		self.y = []
		self.nbeats = [0] * self.nstreams
		self.streamstates = [0] * self.nstreams
		self.TriggerTrouble = [None] * self.nstreams
		
		if self.saving:
			self.dump(channels=list(self.inchannels()), fs=self.nominal['SamplesPerSecond'])
				
	#############################################################
	
	def StopRun(self):
		
		if self.saving:
			self.dump('flush') # the newer, python way
			# and now, the older, matlab way:
			if len(self.x) and self.x[0] != None:
				if not isinstance(self.data_file, str): raise RuntimeError,'StopRun failed in PythonSig because self.data_file is not valid'
				x = [numpy.matrix(numpy.asarray(xi).T.flatten()).A for xi in self.x]
				xsiz = numpy.matrix((len(x),) + self.x[0].shape, dtype=numpy.float64)
				unexpected = [y-1 not in range(self.nstreams) for y in self.y]
				if any(unexpected): print "WARNING: unexpected labels: self.y = ",self.y
				a = {
					'x':numpy.concatenate(x, axis=0),
					'xsiz':xsiz,
					'y':numpy.matrix(self.y).T,
					'channels':'\n'.join(self.inchannels()),
					'fs':self.eegfs,
				}
				SigTools.savemat(self.data_file.replace('.dat', '_features.mat'), a)
		else:
			print "features not saved (traps not verified)"
		
		p = getattr(self, 'player', None)
		if p: p.stop()
		
	#############################################################

	def Process(self, sig):
		
		if self.bandpass != None:
			sig[self.sigchan,:] = self.bandpass(sig[self.sigchan,:], axis=1)
		if self.triggerfilter != None:
			sig[self.trigchan,:] = self.triggerfilter(sig[self.trigchan,:], axis=1)				

		if self.recorder: # mic-trigger   # TODO: remove this
			sig[self.trigchan, :] = 0
			if self.recorder.going: # and self.LastPacketMic.full():
				micdata = self.LastPacketMic.read()
				prevsample = None
				for trigsample in range(sig.shape[1]):
					micstart = int(round( self.recorder.wav.fs * float(trigsample)   / self.eegfs ))
					micstop  = int(round( self.recorder.wav.fs * float(trigsample+1) / self.eegfs ))
					if micstart >= micdata.shape[1]: break
					sig[self.trigchan, -1-trigsample].flat = (numpy.mean( micdata[:, -1-micstop:-1-micstart]**2, axis=1 )**0.5).flat
			if self.changed('CueOn', only=1):   self.recorder.record()
			if self.changed('Stream1', only=0): self.recorder.stop()
			
		self.Last10SecondsTrigger.process(sig[self.trigchan,:])
					
		collected = False
		collecting = False
		for istream in xrange(self.nstreams):

			seq = self.seq[istream]
			if len(seq.active) > 0: collected=True
			
			statename = 'Stream%d'%(istream+1)
			streamstate = self.states[statename] # ZZZ require states called Stream1, Stream2 etc
			previous_streamstate,self.streamstates[istream] = self.streamstates[istream],streamstate
			if streamstate == 0: continue
			if previous_streamstate == 0:
				self.nbeats[istream] = 0
				self.seq[istream].reset() # ZZZ  reset when streamstate becomes non-zero
				
			if previous_streamstate <= 1 and streamstate > 1: self.nbeats[istream] += 1   # ZZZ increment nbeats when streamstate indicates it
			collecting = True # ZZZ only set collecting=True when streamstate != 0
			
			self.seq[istream].process(sig) # ZZZ only feed signal into trapsequence when streamstate != 0
		
		give_answer = False
		if collected and not collecting:  give_answer = True
			
		if self.continuous: give_answer = False
		
		if give_answer: # ZZZ for continuous (non-trial-based) control, disable this
			for istream in xrange(self.nstreams):
				seq = self.seq[istream]
				if seq.ndelivered != self.nbeats[istream]:
					self.TriggerTrouble[istream] = "expected %d beats in stream %d, but trapped %d" % (self.nbeats[istream], istream+1, seq.ndelivered)
				elif True in [t.collected() and not t.full() for t in seq.active]:
					self.TriggerTrouble[istream] = "not all traps are full in stream %d" % (istream+1)
				del self.seq[istream].active[:]
			
			trouble = '\n'.join( [''] + [x for x in self.TriggerTrouble if x != None] )
			if int( self.params['CheckNumberOfEpochs'] ):
				if len(trouble): raise RuntimeError(trouble)
			else:
				print 'warning: traps not verified, so features will not be saved' + trouble.replace('\n', '\n   ')

			
			xi = self.UpdatePrediction()
			yi = self.states['TargetStream']
			
			self.states['PredictedStream'] = {0:0, -1:1, +1:2}[int(numpy.sign(self.prediction))]

			self.dump(x=xi,y=yi) # the new python way
			self.x.append(xi) # the old matlab way
			self.y.append(yi) # the old matlab way
						
			
		if 'PredictedStream' in self.states and self.changed('PredictedStream',0):
			self.prediction, self.prediction_se = 0.0, 1.0
		if max(self.out_signal_dim) == 1:
			control_signal = self.prediction / self.prediction_se
			if self.continuous:
				if not collecting: control_signal = 0
				if self.controlfilter: control_signal = self.controlfilter.apply([[control_signal]])
				
			return control_signal
			
		return sig
		
	#############################################################
	
	def UpdatePrediction(self):

		nmin = min([s.avg.n for s in self.seq])
		if nmin < 1: return None

		#if nmin < 2: return None
		#estvar = [s.avg.v_unbiased / s.avg.n for s in self.seq]
		
		if self.takediff:
			xi = self.seq[1].avg.m - self.seq[0].avg.m
			#vi = estvar[1] + estvar[0]
		else:
			raise RuntimeError,"multiple feature sets not yet implemented"
		
		if self.weights == None:
			self.prediction = 0.0
			self.prediction_se = 1.0
		else:
			self.prediction    = sum(numpy.multiply(self.weights,  xi).flat) + self.bias
			#self.prediction_se = sum(numpy.multiply(self.weights2, vi).flat) ** 0.5
		
		return xi
	
	#############################################################

	def ProcessTrigger(self, tr):
		return numpy.abs(tr)

	#############################################################

	def foo(self, istream=0):
		"""
		To plot multiple epochs of the first channel (as cut up according to stream 0 triggers)  try this:
		    z = self.foo(istream=0)
		    z.plot(z.collated[0])  # channel index 0:  probably the stream-0 trigger itself.
		but don't expect the y tick-labels to make sense
		"""###
		from SigTools import sdict
		from BCI2000Tools.FileReader import bcistream
		b = bcistream(self.data_file)
		print b
		sig,states=b.decode('all')
		epochs = self.seq[istream].recent
		collated = []
		for i in xrange(epochs[0].shape[0]):
			collated.append(numpy.concatenate([numpy.asmatrix(x[i,:]) for x in epochs], axis=0))
		return sdict({'b':b, 'sig':sig, 'states':sdict(states), 'epochs':epochs, 'collated':collated, 'plot':b.plotsig})
	
	def plottrap(self, istream=0, itrap=-1):
		"""
		Plot all trigger channels in (by default) the last-trapped epoch, where an epoch is defined
		according to the trigger for stream <istream>.		
		To plot the last-but-one epoch, use itrap=-2,  and so on.  It's most sensible to use negative
		indices to work backwards from the end, since the number of epochs remembered at any one time
		is somewhat arbitrary.
		"""###
		delivered = self.seq[istream].recent
		active = [t.read() for t in self.seq[istream].active if t.collected()] # non-empty, undelivered
		y = (delivered + active)[itrap]
		
		y = y[self.trigchan, :].T
		t = SigTools.samples2msec(numpy.arange(y.shape[0]), self.eegfs)
		SigTools.plot(t, y)
		import pylab
		pylab.grid()
	
	def PlotTrigger(self, msg=None, stream=None):
		import pylab
		
		buffers = [self.Last10SecondsTrigger]
		for fig,buf in enumerate(buffers):
			pylab.figure(fig+1)
			for i,trig in enumerate(buf.read()):
				tt = float(self.params.TriggerThreshold[i])
				msec = SigTools.samples2msec(numpy.arange(trig.shape[-1]), self.eegfs)
				pylab.subplot(self.nstreams, 1, i+1)
				SigTools.plot(msec.T, trig.T)
				if self.TriggerTrouble[i]: pylab.title(self.TriggerTrouble[i])
				pylab.plot(pylab.xlim(), [tt,tt])
				pylab.plot(pylab.xlim(), [-tt,-tt])
				pylab.xlim([0, msec[-1]])
				pylab.grid('on')
			pylab.draw()
	
	#################################################################
	
	def classify(self, runs=None, xtn='.pk', C=(1e-0,1e-2,1e-4,1e-6), gamma=0.0, rebias=False, save=True, plotopt=False, **kwargs):
		from BCI2000Tools.Classification import ClassifyERPs
		files = self.find_data_files(xtn=xtn, runs=runs)
		u,c = ClassifyERPs(files, C=C, gamma=gamma, rebias=rebias, save=save, **kwargs)
		import SigTools
		u.channels = SigTools.ChannelSet(u.channels)
		
		print u.description
		return u,c
		
	#################################################################
	
	def stimulus(self, filename=None, pan=None):
		if getattr(self, 'player', None) == None:
			if filename == None: filename = 'sample_wavs/fixed_long_leftAttenuated12dB_1.wav'
				
		if filename != None:
			import WavTools
			self.player = WavTools.player(filename)
			self.player.set_preplay_hook( self.states.update, dict([('Stream%d'%(i+1), 1) for i in range(self.player.wav.channels())]))
			self.player.set_postplay_hook(self.states.update, dict([('Stream%d'%(i+1), 0) for i in range(self.player.wav.channels())]))
			print self.player.wav
			
		if pan != None: self.player.pan = pan
			
		return self.player
		
	#################################################################
	
	def play(self, *pargs, **kwargs):
		self.stimulus(*pargs, **kwargs).play(-1)
		
	#################################################################
	
	def stop(self):
		self.stimulus().stop()
		
#################################################################
#################################################################
