import time
import numpy
import scipy.signal # for detrending
import SigTools
import WavTools

from BCPy2000.GenericSignalProcessing import BciGenericSignalProcessing
from BCPy2000.GenericApplication import BciGenericApplication

def trapseq_onreset(self, discard=None, remember=None, persistence=None, detrend=None, bci=None, streamnumber=None):
	if discard == None: discard = getattr(self, 'discard', 0)
	if remember == None: remember = getattr(self, 'remember', 10)
	if persistence == None: persistence = getattr(self, 'persistence', 1.0)
	if detrend == None: detrend = getattr(self, 'detrend', None)
	if bci == None: bci = getattr(self, 'bci', None)
	if streamnumber == None: streamnumber = getattr(self, 'streamnumber', 0)
			
	self.discard = discard
	self.remember = remember   # keep how many individual epochs in memory at any one time?
	self.persistence = persistence  # for running mean
	self.bci = bci
	self.detrend = detrend  # None,  'constant' or 'linear'
	self.streamnumber = streamnumber

	self.avg = SigTools.running_mean(persistence=self.persistence)
	self.ndelivered = 0
	self.recent = []
	
def trapseq_oncollect(self, x, n):
	if self.detrend != None: x = scipy.signal.detrend(x, axis=1, type=self.detrend)		
	self.ndelivered += 1
	if self.ndelivered > self.discard: self.avg += x   # keep a running average
	self.recent.append(x)
	while len(self.recent) > self.remember: self.recent.pop(0)

	if self.bci != None: self.bci.DoSomethingWith(x, self.streamnumber)

class MyTrapSequence(SigTools.TrapSequence):
	onreset = trapseq_onreset
	oncollect = trapseq_oncollect

class MyTriggerlessTrapSequence(SigTools.TriggerlessTrapSequence):
	onreset = trapseq_onreset
	oncollect = trapseq_oncollect

#################################################################
#################################################################

class BciSignalProcessing(BciGenericSignalProcessing):	

	#############################################################

	def Construct(self):
			
		self.define_param( 'PythonSig:Epoch   int       NumberOfStreams=                          2                      2     1 % // ' )
		self.define_param( 'PythonSig:Epoch   float     EpochDurationMsec=                      800                    600   100 % // ' )
		self.define_param( 'PythonSig:Epoch   floatlist EpochLowerBoundMsec=                2   600     600            100     0 % // after springing, each ERP trap will not spring again for this many milliseconds' )
		self.define_param( 'PythonSig:Epoch   list      TriggerChannels=                    2     L       R              %     % % // ' )
		self.define_param( 'PythonSig:Epoch   floatlist TriggerThreshold=                   2    10      10              %     0 % // ' )
		self.define_param( 'PythonSig:Epoch   float     TriggerHPCutoff=                          0.0                    0.0   0 % // ' )
		self.define_param( 'PythonSig:Epoch   int       TriggerHPOrder=                           4                      4     0 % // ' )
		self.define_param( 'PythonSig:Epoch   float     TriggerlessOffsetMsec=                   50.0                 50.0     0 % // used to compensate for stimulus output latency to make weights from triggered and triggerless versions as compatible as possible' )
		self.define_param( 'PythonSig:Epoch   floatlist ERPFilterFreqHz=                    2     0.1     8              %     0 % // lower and upper frequencies of bandpass filter for ERP feature set' )
		self.define_param( 'PythonSig:Epoch   int       ERPFilterOrder=                           8                      8     0 % // order of bandpass filter for ERP feature set' )
		self.define_param( 'PythonSig:Epoch   intlist   DiscardEpochs=                      2     2       2              2     0 % // for classification, discard this many epochs at the beginning' )
		self.define_param( 'PythonSig:Epoch   int       DetrendEpochs=                            2                      2     0 2 // Detrend data? 0: no, 1: mean, 2: linear (enumeration)' )
			
		self.define_param( 'PythonSig:Control float     EpochAveragingPersistence=                1.0                    1.0   0 % // persistence parameter for the running average of ERPs' )

	#############################################################

	def Preflight(self, sigprops):
		self.eegfs = self.nominal['SamplesPerSecond']
		self.nstreams = self.params['NumberOfStreams'].val
		self.epoch_samples = SigTools.msec2samples(self.params['EpochDurationMsec'], self.eegfs)
		self.discard = self.params['DiscardEpochs'].val
		
		band  = self.params['ERPFilterFreqHz'].val
		order = self.params['ERPFilterOrder'].val
		if len(band) == 0 or order == 0: self.bandpass = None
		else: self.bandpass = SigTools.causalfilter(type='bandpass', order=order, freq_hz=band, samplingfreq_hz=self.eegfs)
		
		trigch = self.params['TriggerChannels'].val
		use_trigger = len(trigch) != 0
		trigHPcutoff = self.params['TriggerHPCutoff'].val
		trigHPorder  = self.params['TriggerHPOrder'].val
		if use_trigger and 'Trigger' not in self.states: raise EndUserError('application must define a Trigger state unless TriggerChannels are used')
		if use_trigger and trigHPorder and trigHPcutoff:
			self.triggerfilter = SigTools.causalfilter(type='highpass', order=trigHPorder, freq_hz=trigHPcutoff, samplingfreq_hz=self.eegfs)
		else:
			self.triggerfilter = None
		
		# check length = number of streams
		perStreamParams = [
			'EpochLowerBoundMsec', 'DiscardEpochs',
		]
		if use_trigger: perStreamParams += ['TriggerChannels', 'TriggerThreshold']
		for paramname in perStreamParams:
			v = self.params[paramname].val
			if len(v) != self.nstreams: raise EndUserError, 'number of elements of %s parameter must match NumberOfStreams (=%d)' % (paramname, self.nstreams)

		self.trapgap = [SigTools.msec2samples(x, self.eegfs) for x in self.params['EpochLowerBoundMsec'].val]
		
		chn = self.inchannels()
		if False in [isinstance(x, int) for x in trigch]:
			nf = filter(lambda x: not str(x) in chn, trigch) 
			if len(nf): raise EndUserError, "failed to find %s in module's list of input channel names" % str(nf)
			self.trigchan = [chn.index(str(x)) for x in trigch]
		else:
			nf = [x for x in trigch if x < 1 or x > len(chn) or x != round(x)] 
			if len(nf): raise EndUserError, 'illegal channel(s): %s' % str(nf)
			self.trigchan = [x-1 for x in trigch]
						
		self.otherchan = [chn.index(x) for x in ['VMRK'] if x in chn]
		self.sigchan = list(set(range(len(chn))).difference(self.trigchan + self.otherchan))
		self.trigthresh = numpy.asarray(self.params['TriggerThreshold'].val)
				
		self.persistence = float(self.params['EpochAveragingPersistence'])
			
				
	#############################################################

	def Initialize(self, indim, outdim):
		
		self.seq = []  # an array for trapsequence objects, one trapsequence per stream
		
		for istream in xrange(self.nstreams):
		
			if len(self.trigchan):
				seqclass = MyTrapSequence
				kwargs = {
					'trigger_channel':    self.trigchan[istream],
					'trigger_threshold':  self.trigthresh[istream],
					'trigger_processing': self.ProcessTrigger,
				}
			else:
				self.extra_event_offset = SigTools.msec2samples(self.params['TriggerlessOffsetMsec'], self.eegfs)
				seqclass = MyTriggerlessTrapSequence
				kwargs = {}
				
				
			s = seqclass (
					nsamp=self.epoch_samples,
					mingap=self.trapgap[istream],
					discard=self.discard[istream],
					streamnumber=istream+1,
					remember=10,
					persistence=self.persistence,
					detrend={0:None, 1:'constant', 2:'linear'}.get(int(self.params['DetrendEpochs'])),
					bci=self,
					**kwargs
			)
			
			self.seq.append(s)
		
		if len(self.trigchan):
			self.Last10SecondsTrigger = SigTools.Buffering.trap(nsamples=SigTools.msec2samples(10000, self.eegfs), nchannels=len(self.trigchan), leaky=True)
					
	#############################################################
	
	def StartRun(self):
	
		for seq in self.seq:  seq.reset()
		#if len(self.trigchan): self.Last10SecondsTrigger.reset()
	
	#############################################################

	def ProcessTrigger(self, tr):
		return numpy.abs(tr)
		
	#############################################################

	def Process(self, sig):
		
		if self.bandpass != None:
			sig[self.sigchan,:] = self.bandpass(sig[self.sigchan,:], axis=1)
		
		if len(self.trigchan):
			if self.triggerfilter != None:
				sig[self.trigchan,:] = self.triggerfilter(sig[self.trigchan,:], axis=1)				
			self.Last10SecondsTrigger.process(sig[self.trigchan,:])
				
			
		for istream,seq in enumerate(self.seq):

			kwargs = {}
			if len(self.trigchan) == 0 and self.changed('Trigger', only=istream+1):
				# some extra fiddling for software-only triggering
				eo = self.detect_event()  # decodes the EventOffset state
				if eo == None:
					self.debug('Trigger without EventOffset')
					eo = self.nominal.SamplesPerPacket
				eo += self.extra_event_offset  # TODO: the necessary "extra" correction seems either non-stationarily or non-linearly related to the offset you can actually see using plottrap; dependent on SamplingRate and SampleBlockSize
				kwargs = { 'event_offset':eo } 
				
			seq.process(sig, **kwargs)
			
		return sig
								
	#############################################################
	
	def DoSomethingWith(self, x, streamnumber):

		print 'got an epoch relevant to stream %d' % streamnumber
	
		
	#############################################################

	def PlotTrap(self, istream=0, itrap=-1, chans=None):
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
	
	#############################################################

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
				pylab.plot(pylab.xlim(), [tt,tt])
				pylab.plot(pylab.xlim(), [-tt,-tt])
				pylab.xlim([0, msec[-1]])
				pylab.grid('on')
			pylab.draw()
			
#################################################################
#################################################################

class BciApplication(BciGenericApplication):	

	#############################################################

	def Construct(self):
	
		self.define_param( 'PythonApp:Stimuli int       DirectSound=                              1                      0     0 1 // use DirectSound interface or not? (boolean)' )
		self.define_param( 'PythonApp:Stimuli floatlist SOARangeMsec=                       2  1000 2000                 0   500 % // ' )

		self.define_state( 'Trigger 2 0 0 0' )		
		
	#############################################################
	
	def Preflight(self, inprops):
	
		if int(self.params['DirectSound']):
			import DirectSoundInterface
		elif 'DirectSoundInterface' in sys.modules and sys.modules['DirectSoundInterface'].loaded:
			raise EndUserError, 'once turned on, the DirectSound setting cannot be turned off without restarting BCI2000'
	
		self.bing = WavTools.player('bing.wav')
		
		self.transient( 'Trigger' )
		
	#############################################################
	
	def Phases(self):
		
		soaRange = numpy.array(self.params.SOARangeMsec.val)
		soa = numpy.random.rand() * soaRange.ptp() + soaRange.min()
		
		self.phase(  duration=2000, name='leadin',    next='bing',  )
		self.phase(  duration=soa,  name='bing',      next='bing',  )
		
		self.design(start='leadin', new_trial='bing')
	
	#############################################################
	
	def Transition(self, phase):
	
		if phase in ['bing'] and not self.bing.playing:
			self.bing.play()
			self.states.Trigger = 1
			
	
#################################################################
#################################################################
