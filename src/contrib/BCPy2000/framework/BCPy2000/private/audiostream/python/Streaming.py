import os,sys
import time
import numpy
import scipy.signal # for detrending
import SigTools
import WavTools
import BCI2000Tools.DataFiles  # adds self.load and self.dump methods

def trapseq_onreset(self, discard=None, remember=None, persistence=None, detrend=None, bci=None):
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
	
def trapseq_oncollect(self, x, n):
	if self.detrend != None: x = scipy.signal.detrend(x, axis=1, type=self.detrend)		
	self.ndelivered += 1
	if self.ndelivered > self.discard: self.avg += x   # keep a running average
	self.recent.append(x)
	while len(self.recent) > self.remember: self.recent.pop(0)
	if self.bci != None: self.bci.UpdatePrediction()

class BciTrapSequence(SigTools.TrapSequence):
	onreset = trapseq_onreset
	oncollect = trapseq_oncollect

class BciTriggerlessTrapSequence(SigTools.TriggerlessTrapSequence):
	onreset = trapseq_onreset
	oncollect = trapseq_oncollect

#################################################################
#################################################################

class BciSignalProcessing(BciGenericSignalProcessing):	

	#############################################################

	def Description(self):
		return "binary classification of difference of L-R ERPs - self-contained streaming module $Id$"

	#############################################################

	def Construct(self):
		parameters = [
			"PythonSig:Streams int       NumberOfStreams=                          2                      2     2 % // ",
			"PythonSig:Streams matrix    StreamStimuli=                    2 { Standard Target } % % % %  %     % % // ",
			"PythonSig:Streams floatlist PeriodMsec=                         2   500   500              500     0 % // ",
			"PythonSig:Streams floatlist OffsetMsec=                         2     0   250              500     0 % // ",
			"PythonSig:Streams intlist   MinTargets=                   2     1     1                      1     0 % // ",
			"PythonSig:Streams intlist   MaxTargets=                   2     3     3                      3     0 % // ",
			"PythonSig:Streams intlist   ScopeForMinMax=               2     7     7                      7     1 % // ",
			"PythonSig:Streams intlist   InitialStandards=                   2     2       2              3     0 % // how many stimuli at the beginning of each stream are guaranteed to be standards",
			#"PythonSig:Streams int      SurroundSoundTrigger=                     0                      0     0 1 // if checked, deliver the trigger signal in sound channels 3 and 4 (boolean)", # removed in favour of configurable SoundChannels parameter
			"PythonSig:Streams matrix    SoundChannels=                4     3     1 % S1     % 1 S2     1 % F       % 1 F     %     % % // ",
			"PythonSig:Streams int       DirectSound=                              1                      0     0 1 // use DirectSound interface or not? (boolean)",
			"PythonSig:Streams floatlist StreamVolumes=                      2     1.0 1.0              1.0     0 1 // ",
			"PythonSig:Streams int       UseWiimotes=                              0                      0     0 1 // use Wiimote vibration instead of sound? (boolean)",
			
			"PythonSig:Epoch   float     EpochDurationMsec=                      600                    600   100 % // ",
			"PythonSig:Epoch   floatlist EpochLowerBoundMsec=                2   100     100            100     0 % // after springing, each ERP trap will not spring again for this many milliseconds",
			"PythonSig:Epoch   list      TriggerChannels=                    2  AUDL    AUDR              %     % % // ",
			"PythonSig:Epoch   floatlist TriggerThreshold=                   2     0.1     0.1            %     0 % // ",
			"PythonSig:Epoch   float     TriggerHPCutoff=                          0.0                    0.0   0 % // ",
			"PythonSig:Epoch   int       TriggerHPOrder=                           4                      4     0 % // ",
			"PythonSig:Epoch   float     TriggerlessOffsetMsec=                   50.0                 50.0     0 % // used to compensate for stimulus output latency to make weights from triggered and triggerless versions as compatible as possible",
			"PythonSig:Epoch   floatlist ERPFilterFreqHz=                    2     0.1     8              %     0 % // lower and upper frequencies of bandpass filter for ERP feature set",
			"PythonSig:Epoch   int       ERPFilterOrder=                           8                      8     0 % // order of bandpass filter for ERP feature set",
			"PythonSig:Epoch   intlist   DiscardEpochs=                      2     2       2              2     0 % // for classification, discard this many epochs at the beginning",
			"PythonSig:Epoch   int       DetrendEpochs=                            2                      2     0 2 // Detrend data? 0: no, 1: mean, 2: linear (enumeration)",
			"PythonSig:Epoch   float     ERPClassifierBias=                        0.0                    0.0   % % // ",
			"PythonSig:Epoch   matrix    ERPClassifierWeights=             0 0                            %     % % // ",
			"PythonSig:Epoch   int       DiffFeatureSets=                          1                      1     0 1 // for 2-stream designs, whether to use the difference between the two feature sets (boolean)",
			"PythonSig:Epoch   int       SaveTrainingData=                         0                      0     0 1 // dump epochs to .pk file (boolean)",
			
			"PythonSig:Control float     EpochAveragingPersistence=                1.0                    1.0   0 % // persistence parameter for the running average of ERPs",
			"PythonSig:Control float     ControlFilterCutoffHz=                    0                      0     0 % // output low-pass cutoff in Hz (0 to disable)",
			"PythonSig:Control int       ControlFilterOrder=                       8                      8     0 % // ",
			"PythonSig:Control float     NormalizerBufferSec=                      0.0                   60.0   0 % // length of buffer in which to remember non-zero control signal values",
			"PythonSig:Control float     NormalizerIntervalSec=                    0.25                   0.25  0 % // interval between informative samples in the normalizer buffer",
		]
		states = [
			"StreamingRequired  1 0 0 0",
			"StreamingFinished  1 0 0 0",
			"StimulusCode       8 0 0 0",
			"StimulusVariant    2 0 0 0",
			"StimulusType       1 0 0 0",
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
		
		trigch = self.params['TriggerChannels'].val
		use_trigger = len(trigch) != 0
		trigHPcutoff = self.params['TriggerHPCutoff'].val
		trigHPorder  = self.params['TriggerHPOrder'].val
		if use_trigger and trigHPorder and trigHPcutoff:
			self.triggerfilter = SigTools.causalfilter(type='highpass', order=trigHPorder, freq_hz=trigHPcutoff, samplingfreq_hz=self.eegfs)
		else:
			self.triggerfilter = None
		
		# check length = number of streams
		perStreamParams = [
			'EpochLowerBoundMsec', 'DiscardEpochs', 'PeriodMsec', 'OffsetMsec',
			'InitialStandards', 'MinTargets', 'MaxTargets', 'ScopeForMinMax',
			'StreamVolumes',
		]
		if use_trigger: perStreamParams += ['TriggerChannels', 'TriggerThreshold']
		for paramname in perStreamParams:
			v = self.params[paramname].val
			if len(v) != self.nstreams: raise EndUserError, "number of elements of %s parameter must match NumberOfStreams (=%d)" % (paramname, self.nstreams)

		self.trapgap = [SigTools.msec2samples(x, self.eegfs) for x in self.params['EpochLowerBoundMsec'].val]
		
		chn = self.inchannels()
		if False in [isinstance(x, int) for x in trigch]:
			nf = filter(lambda x: not str(x) in chn, trigch) 
			if len(nf): raise EndUserError, "failed to find %s in module's list of input channel names" % str(nf)
			self.trigchan = [chn.index(str(x)) for x in trigch]
		else:
			nf = [x for x in trigch if x < 1 or x > len(chn) or x != round(x)] 
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
		ctrllp = float(self.params['ControlFilterCutoffHz'])
		ctrlorder = int(self.params['ControlFilterOrder'])
		if ctrllp and ctrlorder:
			self.controlfilter = SigTools.causalfilter(type='lowpass', order=ctrlorder, freq_hz=ctrllp, samplingfreq_hz=self.eegfs)
		else:
			self.controlfilter = None
		

		self.offsets = []
		self.periods = []
		for istream in range(self.nstreams):
			self.periods.append( self.RoundToPackets('PeriodMsec', istream, minval=1) )
			self.offsets.append( self.RoundToPackets('OffsetMsec', istream, minval=0) )
			tmin = self.params['MinTargets'].val[istream]
			tmax = self.params['MaxTargets'].val[istream]
			bmax = self.params['ScopeForMinMax'].val[istream]
			istd = self.params['InitialStandards'].val[istream]
			for pname in ['MinTargets', 'MaxTargets', 'ScopeForMinMax', 'InitialStandards']:
				val = self.params[pname].val[istream]
				if val < 0.0 or val != round(val): raise EndUserError('%s values must be integers >= 0' % pname)
			if bmax < 1: raise EndUserError('ScopeForMinMax values cannot be less than 1')
			if istd > bmax: raise EndUserError('InitialStandards values cannot be larger than the corresponding ScopeForMinMax values')
			if tmin > bmax-istd: raise EndUserError('MinTargets value %d in stream #%d is too large: it should not exceed the corresponding value of ScopeForMinMax-InitialStandards = %d' % (tmin, istream+1, bmax-istd))
			if tmax > bmax-istd: raise EndUserError('MaxTargets value %d in stream #%d is too large: it should not exceed the corresponding value of ScopeForMinMax-InitialStandards = %d' % (tmax, istream+1, bmax-istd))
			if tmin > tmax: raise EndUserError('MinTargets value %d in stream #%d is larger than the corresponding MaxTargets value (%d)' % (tmin, istream+1, tmax))
			
		if int(self.params['DirectSound']):
			import DirectSoundInterface
		elif 'DirectSoundInterface' in sys.modules and sys.modules['DirectSoundInterface'].loaded:
			raise EndUserError, "once turned on, the DirectSound setting cannot be turned off without restarting BCI2000"
			
		sounds = []
		triggers = []
		scparam = self.params['SoundChannels']
		if len(scparam) < 2 or len(scparam[0]) < 3:
			raise EndUserError("SoundChannels must have at least 2 rows and at least 3 columns")
		nAudioChannels = len(scparam[0]) - 1
		self.soundmasks   = [[0]*nAudioChannels for istream in range(self.nstreams)]
		self.triggermasks = [[0]*nAudioChannels for istream in range(self.nstreams)]
		
		sstrings = ['S%d'%(istream+1) for istream in range(self.nstreams)]
		tstrings = ['T%d'%(istream+1) for istream in range(self.nstreams)]
		for row in self.params['SoundChannels']:
			code = row[-1].upper()
			try: row = [int({'':'0'}.get(x,x)) for x in row[:-1]]
			except: raise EndUserError("failed to interpret %s as floating-point numbers" % str(row))
			if code in sstrings:
				istream = int(code[1:])-1
				sounds.append(istream)
				self.soundmasks[istream] = row
			elif code in tstrings:
				istream = int(code[1:])-1
				triggers.append(istream)
				self.triggermasks[istream] = row
			elif code not in ['F', 'B']:
				raise EndUserError('unrecognized SoundChannels code "%s": legal values for a %d-stream system are F B %s'%(code, self.nstreams,' '.join(sstrings+tstrings)))
		self.surround = len(triggers) > 0
		if self.surround and sorted(triggers) != range(self.nstreams): raise EndUserError("if Tx values are specified in SoundChannels, all T1 through T%d must be specified without repetition"%self.nstreams)
		if sorted(sounds) != range(self.nstreams): raise EndUserError("last column of SoundChannels must contain all S1 through S%d without repetition"%self.nstreams)
		
		stim = numpy.array(self.params['StreamStimuli'])
		if stim.shape[1] == 1:
			if not self.surround: raise EndUserError("To use pre-prepared multichannel StreamStimuli, trigger outputs must be specified in SoundChannels")
			if len(self.trigchan) == 0:  raise EndUserError("To use pre-prepared multichannel StreamStimuli, TriggerChannels must be used")
			if max(self.params['MaxTargets'].val) > 0: raise EndUserError("To use pre-prepared multichannel StreamStimuli, MaxTargets must be 0 for all streams")
			self.precooked_wavs = [WavTools.player(x) for x in stim[:,0]]
			for p in self.precooked_wavs: p.play(vol=0); time.sleep(0.01)
			for p in self.precooked_wavs: p.stop(); p.vol = 1;
		else:
			self.precooked_wavs = None
			if stim.shape[1] != 2: raise EndUserError("StreamStimuli parameter must have 2 columns (Standard and Target)")
			if stim.shape[0] != self.nstreams: raise EndUserError("StreamStimuli parameter must have one row per streams (NumberOfStreams = %d)" % self.nstreams)
			self.standards = [self.prepwav(prmval, istream                      ) for istream,prmval in enumerate(stim[:,0])]
			self.targets   = [self.prepwav(prmval, istream, base=stim[istream,0]) for istream,prmval in enumerate(stim[:,1])]
			time.sleep(0.1)
			for p in self.standards + self.targets: p.stop(); p.vol = 1.0
		
	#############################################################
	
	def RoundToPackets(self, paramname, index=None, minval=0):
		
		msec = self.params[paramname].val
		desc = '%s parameter' % paramname
		if index != None:
			msec = msec[index];
			desc = 'element %d of %s' % (index+1, desc)
		packets = max(minval, SigTools.msec2samples(msec, self.nominal.PacketsPerSecond))
		msec_rounded = SigTools.samples2msec(packets, self.nominal.PacketsPerSecond)
		if abs(msec - msec_rounded) > 0.5: raise EndUserError("at a SampleBlock duration of %g msec, %s gets rounded from %g to %g. To remove this error, specify this as %g to begin with." % (1000.0*self.nominal.SecondsPerPacket, desc, msec, msec_rounded, round(msec_rounded)))
		return packets
		
	#############################################################
	
	def prepwav(self, prmval, istream, base=None):
		
		if base != None:
			amfreq = shift = None
			try:
				if prmval.lower().endswith('hz'): amfreq = float(prmval.lower().rstrip('hz'))
				else: shift = float(prmval.lower().rstrip('msec'))/1000.0
			except ValueError:
				base = None
			else:
				w = WavTools.wav(base)
				if shift != None: w += (shift % w)
				if amfreq != None: w = SigTools.ampmod(w, freq_hz=amfreq)
				
		if base == None: # no, don't turn this into an else
			try: w = WavTools.wav(prmval)
			except IOError: raise EndUserError("failed to load '%s' as a wav file" % prmval)
		
		self.normalizer = None
		nrmlen_sec = float(self.params['NormalizerBufferSec'])
		nrmlen_packets = round(nrmlen_sec * self.nominal.PacketsPerSecond)
		nrmskip_sec = float(self.params['NormalizerIntervalSec'])
		nrmskip_packets = round(nrmskip_sec * self.nominal.PacketsPerSecond)
		if nrmlen_sec:
			self.normalizer = SigTools.trap(nrmlen_packets, 1, leaky=True)
			self.normalizer_decimation = max(1, min(nrmlen_packets-1,  nrmskip_packets))
		
		if w.channels() != 1: raise EndUserError("StreamStimuli wav files must be single-channel: found %d channels in %s" % (w.channels(), w.filename))
			
		if int(self.params['UseWiimotes']):
			if self.surround: raise EndUserError("cannot use trigger channels when using wiimotes")
			import wiiplayer
			if wiiplayer.wiimote_ptrs == None:  wiiplayer.init(self.nstreams)
			return wiiplayer.wiiplayer(w, wiimote_index=istream)		# changed w -> prmval
		
		w.padendto(0.05) # causes some kind of stream-hanging/-silent-crashing problem on mac mini if wavs very short(??)
		w *= float(self.params['StreamVolumes'][istream])
		w *= self.soundmasks[istream]
		if self.surround:
			f = self.eegfs/4.0
			d = 100
			d = SigTools.samples2msec(max(1, SigTools.msec2samples(d, f)), f)
			w += SigTools.wavegen(freq_hz=f, duration_msec=d, container=w[:,0]*0, waveform=numpy.sin) * self.triggermasks[istream]
					
		p = WavTools.player(w)
		p.vol = 0.0
		p.play()
		return p
		
	#############################################################

	def Initialize(self, indim, outdim):
		
		self.seq = []
		for istream in xrange(self.nstreams):
			if len(self.trigchan):
				seqclass = BciTrapSequence
				kwargs = {
					'trigger_channel':self.trigchan[istream],
					'trigger_threshold':self.trigthresh[istream],
					'trigger_processing':self.ProcessTrigger,
				}
			else:
				self.extra_event_offset = SigTools.msec2samples(self.params['TriggerlessOffsetMsec'], self.eegfs)
				seqclass = BciTriggerlessTrapSequence
				kwargs = {}
			s = seqclass (
					nsamp=self.epoch_samples,
					mingap=self.trapgap[istream],
					discard=self.discard[istream],
					remember=10,
					persistence=self.persistence,
					detrend={0:None, 1:'constant', 2:'linear'}.get(int(self.params['DetrendEpochs'])),
					bci=self,
					**kwargs
			)
			self.seq.append(s)

		self.prediction, self.prediction_se = 0.0, 1.0
		
		self.Last10SecondsTrigger = SigTools.Buffering.trap(nsamples=SigTools.msec2samples(10000, self.eegfs), nchannels=2, leaky=True)
			
		self.saving = int(self.params['SaveTrainingData']) != 0       # are we gathering and dumping preprocessed data?
	
	#############################################################
		
	def StartRun(self):
		
		self.x = []
		self.y = []
		self.nbeats = [0] * self.nstreams
		self.TriggerTrouble = [None] * self.nstreams
		self.started = False
		
		if self.saving:
			self.dump(channels=list(self.inchannels()), fs=self.nominal['SamplesPerSecond'])
			
		self.precooked_playing = None
		self.precooked_counter = 0
				
	#############################################################
	
	def StopRun(self):
		
		if self.saving:
			self.dump('flush') # the newer, python way
			# and now, the older, matlab way (TODO: remove this)  :
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

	def GetVariant(self, istream):
		
		tmin = self.params['MinTargets'].val[istream]
		tmax = self.params['MaxTargets'].val[istream]
		batch = self.params['ScopeForMinMax'].val[istream]
		istd = self.params['InitialStandards'].val[istream]
		done = self.nbeats[istream]
		
		standard = 1
		target = 2
		
		if done < istd: return standard
		if done < batch: batch -= istd
		self.prepseq = getattr(self, 'prepseq', [[] for x in range(self.nstreams)])
		s = self.prepseq[istream]
		if len(s) == 0:
			ntargets = numpy.random.randint(tmin, tmax+1)
			s += [target] * ntargets + [standard] * (batch-ntargets)
			numpy.random.shuffle(s)
			
		return s.pop(0)
		
		
	#############################################################

	def Process(self, sig):
		
		# if len(self.trigchan): sig[:len(self.trigchan),:]=sig[self.trigchan,:]; self.debug('signal butcherings') # comment this out!! diagnostic purposes only
		
		if self.bandpass != None:
			sig[self.sigchan,:] = self.bandpass(sig[self.sigchan,:], axis=1)
		
		if len(self.trigchan):
			if self.triggerfilter != None:
				sig[self.trigchan,:] = self.triggerfilter(sig[self.trigchan,:], axis=1)				
			self.Last10SecondsTrigger.process(sig[self.trigchan,:])
		
		starting = self.changed('StreamingRequired', fromVals=0)
		presenting = self.states['StreamingRequired'] != 0
		if starting:
			self.started = True
			self.remember('streaming')
		resetStates = False
		stimDelivered = 0
		
		counts = []
		np = self.since('streaming')['packets']
		
		if self.precooked_wavs:
			if starting:
				self.precooked_playing = self.precooked_wavs[self.precooked_counter % len(self.precooked_wavs)]
				self.precooked_playing.play()
				self.precooked_counter += 1
			if not presenting and self.precooked_playing != None:
				if self.precooked_playing.going: self.precooked_playing.stop()
				self.precooked_playing = None
				
		for istream in range(self.nstreams):
									
			if starting:
				self.nbeats[istream] = 0
				
			if presenting:
				count = (np - self.offsets[istream]) % self.periods[istream]
				counts.append(count)
				if count == 0:
					if int(self.params.get('EnslavePython', 0)): variant = self.states['StimulusVariant']
					else: variant = self.GetVariant(istream)
					
					if self.precooked_wavs:
						stim = None
					elif variant == 2:
						stim = self.targets[istream]
					else:
						stim = self.standards[istream]
						
					if stim != None and stim.going:
						self.debug('stimuli skipped', counts=counts, packetsSinceStartOfStreaming=np, nbeats=list(self.nbeats))
					else:
						if stim != None: stim.play()
						self.nbeats[istream] += 1
						self.states['StimulusCode'] = istream + 1
						self.states['StimulusVariant'] = variant
						self.states['StimulusType'] = int( self.states.get('TargetStream', 0) == istream + 1 )
						stimDelivered += 1
						
				if count == 1:
					resetStates = True
					
		if (resetStates or not presenting) and not stimDelivered:
			self.states['StimulusCode'] = 0
			self.states['StimulusVariant'] = 0
			self.states['StimulusType'] = 0
			
		if stimDelivered > 1:
			self.debug("stimulus collisions", counts=counts, packetsSinceStartOfStreaming=np, nbeats=list(self.nbeats))


		stillWaiting = False
		
		for istream, seq in enumerate(self.seq):
			if starting: seq.reset()
				
			if len(self.trigchan) == 0 and stimDelivered==1 and counts[istream]==0:
				kwargs = {'event_offset':self.nominal.SamplesPerPacket + self.extra_event_offset} # TODO: this seems either non-stationarily or non-linearly related to the offset you can actually see using plottrap
			else:
				kwargs = {}
			waiting = seq.ndelivered < self.nbeats[istream]
			if presenting or waiting: seq.process(sig, **kwargs)
			waiting = seq.ndelivered < self.nbeats[istream]
			stillWaiting |= waiting
			
		if not self.started:
			self.prediction, self.prediction_se = 0.0, 1.0
				
		if self.started and not presenting and not stillWaiting:
			self.started = False
			self.states['StreamingFinished'] = 1
			for istream, seq in enumerate(self.seq):
				if seq.ndelivered != self.nbeats[istream]:
					self.TriggerTrouble[istream] = "expected %d beats in stream %d, but trapped %d" % (self.nbeats[istream], istream+1, seq.ndelivered)
				elif True in [t.collected() and not t.full() for t in seq.active]:
					self.TriggerTrouble[istream] = "not all traps are full in stream %d" % (istream+1)
				del self.seq[istream].active[:]
			
			trouble = '\n'.join( [''] + [x for x in self.TriggerTrouble if x != None] )
			if int( self.params['SaveTrainingData'] ):
				if len(trouble): raise RuntimeError(trouble)
			else:
				print 'warning: traps not verified, so features will not be saved' + trouble.replace('\n', '\n   ')
			
			xi = self.UpdatePrediction()
			yi = self.states.get('TargetStream', 0)
				
			if xi != None:
				self.dump(x=xi,y=yi) # the new python way
				self.x.append(xi) # the old matlab way
				self.y.append(yi) # the old matlab way
		else:
			self.states['StreamingFinished'] = 0
			
		control_signal = self.prediction / self.prediction_se
		if self.controlfilter: control_signal = self.controlfilter.apply([[control_signal]])
		
		if self.normalizer and control_signal != 0:
			self.normalizer.process(control_signal)
			history = numpy.abs( self.normalizer.read()[0][::self.normalizer_decimation] )
			history.sort()
			if history.size:
				mag = 1.5 * numpy.diff(history[int(round(0.5*(history.size-1)))])  # for iid samples ~N(0,1) this statistic is around 1.0 (+/- 12% if history contains 100 samples at a time)
				if mag: control_signal /= mag
		
		if max(self.out_signal_dim) == 1: return control_signal
		else: return sig
								
	#############################################################
	
	def UpdatePrediction(self):

		nmin = min([s.avg.n for s in self.seq])
		if nmin < 1: return None

		#if nmin < 2: return None
		#estvar = [s.avg.v_unbiased / s.avg.n for s in self.seq]
		
		if self.takediff and self.nstreams == 2:
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

	#################################################################
	
	def classify(self, runs=None, xtn='.pk', C=(1e+4,1e+2,1e-0,1e-2,1e-4,1e-6), gamma=0.0, rebias=False, save=True, plotopt=False, **kwargs):
		from BCI2000Tools.Classification import ClassifyERPs
		files = self.find_data_files(xtn=xtn, runs=runs)
		u,c = ClassifyERPs(files, C=C, gamma=gamma, rebias=rebias, save=save, **kwargs)
		import SigTools
		u.channels = SigTools.ChannelSet(u.channels)
		
		print u.description
		return u,c
		
	#############################################################

	def foo(self, istream=0):
		"""
		To plot multiple epochs of the first channel (read from the last .dat file and cut
		up according to stream 0 triggers)  try this:
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
	
	def plottrap(self, istream=0, itrap=-1, chans=None):
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
		
		if chans == None and len(self.trigchan): chans = self.trigchan
		if chans == None: chans = [0,1]
		y = y[chans, :].T
		t = SigTools.samples2msec(numpy.arange(y.shape[0]), self.eegfs)
		SigTools.plot(t, y)
		import pylab
		pylab.grid()
	
	def PlotTrigger(self, msg=None, stream=None):
		"""
		Plot the last 10 seconds' worth of (possibly filtered) uncut trigger signals,
		together with the current trigger thresholds.
		"""###
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
			
	#############################################################
	
	def play(self, channels='left', reps=None):
		if self.states['Running']: print "not now"; return
		if isinstance(channels, basestring):
			if channels == 'both': channels = 'left+right'
			channels = [c for c in channels.replace(',',' ').replace('&',' ').replace('+',' ').split() if len(c)]
		if not isinstance(channels, (tuple,list)): channels = [channels]
		ww = 0.0
		self.nbeats = [0] * self.nstreams
		for c in channels:
			if c == '': continue
			if isinstance(c, basestring):
				streamstring = c
				c = {'left':1, 'right':2}[c.lower()]
			else:
				streamstring = 'stream %d' % c
			istream = c - 1
			w = float(self.params['OffsetMsec'][istream]) / 1000.0
			period = float(self.params['PeriodMsec'][istream]) / 1000.0
			ntargets = 0
			standard = self.standards[istream].wav.copy(); standard.padendto(period)
			target = self.targets[istream].wav.copy(); target.padendto(period)
			nbmax = reps
			if nbmax == None:
				if 'BeatsPerTrial' in self.params: nbmax = int((self.params['BeatsPerTrial']*self.nstreams)[istream])
				else:                              nbmax = int((self.params['ScopeForMinMax']*self.nstreams)[istream])
			while self.nbeats[istream] < nbmax:
				v = self.GetVariant(istream)
				if v == 2: w = w % target; ntargets += 1
				else: w = w % standard
				self.nbeats[istream] += 1
			print "%d targets in %s" % (ntargets, streamstring)
			ww = ww + w
		del self.prepseq
		self.nbeats = [0] * self.nstreams
		WavTools.player(ww).play(bg=False)
		
#################################################################
#################################################################
