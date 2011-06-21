import numpy
import SigTools

class BciTrapSequence(SigTools.TrapSequence):
	
	def onreset(self, discard=None, remember=None, persistence=None, bci=None):
		if discard == None: discard = getattr(self, 'discard', 0)
		if remember == None: remember = getattr(self, 'remember', 10)
		if persistence == None: persistence = getattr(self, 'persistence', 1.0)
		if bci == None: bci = getattr(self, 'bci', None)
		
		self.discard = discard
		self.remember = remember
		self.persistence = persistence
		self.bci = bci

		self.avg = SigTools.running_mean(persistence=self.persistence)
		self.ndelivered = 0
		self.recent = []
		
	def oncollect(self, x, n):
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
			"PythonSig:Epoch   float     TriggerHPCutoff=                          0.0                    0.0   0 % // ",
			"PythonSig:Epoch   floatlist ERPFilterFreqHz=                    2     0.1     8              %     0 % // lower and upper frequencies of bandpass filter for ERP feature set",
			"PythonSig:Epoch   int       ERPFilterOrder=                           8                      8     0 % // order of bandpass filter for ERP feature set",
			"PythonSig:Epoch   float     ERPClassifierBias=                        0.0                    0.0   % % // ",
			"PythonSig:Epoch   matrix    ERPClassifierWeights=             0 0                            %     % % // ",
			"PythonSig:Epoch   int       DiffFeatureSets=                          1                      1     0 1 // for 2-stream designs, whether to use the difference between the two feature sets (boolean)",
			"PythonSig:Epoch   intlist   DiscardEpochs=                      2     2       2              2     0 % // for classification, discard this many epochs at the beginning",
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
			if not sn in self.states: raise EndUserError, "state %s is not defined"%sn
		
		if 1:  # turn back off in order to visualize temporally-filtered output
			self.out_signal_props['ChannelLabels']         =  ['prediction']
			self.out_signal_props['ElementLabels']         =  ['1']
			self.out_signal_props['ElementUnit']['Gain']   =  self.nominal['SecondsPerPacket']
			self.out_signal_props['ElementUnit']['RawMin'] =  0.0
			self.out_signal_props['ElementUnit']['RawMax'] =  self.nominal['PacketsPerSecond'] * 10.0 - 1.0
			self.out_signal_props['ValueUnit']['Gain']     =  1.0
			self.out_signal_props['ValueUnit']['RawMin']   = -2.0
			self.out_signal_props['ValueUnit']['RawMax']   = +2.0
		
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
					persistence=1.0,
					bci=self,
			)
			self.seq.append(s)

		self.prediction, self.prediction_se = 0.0, 1.0

	#############################################################
	
	def StartRun(self):
		self.x = []
		self.y = []
		self.nbeats = [0] * self.nstreams
		self.streamstates = [0] * self.nstreams
		
	#############################################################
	
	def StopRun(self):
		if int(self.params['CheckNumberOfEpochs']) == 0:
			print "features not saved (traps not verified)"
		elif len(self.x):
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

	#############################################################

	def Process(self, sig):
		
		if self.bandpass != None:
			sig[self.sigchan,:] = self.bandpass(sig[self.sigchan,:], axis=1)
		if self.triggerfilter != None:
			sig[self.trigchan,:] = self.triggerfilter(sig[self.trigchan,:], axis=1)
		
		
		collected = False
		collecting = False
		for istream in xrange(self.nstreams):

			seq = self.seq[istream]
			if len(seq.active) > 0: collected=True
			
			statename = 'Stream%d'%(istream+1)
			streamstate = self.states[statename]
			previous_streamstate,self.streamstates[istream] = self.streamstates[istream],streamstate
			if streamstate == 0: continue
			if previous_streamstate == 0:
				self.nbeats[istream] = 0
				self.seq[istream].reset()
				
			if previous_streamstate <= 1 and streamstate > 1: self.nbeats[istream] += 1 
			collecting = True
			
			self.seq[istream].process(sig)
		
		give_answer = False
		if collected and not collecting:  give_answer = True

		if give_answer:
			for istream in xrange(self.nstreams):
				seq = self.seq[istream]
				if int(self.params['CheckNumberOfEpochs']):
					if seq.ndelivered != self.nbeats[istream]:
						raise RuntimeError, "expected %d beats in stream %d, but trapped %d" % (self.nbeats[istream], istream+1, seq.ndelivered)
					if True in [t.collected() and not t.full() for t in seq.active]:
						raise RuntimeError, "not all traps are full"
				else:
					print "warning: traps not verified"
				del self.seq[istream].active[:]
			
			xi = self.UpdatePrediction()
			yi = self.states['TargetStream']
			
			self.states['PredictedStream'] = {0:0, -1:1, +1:2}[int(numpy.sign(self.prediction))]

			self.x.append(xi)
			self.y.append(yi)
				
		if self.changed('PredictedStream',0): self.prediction, self.prediction_se = 0.0, 1.0
		if max(self.out_signal_dim) == 1: return self.prediction / self.prediction_se
		else: return sig
		
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
		
#################################################################
#################################################################
