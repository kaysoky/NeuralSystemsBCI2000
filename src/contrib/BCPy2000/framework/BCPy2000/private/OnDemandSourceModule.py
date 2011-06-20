import numpy,os
from SigTools import loadmat, msec2samples, summarize, fader


#################################################################
#################################################################

class BciSource(BciGenericSource):

	#############################################################

	def Construct(self):
		parameters = [
			"Source:Playback string SourceFileName= % % % % // play back the named BCI2000 file (inputfile)",
			"Source:Playback string TargetStateName= target % % % // the name of the state variable that indicates target class",
			"Source:Playback float  RiseTimeMsec= 200 200 0 % // time in msec taken to fade in signals",
			"Source:Playback float  FallTimeMsec= 200 200 0 % // time in msec taken to fade out signals",
		]
		states = [
			
		]
				
		return (parameters, states)
		
	#############################################################
	
	def Preflight(self, inprop):
		if not self.params['TargetStateName'] in self.states:
			raise EndUserError, "no state variable '%s'"%self.params['TargetStateName']

	#############################################################

	def Initialize(self, indim, outdim):
		
		fn = self.params['SourceFileName']
		try: source = loadmat(fn)
		except Exception, e: raise EndUserError(str(e))
		
		self.x = source.get('x')
		if self.x == None: raise EndUserError, "no 'x' variable in source file"
		try: self.ntrials,self.nchannels,self.nsamples = self.x.shape
		except: raise EndUserError, "'x' variable in source file must be a 3-D array"

		self.y = source.get('y')
		if self.y == None: raise EndUserError, "no 'y' variable in source file"
		self.y = numpy.asarray(self.y).ravel()
		if len(self.y) != self.ntrials: raise EndUserError, "'y' variable in source file has the wrong number of elements (expected %d, got %d)" % (self.ntrials, len(self.y))
		
		self.trialind = {}
		for i in set(self.y): self.trialind[i] = set(numpy.argwhere(self.y==i).ravel().tolist())
		
		prmch = int(self.params['SourceCh'])
		if self.nchannels != prmch: raise EndUserError, 'mismatch between number of channels in SourceCh parameter (%d) and source file (%d)' % (prmch,self.nchannels)

		prmfs = self.samplingrate()
		fs = source.get('fs')
		if fs == None:  raise EndUserError, "no 'fs' variable in source file"
		if fs != prmfs: raise EndUserError, 'mismatch between sampling rate in SamplingRate parameter (%gHz) and source file (%gHz)' % (prmfs,fs)
		
		self.target = 0
		self.fader = fader(0.0)
		self.rise = 0.5+0.5*numpy.sin(numpy.linspace(-numpy.pi/2, +numpy.pi/2, msec2samples(self.params['RiseTimeMsec'], prmfs), endpoint=True))
		self.fall = 0.5+0.5*numpy.sin(numpy.linspace(+numpy.pi/2, -numpy.pi/2, msec2samples(self.params['FallTimeMsec'], prmfs), endpoint=True))
		
	#############################################################

	def StartRun(self):
		self.fader = fader(0.0)
		self.target = 0
		
	#############################################################

	def Process(self, sig):
		target = self.states[self.params['TargetStateName']]
		prevtarget,self.target = self.target,target
		
		if target != prevtarget:
			if prevtarget != 0:
				self.fader += self.fall * self.fader.gain
			if target != 0:
				self.fader += self.rise
		
		sig = 10.0 * numpy.random.randn(*sig.shape)
		return self.fader.apply(sig)
		
		
#################################################################
#################################################################
