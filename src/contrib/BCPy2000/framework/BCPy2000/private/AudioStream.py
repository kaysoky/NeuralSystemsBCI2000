# TODO:
#   - maybe an option to have the background continuous, with beats added, instead of current cross-fade (convex combination)

__all__ = ['Stimulus', 'Maker']
import numpy
import random
import copy

import WavTools
import SigTools

import sys
if 'BCI2000PythonCore' in sys.modules: import BCI2000PythonCore as core
elif 'BCPy2000.Generic' in sys.modules: import BCPy2000.Generic as core
else: raise ImportError, 'could not find core'
EndUserError = core.EndUserError 
BciFunc = core.BciFunc
# Ugh. Really, ugh. Surely there must be some way of saying
# from (wherever the superclass of a particular instance came from) import *
# but I haven't figured it out.

#################################################################
#################################################################

class StimulusError(Exception): pass

#################################################################
#################################################################

StimulusTypes = ['None', 'Background', 'Standard', 'Target', 'Deviant']
RowLabels = ' '.join(StimulusTypes[1:])
ParameterDefinitions = [
	"PythonApp:Task    int       PerceptualOnly=                           0                      0     0 1 // if checked, mute non-target streams (boolean)",
	"PythonApp         int       SurroundSoundTrigger=                     0                      0     0 1 // if checked, deliver the trigger signal in sound channels 3 and 4 (boolean)",
	"PythonApp         int       LowerStimulusThreadPriority=              0                      0     0 1 // if checked, try to lower the priority of the stimulus factory thread (boolean)",

	"PythonApp:Streams int       NumberOfStreams=                          2                      2     2 % // number of streams",
	"PythonApp:Streams float     MinPropTargets=                           0.1                    0.1   0 1 // minimum number of targets, as a proportion of the largest number of variable beats among streams",
	"PythonApp:Streams float     MaxPropTargets=                           0.8                    0.8   0 1 // maximum number of targets, as a proportion of the smallest number of variable beats among streams",
	"PythonApp:Streams floatlist PeriodMsec=                         2   490     555            500     0 % // period of stimuli for each stream in msec",
	"PythonApp:Streams floatlist OffsetMsec=                         2   500     570              0     0 % // offset of each stimulus stream in msec",
	"PythonApp:Streams floatlist InitialStandards=                   2     3       3              3     0 % // how many stimuli at the beginning of each stream are guaranteed to be standards",
	"PythonApp:Streams intlist   NumberOfDeviants=                   2     0       0              0     0 1 // number of deviants in each stream",

	"PythonApp:Streams int       RoundPeriods=                             0                      0     0 2 // stream period rounding: 0 no rounding, 1 adjust to an integer number of carrier cycles, 2 adjust to an integer number of modulation cycles (enumeration)",
	"PythonApp:Streams int       RoundModFreq=                             0                      0     0 2 // modulation frequency rounding: 0 no rounding, 1 integer number of cycles per beat, 2 integer number of EEG samples per modulation cycle (enumeration)",
	"PythonApp:Streams int       RoundCarrierFreq=                         0                      0     0 2 // carrier frequency rounding: 0 no rounding, 1 integer number of cycles per beat, 2 integer number of cycles per modulation cycle (enumeration)",
	"PythonApp:Streams int       RoundOffsets=                             0                      0     0 2 // stream SOA rounding: 0 no rounding, 1 adjust to an integer number of carrier cycles, 2 adjust to an integer number of modulation cycles (enumeration)",
	"PythonApp:Streams int       RoundDurations=                           0                      0     0 2 // pulse duration rounding: 0 no rounding, 1 adjust to an integer number of carrier cycles, 2 adjust to an integer number of modulation cycles (enumeration)",

	"PythonApp:Streams matrix    Amplitude=          {"+RowLabels+"} 2     0.0     0.0        1.0     0.9        1.0     0.9        1.0     0.9            1.0     0.0 1.0  // amplitude of each stimulus (types by streams)",
	"PythonApp:Streams matrix    DurationMsec=       {"+RowLabels+"} 2  5000.0  5000.0       50.0    50.0       50.0    50.0       50.0    50.0           50.0     0.0  %   // duration of each stimulus pulse (types by streams)",
	"PythonApp:Streams matrix    AttackMsec=         {"+RowLabels+"} 2   250.0   250.0        5.0     5.0        5.0     5.0        5.0     5.0            5.0     0.0  %   // rise time of each stimulus pulse (types by streams)",
	"PythonApp:Streams matrix    DecayMsec=          {"+RowLabels+"} 2   100.0   100.0        5.0     5.0        5.0     5.0        5.0     5.0            5.0     0.0  %   // fall time of each stimulus pulse (types by streams)",
	"PythonApp:Streams matrix    CarrierType=        {"+RowLabels+"} 2  square  square     square  square     square  square     square  square         square      %   %   // (types by streams)",
	"PythonApp:Streams matrix    CarrierFreqHz=      {"+RowLabels+"} 2  1500.0   800.0     1500.0   800.0     1650.0   880.0     1500.0   800.0          800.0     0.0  %   // (types by streams)",
	"PythonApp:Streams matrix    ModulationType=     {"+RowLabels+"} 2    sine    sine       sine    sine       sine    sine       sine    sine           sine      %   %   // (types by streams)",
	"PythonApp:Streams matrix    ModulationFreqHz=   {"+RowLabels+"} 2    45.0    62.0       45.0    62.0       45.0    62.0       45.0    62.0           45.0      %   %   // (types by streams)",
	"PythonApp:Streams matrix    ModulationDepth=    {"+RowLabels+"} 2     0.0     0.0        0.0     0.0        0.0     0.0        0.0     0.0            0.0     0.0 1.0  // (types by streams)",

	"PythonApp:Streams matrix    AudioMixingMatrix=                2 2   1.0 0.0   0.0 1.0        1.0   % % // audio mixing matrix (outputs by inputs)",
]

#################################################################
#################################################################

class Maker(object):

	#############################################################

	def __init__(self, params):
		
		stimtypes = RowLabels.split()
		
		nstreams = params['NumberOfStreams'].val
		# check length = number of streams
		for paramname in ['PeriodMsec', 'OffsetMsec', 'InitialStandards', 'NumberOfDeviants']:
			v = params[paramname].val
			if len(v) != nstreams: raise EndUserError, "number of elements of %s parameter must match NumberOfStreams (=%d)" % (paramname, nstreams)
		
		# check number of columns = number of streams
		for paramname in ['Amplitude', 'DurationMsec', 'AttackMsec', 'DecayMsec', 'CarrierType', 'CarrierFreqHz', 'ModulationType', 'ModulationFreqHz', 'ModulationDepth', 'AudioMixingMatrix']:
			v = params[paramname].val
			if v.shape[1] != nstreams: raise EndUserError, "number of columns of %s parameter must match NumberOfStreams (=%d)" % (paramname, nstreams)
		
		# check row labels correspond to stimulus types (Background, Standard, Target, Deviant)
		for paramname in ['Amplitude', 'DurationMsec', 'AttackMsec', 'DecayMsec', 'CarrierType', 'CarrierFreqHz', 'ModulationType', 'ModulationFreqHz', 'ModulationDepth']:
			v = params[paramname]
			if v.matrixlabels()[0] != stimtypes:
				raise EndUserError, "%s parameter must have rows %s" % (paramname, str(stimtypes))
	
		BciDict = params.__class__
		
		relevant_paramnames = [x.split()[2].rstrip('=') for x in ParameterDefinitions]
		relevant_paramnames.append('EpochDurationMsec') # TODO: this is slightly ugly---it makes sense to keep things such that the signal-processing module declares this parameter, but it's helpful to know it here
		relevant_paramnames.append('EnslavePython')
		relevant_paramnames.append('SamplingRate')
		subset = [(key,copy.deepcopy(params[key])) for key in relevant_paramnames]
		params = BciDict(dict(subset), lazy=True)
		params['SamplingRate'] = [float(params['SamplingRate'].lower().rstrip('hz'))] * int(params['NumberOfStreams'])
		
		def roundto(params, roundee, rounder, stimtype, istream):
			p = params[roundee]
			if isinstance(p[0],list): x = float(p[stimtype][istream])
			else: x = float(p[istream])
			q = params[rounder]
			if isinstance(q[0],list): y = float(q[stimtype][istream])
			else: y = float(q[istream])
			if   rounder == 'SamplingRate' and roundee.endswith('Hz'): xout = y / round(y / x)
			else:
				if   rounder.endswith('Msec') and roundee.endswith('Hz'):   fac = 1000.0 / y
				elif rounder.endswith('Hz')   and roundee.endswith('Msec'): fac = 1000.0 / y
				elif rounder.endswith('Hz')   and roundee.endswith('Hz'):   fac = y
				elif rounder.endswith('Msec') and roundee.endswith('Msec'): fac = y
				else: raise ValueError, "huh?"
				xout = fac * round(x / fac)
			if abs(xout-x) > 0.5: raise EndUserError, "rounding according to %s=%g changes %s from %g to %g. Maybe set %s=%g to start with?" % (rounder,y,roundee,x,xout,roundee,round(xout))
			if isinstance(p[0],list): p[stimtype][istream] = xout
			else: p[istream] = xout
		
		RoundPeriods = params['RoundPeriods'].val
		if params.has_key('IntegerModCyclesPerBeat'): RoundModFreq = int(params['IntegerModCyclesPerBeat'])
		else: RoundModFreq = int(params['RoundModFreq'])
		if params.has_key('IntegerCarrierCyclesPerBeat'): RoundCarrierFreq = int(params['IntegerCarrierCyclesPerBeat'])
		else: RoundCarrierFreq = int(params['RoundCarrierFreq'])			
		RoundDurations = params['RoundDurations'].val
		RoundOffsets   = params['RoundOffsets'].val

		for istream in range(nstreams):

			if RoundModFreq == 2:
				for stimtype in stimtypes:
					roundto(params, 'ModulationFreqHz', 'SamplingRate', stimtype, istream)
			
			if   RoundPeriods == 1: roundto(params, 'PeriodMsec', 'CarrierFreqHz',    'Background', istream)
			elif RoundPeriods == 2: roundto(params, 'PeriodMsec', 'ModulationFreqHz', 'Background', istream)
			elif RoundPeriods: raise EndUserError, 'unsupported value for parameter "RoundPeriods"'

			for stimtype in stimtypes:
				if   RoundModFreq == 1: roundto(params, 'ModulationFreqHz', 'PeriodMsec', stimtype, istream)
				elif RoundModFreq == 2: pass # round according to sampling rate---handled above
				elif RoundModFreq: raise EndUserError, 'unsupported value for parameter "RoundModFreq"'
	
				if   RoundCarrierFreq == 1: roundto(params, 'CarrierFreqHz', 'PeriodMsec',       stimtype, istream)
				elif RoundCarrierFreq == 2: roundto(params, 'CarrierFreqHz', 'ModulationFreqHz', stimtype, istream)
				elif RoundCarrierFreq: raise EndUserError, 'unsupported value for parameter "RoundCarrierFreq"'
	
				if   RoundDurations == 1: roundto(params, 'DurationMsec', 'CarrierFreqHz',    stimtype, istream)
				elif RoundDurations == 2: roundto(params, 'DurationMsec', 'ModulationFreqHz', stimtype, istream)
				elif RoundDurations: raise EndUserError, 'unsupported value for parameter "RoundDurations"'
	
			if   RoundOffsets == 1: roundto(params, 'OffsetMsec', 'CarrierFreqHz',    'Background', istream)
			elif RoundOffsets == 2: roundto(params, 'OffsetMsec', 'ModulationFreqHz', 'Background', istream)
			elif RoundOffsets: raise EndUserError, 'unsupported value for parameter "RoundOffsets"'
	
		self.params = params
		self.soundfs = 44100
		
		total_msec = numpy.asarray(params['DurationMsec']['Background',:].val)
		total_samples = WavTools.msec2samples(max(total_msec), self.soundfs)
		
		self.samplewavs = BciDict(lazy=True)
		self.pulses = BciDict(lazy=True)
		for stimtype in stimtypes:
			self.pulses[stimtype] = BciDict(lazy=True)
			for istream in range(nstreams):
				attack    = float(params['AttackMsec'][stimtype][istream])
				decay     = float(params['DecayMsec'][stimtype][istream])
				duration  = float(params['DurationMsec'][stimtype][istream])
				
				rise    = WavTools.rise(duration=attack/1000.0, fs=self.soundfs, hanning=True)
				fall    = WavTools.fall(duration=decay/1000.0,  fs=self.soundfs, hanning=True)
				ns      = WavTools.msec2samples(duration, self.soundfs) - rise.samples() - fall.samples()
				plateau = WavTools.silence(ns, 1, dtype=rise, dc=1.0)
				pulse   = rise % plateau % fall
				self.pulses[stimtype][istream] = pulse
				if stimtype == 'Background':
					blank = WavTools.silence(total_samples - pulse.samples(), 1)
					self.bgrise = rise
					self.bgfall = fall % blank
					self.pulses[stimtype][istream] %= blank
				
				ctype = params['CarrierType'][stimtype, istream]
				if ctype.lower().endswith('.wav'):
					try: w = WavTools.wav(ctype)
					except Exception, e: raise EndUserError, e.message
					if w.fs != self.soundfs: raise EndUserError, "file '%s' has the wrong sampling frequency (should be %gHz)" %(ctype,self.soundfs)
					w.trim()
					w = w.mono()
					ww = w.copy()
					while ww.duration() < duration/1000.0: ww %= w
					self.samplewavs[stimtype] = ww[:duration/1000.0]
					sampletrack[stimtype] = silent.copy()
				
	#############################################################

	def NewWav(self, duration=0.0,nchan=0,dc=0.0):
		nsamp = WavTools.msec2samples(duration*1000, self.soundfs)
		if nsamp > 0 and nchan == 0: nchan = 1
		w = WavTools.wav(fs=self.soundfs,bits=16,nchan=nchan)
		w.y = WavTools.silence(nsamp, nchan, dtype=numpy.float64, dc=dc)
		return w
	
	#############################################################

	def waves(self, freq, wavetypes, paramname, sampletrack={}):

		theta =  numpy.cumsum(freq * (2.0 * numpy.pi / self.soundfs), axis=WavTools.across_samples)
		antialias = True

		if paramname.lower().startswith('mod'):
			theta -= numpy.pi/2
			antialias = False
			
		kwargs = {'maxharm':None, 'rescale':True}
		if antialias: kwargs['maxharm'] = int(0.95 * self.soundfs / (2.0 * freq.max()))
		
		funcs = {
			'sine'  :   SigTools.sinewave,
			'square':   SigTools.squarewave,
			'triangle': SigTools.trianglewave,
			'sawtooth': SigTools.sawtoothwave,
		}
		f = list(wavetypes)
		for i in range(len(f)):
			func = funcs.get(f[i])						
			if func == None and f[i].startswith('square'):
				try: duty = float(f[i].lstrip('square (').rstrip(')'))
				except: pass
				else:
					import scipy
					func = BciFunc(scipy.signal.square, duty=duty)
					kwargs = {}
					if antialias: raise ValueError, "'square' cannot have a custom duty parameter for the carrier, only for the modulator"
			if func == None:
				s = sampletrack.get(StimulusTypes[i+1])
				if s == None: raise ValueError, "unrecognized periodic function \"%s\" in parameter %s" % (str(f[i]), paramname)
				f[i] = s.y
			else:
				f[i] = func(theta, **kwargs)
		f = numpy.concatenate(f, axis=WavTools.across_channels)
		return f

#################################################################
#################################################################

class Stimulus(object):

	#############################################################

	def __init__(self, factory, nexttarget=None):
		
		params = factory.params
		
		if params['LowerStimulusThreadPriority'].val:
			try: import PrecisionTiming; PrecisionTiming.SetThreadPriority(-3)
			except: print "failed to lower priority of AudioStream.Stimulus factory thread"
					
		epoch = params['EpochDurationMsec'].val  / 1000.0

		total_sec = numpy.asarray(params['DurationMsec']['Background',:].val) / 1000.0

		silent       = factory.NewWav(max(total_sec),1)
		self.states  = factory.NewWav()
		self.streams = factory.NewWav() # before AudioMixingMatrix is applied
		self.sound   = factory.NewWav() # after AudioMixingMatrix is applied

		nstreams = params['NumberOfStreams'].val

		if params['EnslavePython'].val:
			self.streams.y = numpy.zeros((silent.samples(), nstreams))
			self.sound.y = (numpy.random.rand(silent.samples(), 2)-0.5) * 0.3
			self.states.y = self.streams.y * 0
			self.nbeats = [0] * nstreams
			self.ntargets = [0] * nstreams
			self.seq = [''] * nstreams
			return
		
		leadin = params['InitialStandards'].val
		ndev = params['NumberOfDeviants'].val
		stimtimes = [[]] * nstreams
		n_variable_beats = [0] * nstreams
		for istream in range(nstreams):
			period       = params['PeriodMsec'].val[istream] / 1000.0
			offset       = params['OffsetMsec'].val[istream] / 1000.0
			stimtimes[istream] = numpy.arange(offset, total_sec[istream]-epoch+0.0000001, period)
			n_variable_beats[istream] = len(stimtimes[istream]) - leadin[istream]

		minntargets = int(numpy.ceil( max(n_variable_beats) * float(params['MinPropTargets'])))
		maxntargets = int(numpy.floor(min(n_variable_beats) * float(params['MaxPropTargets'])))		
		maxperiod = max(params['PeriodMsec'].val)/1000.0

		self.nbeats = [len(tt) for tt in stimtimes]
		
		for attempt in range(100):
			self.ntargets = [0] * nstreams
			devtimes = [[]] * nstreams
			targettimes = [[]] * nstreams
			alldevs = []
			for istream in range(nstreams):
				t = stimtimes[istream][leadin[istream]:]
				random.shuffle(t)
				nd = ndev[istream]
				dt,t = t[:nd],t[nd:]
				if len(dt):
					devtimes[istream] = (sorted(dt))
					clearance = min([numpy.inf] + [abs(x-y) for x in devtimes[istream] for y in alldevs])
					if clearance < maxperiod: break # failure - try again
					alldevs += devtimes[istream]
				else:
					devtimes[istream] = []
				
				nt = self.ntargets[istream] = random.randint(minntargets, maxntargets)
				tt,t = t[:nt],t[nt:]
				targettimes[istream] = sorted(tt)				
			else:
				break # success
		else: # tried too many times
			raise StimulusError, 'failed to schedule deviants'
		
		self.seq = [''] * nstreams
		for istream in range(nstreams):
			
			weighting = {}; sampletrack = {}
			for stimtype in StimulusTypes[1:]:
				weighting[stimtype] = silent.copy()
				if stimtype in factory.samplewavs:
					sampletrack[stimtype] = silent.copy()
					
			for ibeat in range(len(stimtimes[istream])):
				t = stimtimes[istream][ibeat]
				if t in devtimes[istream]: stimtype = 'Deviant'
				elif t in targettimes[istream]: stimtype = 'Target'
				else: stimtype = 'Standard'
				self.seq[istream] += stimtype[0].lower()
				
				pulse = factory.pulses[stimtype][istream]
				weighting[stimtype][t:t+pulse.duration()] += pulse
				s = factory.samplewavs.get(stimtype)
				if s != None: sampletrack[stimtype][t:t+s.duration()] += s
			
			t = 0.0; stimtype = 'Background'
			s = factory.samplewavs.get(stimtype)
			if s != None: sampletrack[stimtype][t:t+s.duration()] += s

			b = weighting['Background'] = 1.0 - sum(weighting.values())
			b.y = b.y.clip(0.0,1.0)
			weighting = [weighting[k] for k in StimulusTypes[1:]]
			weighting = WavTools.stack(weighting) / sum(weighting)
			
			# Per-stream state variables will reflect 1 + max column index of nonzero values on each row.
			# Currently these integer values are stored at the audio sampling rate: they can be used
			# to make synch signals on additional surround-sound channels of the soundcard, and can be
			# subsampled to set the state variables (provided we can get the offset right...)
			self.states &= numpy.asmatrix([range(1,weighting.channels()+1)] * (weighting.y > 0.0), dtype=numpy.float64).max(axis=WavTools.across_channels).A
			
			wt = numpy.asmatrix(weighting.y)
			
			# set up modulator for each stimulus type
			modulation_freq  = (wt * params['ModulationFreqHz'].val[:,istream]).A
			modulation = factory.waves(modulation_freq, params['ModulationType'][:,istream], 'ModulationType')
			depth = params['ModulationDepth'].val[:, istream].A.T / 2.0
			modulation = (1.0 - depth) + depth * modulation

			# set up carrier for each stimulus type
			carrier_freq = (wt * params['CarrierFreqHz'].val[:,istream]).A
			carrier = factory.waves(carrier_freq, params['CarrierType'][:,istream], 'CarrierType', sampletrack=sampletrack)
			amp = params['Amplitude'].val[:, istream].A.T
			if int(params['PerceptualOnly']):
				nt = nexttarget - 1
				amp = amp * float(istream == nt)
				self.ntargets = [int(i==nt) * self.ntargets[i] for i in range(nstreams)]
				
			carrier = carrier * amp
			
			# special case: adjust background carrier according to this individual stream's
			# duration, rise time and fall time parameters
			bgindex = StimulusTypes[1:].index('Background')
			rise,fall = factory.bgrise,factory.bgfall
			carrier[:rise.samples(),  bgindex] *= rise.y[:,0]
			carrier[-fall.samples():, bgindex] *= fall.y[:,0]

			# combine the different carriers according to the weighting
			carrier = numpy.asmatrix(wt.A * carrier).sum(axis=WavTools.across_channels)
			# combine the different modulators according to the weighting
			modulation = numpy.asmatrix(wt.A * modulation).sum(axis=WavTools.across_channels)
			# multiply carrier and modulator
			self.streams &= carrier.A * modulation.A
		
		mm = params['AudioMixingMatrix'].val.T
		# AudioMixingMatrix is specified in the param spec as one row per outputs, one column per input.
		# To post-multiply the samples-by-channels data in wav object self.streams, we need it transposed.

		self.sound.y = (numpy.asmatrix(self.streams.y) * mm).A		
		if int(params['SurroundSoundTrigger']):
			self.sound &= (self.states-1) * SigTools.wavegen(freq_hz=1000, container=self.states.copy())
		
	#############################################################
	
	def plot(self):
		(self.sound & (self.states/5)).plot()
		
	#############################################################
	
	def plotsig(self):
		self.sound.plotsig()
		
	#############################################################
	
	def play(self):
		self.sound.play()
		
#################################################################
#################################################################
