import numpy
import SigTools
import BCI2000Tools.DataFiles  # adds self.load and self.dump methods
import scipy.signal # for detrend

#################################################################
#################################################################

# Let's create some SigTools.TrapSequence and SigTools.TriggerlessTrapSequence subclasses.
# Customize their callbacks to talk to the BciSignalProcessing object, dumping epoched
# data and timestamp into the latter's .trapped attribute

def trapseq_onreset(self, bci=None):
	if bci != None: self.bci = bci

def trapseq_oncollect(self, x, n):
	self.bci.trapped.append((x,n))

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
		return "signal-processing for the pyspeller"

	#############################################################

	def Construct(self):
		self.require_version(13980)

		parameters = [
			"PythonSig:Epoch    int       CheckNumberOfEpochs=                   1                   1     0 1 // verify that the correct number of epochs has been trapped (boolean)",
			"PythonSig:Epoch    float     EpochDurationMsec=                   600                 600   100 % // ",
			"PythonSig:Epoch    float     EpochLowerBoundMsec=                 100                 100     0 % // after springing, each ERP trap will not spring again for this many milliseconds",
			"PythonSig:Epoch    string    TriggerChannel=                        %                   %     % % // name of, or index to, the optical sync channel (leave blank if you dare to run without one)",
			"PythonSig:Epoch    float     TriggerThreshold=                      2                 0.1     0 % // ",
			"PythonSig:Epoch    float     TriggerHPCutoff=                       0.0               0.0     0 % // ",
			"PythonSig:Epoch    floatlist ERPFilterFreqHz=                 2     0.1   8             %     0 % // lower and upper frequencies of bandpass filter for ERP feature set",
			"PythonSig:Epoch    int       ERPFilterOrder=                        6                   6     0 % // order of bandpass filter for ERP feature set",
			"PythonSig:Epoch    int       Detrend=                               1                   1     0 1 // detrend each epoch? (boolean)",
			"PythonSig:Epoch    matrix    ERPClassifierWeights=          0 0                         %     % % // classifier weight vector, formatted as a channels-by-samples matrix",
			"PythonSig:Epoch    float     ERPClassifierBias=                     0.0                 0.0   % % // classifier bias",
		]
		states = [
			"Ready           1  0 0 0",
			# NB: we also require the Epoch state from application
		]
		return (parameters, states)

	#############################################################

	def Preflight(self, sigprops):
		
		if 'Epoch' not in self.states: raise 'required "Epoch" state is missing (should be defined by Application module)'

		# initialize some useful constants
		self.eegfs = self.nominal['SamplesPerSecond']
		self.epoch_samples    = SigTools.msec2samples(self.params['EpochDurationMsec'],   self.eegfs) # samples per epoch
		self.trapgap          = SigTools.msec2samples(self.params['EpochLowerBoundMsec'], self.eegfs) # min. num samples gap between 2 traps
		
		# set up the bandpass filter
		band  = self.params['ERPFilterFreqHz'].val
		order = self.params['ERPFilterOrder'].val
		if len(band) == 0 or order == 0: self.bandpass = None
		else: self.bandpass = SigTools.causalfilter(type='bandpass', order=order, freq_hz=band, samplingfreq_hz=self.eegfs)

		# if required, set up a high pass filter for the trigger channel (if any)
		trigHP = self.params['TriggerHPCutoff'].val
		if trigHP: self.triggerfilter = SigTools.causalfilter(type='highpass', order=4, freq_hz=trigHP, samplingfreq_hz=self.eegfs)
		else: self.triggerfilter = None

		# set some boolean flags:
		self.detrend = int(self.params['Detrend'])                       # detrend each epoch or not?
		self.copyspelling = len(self.params.get('TextToSpell', '')) > 0  # are we copy-spelling or free-spelling?
		self.saving = int(self.params['CheckNumberOfEpochs']) != 0       # are we gathering and dumping preprocessed data?
		self.soundtest = int(self.params['SoundTest']) != 0              # are we using the soundcard to make fake "ERPs"?
				
		# set up 3 lists of indices corresponding to 3 different channel types:  self.sigchan, self.trigchan and self.otherchan
		chn = self.inchannels()
		trigch = self.params['TriggerChannel'].val
		if isinstance(trigch, int):
			if trigch < 1 or trigch > len(chn): raise EndUserError('illegal TriggerChannel index %d' % trigch)
			self.trigchan = trigch - 1
		elif trigch == '':
			self.trigchan = None
		else:
			if str(trigch) not in chn: raise EndUserError('TriggerChannel "%s" was not found in the list of input channel names' % trigch)
			self.trigchan = chn.index(str(trigch))
		self.otherchan = [chn.index(x) for x in ['VMRK'] if x in chn]
		self.sigchan = list(set(range(len(chn))).difference([self.trigchan] + self.otherchan))

		# in addition to knowing the trigger channel index, we need to know the threshold to set on that channel
		self.trigthresh = float(self.params['TriggerThreshold'])

		# set up classifier parameters
		self.weights = self.params['ERPClassifierWeights'].val
		if len(self.weights) == 0 or (self.weights==0).all():  # empty weights matrix or all-zero weights matrix means: perform no online classification
			self.weights = None
		else:
			if self.weights.shape[0] != len(chn): raise EndUserError, "ERPClassifierWeights should have %d rows to match the number of channels" % len(chn)
			if self.weights.shape[1] != self.epoch_samples: raise EndUserError, "ERPClassifierWeights should have %d cols to match the number of samples in an epoch" % self.epoch_samples
		self.bias = float(self.params['ERPClassifierBias'])
		
		# set the output packet dimensions / annotations
		self.out_signal_props['ChannelLabels'] = ['probability', 'msec']
		self.out_signal_props['ElementLabels'] = ['1']
				
	#############################################################

	def Initialize(self, indim, outdim):
		
		# set up a TrapSequence or TriggerlessTrapSequence object for gathering data epochs
		if self.trigchan == None:
			lookback_samples = SigTools.msec2samples(1000.0, self.eegfs)
			self.seq = BciTriggerlessTrapSequence(nsamp=self.epoch_samples, mingap=self.trapgap, lookback_samples=lookback_samples, bci=self)
		else:
			self.seq = BciTrapSequence(nsamp=self.epoch_samples, mingap=self.trapgap, trigger_channel=self.trigchan, trigger_threshold=self.trigthresh, bci=self)
				
	#############################################################

	def Halt(self):
		seq = getattr(self, 'seq', None)
		if seq != None: seq.bci = None
		
	#############################################################
	
	def StartRun(self):
		self.eo = []             # a list of event offsets, for debugging
		self.sample0 = 0         # memory of the sample-index-within-run at which the first epoch of a trial arrived
		self.nEpochsExpected = 0 # incremented each time the 'Epoch' state changes to a non-zero value
		self.nEpochsTrapped = 0  # incremented each time an epoch is recovered from self.trapped, where it has been dumped by our TriggerlessTrapSequence.oncollect() callback
		self.seq.reset()
		if self.bandpass != None: self.bandpass.reset()
		if self.saving:
			chnames = list(self.inchannels())
			if int(self.params.get('SoundTest', 0)):
				if   'AUDL' in chnames: chnames[chnames.index('AUDL')] = 'SOUNDTEST'
				elif 'LAUD' in chnames: chnames[chnames.index('LAUD')] = 'SOUNDTEST'
			self.dump(channels=chnames, fs=self.eegfs)
		
	#############################################################
	
	def StopRun(self):
		self.CheckEpochs()
		if self.saving: self.dump('flush')

	#############################################################

	def Process(self, sig):
		
		if self.changed('CurrentTrial', ignore=0): self.CheckEpochs()
			
		if self.changed('Epoch', ignore=0):
			samplenumber = self.packet_count * self.nominal['SamplesPerPacket']
			epoch = self.states.get('Epoch', 0)          # the Epoch state counts the epochs since the beginning of the trial
			if epoch == 1: self.sample0 = samplenumber   # self.sample0 is now the timestamp of the first epoch of the trial, measured as a number of samples since the beginning of the run
			self.nEpochsExpected += 1
			if self.saving:
				y = self.states.get('TargetBitValue', 0)
				y = {0:0, 1:1, 2:-1}[y]
				yt = (self.states.get('CurrentBlock', 0), self.states.get('CurrentTrial', 0), epoch, samplenumber-self.sample0)
				self.dump(y=y, yt=yt)                    # y is the target value of the epoch: -1 for should-be-unattended, 1 for should-be-attended, 0 for we-don't-know
				                                         # yt is a timestamp expressed as indices to (block-within-run,  trial-within-block, epoch-within-trial, sample-within-trial)

		if int(self.params.get('SoundTest', 0)) > 0: sig = numpy.abs(sig)
		if self.bandpass != None:                        # apply bandpass filter, if any, to signal channels
			sig[self.sigchan,:] = self.bandpass(sig[self.sigchan,:], axis=1)
		
		self.trapped = []                                # the oncollect() callback of self.seq will dump into this 
		if self.trigchan == None:                        # there is no physical trigger channel, so use software timing, via the EventOffset state:
			eo = self.detect_event()                     # first decode it, via the API function detect_event
			if self.states['Epoch'] == 0: eo = None      # ignore it if the application module has not announced that the trial has begun
			self.seq.process(sig, eo)                    # and pass the signal to the (triggerless) trap sequence
			if eo != None: self.eo.append(eo)            
		else:                                            # a physical trigger channel exists
			if self.triggerfilter != None:               # so highpass the trigger signals if asked to
				sig[self.trigchan,:] = self.triggerfilter(sig[self.trigchan,:], axis=1)
			self.seq.process(sig)                        # ...and pass the signal to the (triggered) trap sequence

		outsig = numpy.zeros(self.out_signal_dim, dtype=numpy.float64) # prepare the output signal packet (all zeros)
		ready = 0
		if len(self.trapped) > 1: raise RuntimeError("more than one trap returned per packet")
		for x,t in self.trapped:                                  # the oncollect() callback of our (Triggerless)TrapSequence subclass appends signal epoch and timestamp to self.trapped
			t -= self.sample0                                     # re-express the timestamp as samples relative to the start of the first epoch of the trial
			self.nEpochsTrapped += 1 
			if self.detrend:                                      # if asked to detrend each epoch, do so (signal channels only)
				todetrend = [i for i in self.sigchan if x[i,:].var()]
				if len(todetrend): x[todetrend, :] = scipy.signal.detrend(x[todetrend, :], axis=1)
			if self.saving: self.dump(x=x, xt=t)                               # saving training data for future classifiers
			if self.weights != None:                                           # such classifiers return sets of weights and a bias term
				dv = sum(numpy.multiply(self.weights, x).flat) + self.bias     # so let's apply them if they already exist
				p = SigTools.logistic(dv)                                      # then transform the output into a probability
				ready = 1                                                      # then get ready to raise the flag, such that the Application module can see it, indicating that a new classification has been made
				if self.saving: self.dump(p=p)                                 # then include the probabilistic classifier output in the bunch of things to save in the output file
				outsig[0, :] = p                                               # first output channel carries the probabilistic classifier output
			outsig[1, :] = SigTools.samples2msec(t, self.eegfs)                # second output channel carries the epoch timestamp, in msec since the beginning of the trial

		self.states['Ready'] = ready
		return outsig

	#############################################################
	
	def CheckEpochs(self):
	  # Check if the number of Epochs processed equals the number trapped
		if int(self.params['CheckNumberOfEpochs']) and self.nEpochsExpected != self.nEpochsTrapped:
			if self.saving:
				self.dump(discard={
					'x': self.nEpochsTrapped,
					'xt':self.nEpochsTrapped,
					'p': self.nEpochsTrapped,
					'y': self.nEpochsExpected,
					'yt':self.nEpochsExpected,
				})
				self.dump('flush')
			raise RuntimeError("expected %d epochs, trapped %d" % (self.nEpochsExpected, self.nEpochsTrapped))
		self.nEpochsExpected = 0
		self.nEpochsTrapped = 0

	#############################################################
	
	def plot(self, chan=None, sep=0, serial=False, f=None):
		
		d = self.load(f=f, catdim=0)
		if len(d) == 0: raise RuntimeError("empty file")

		if chan == None: chan = self.trigchan
		if serial:
			axis = 1
			if chan == None: chan = range(d['x'][0].shape[0])
		else:
			axis = 0
			if chan == None: chan = 0
		if not isinstance(chan, (tuple,list,numpy.ndarray)): chan = [chan]
		chnames = d['channels']
		x = numpy.concatenate([x[chan,:] for x in d['x']], axis=axis).T
		t = SigTools.samples2msec(numpy.arange(x.shape[0]), d['fs'])
		for i in range(len(chan)):
			if isinstance(chan[i], basestring): chan[i] = chnames.index(chan[i])
		chan = tuple(chan)
		if sep == 'image':
			import pylab
			pylab.subplot(211); SigTools.imagesc(img=x[:,d['y']<0].T, x=t, aspect='auto', clim=(x.min(),x.max()))
			pylab.subplot(212); SigTools.imagesc(img=x[:,d['y']>0].T, x=t, aspect='auto', clim=(x.min(),x.max()))
		else:
			x = x + [numpy.arange(x.shape[1]) * sep]
			SigTools.plot(t, x)
			import pylab; pylab.grid()
		
		
		
	#############################################################
	
	def classify(self, runs=None, xtn='.pk', C=0.25, rebias=True, save=True, plotopt=False, **kwargs):
		from BCI2000Tools.Classification import ClassifyERPs
		files = self.find_data_files(xtn=xtn, runs=runs)
		if runs == None: files = files[-1]
		u,c = ClassifyERPs(files, C=C, rebias=rebias, save=save, **kwargs)

		import SigTools
		u.channels = SigTools.ChannelSet(u.channels)
		
		print u.description
		return u,c
		
#################################################################
#################################################################
