#import ConsoleRenderer

import os
import sys
import random
import numpy
import time
import WavTools
import AppTools.ParallelPort
import AppTools.Displays
import AppTools.StateMonitors
import pygame.locals

import AudioStream
		
#################################################################
#################################################################

class BciApplication(BciGenericApplication):
				
	#############################################################

	def Description(self):
		return "attention to auditory streams"

	#############################################################

	def Construct(self):
		self.maxstreams = 2
		params = [
			"PythonApp         float     WindowSize=                               1.0                    1.0   0 1 // subject window size from 0 to 1",
			"PythonApp         float     SystemMasterVolume=                       1.0                    1.0   0 1 // operating-system volume setting from 0 to 1",
			"PythonApp         floatlist ChannelVolumesDB=                  1    -18                      0.0   % 0 // per-audio-channel stimulus attenuation in dB",
			"PythonApp         int       HeadPhones=                               0                      0     0 1 // use headphones or not? (boolean)",
			"PythonApp         int       DirectSound=                              1                      0     0 1 // use DirectSound interface or not? (boolean)",
			"PythonApp:Task    int       FreeChoice=                               0                      0     0 1 // allow user to choose freely (boolean)",
		]
		params += AudioStream.ParameterDefinitions
		nbits = numpy.ceil(numpy.log2(self.maxstreams + 1.0))
		states = [
			"TargetStream    %d 0 0 0"%nbits,
			"PredictedStream %d 0 0 0"%nbits,
			"CueOn            1 0 0 0",
			"Response         4 0 0 0",
			"CorrectResponse  4 0 0 0",
		]		
		nbits = numpy.ceil(numpy.log2(len(AudioStream.StimulusTypes)))
		for stream in range(1, self.maxstreams+1):
			states.append("Stream%d %d 0 0 0"%(stream,nbits))

		self.lpt = AppTools.ParallelPort.lpt()

		return params,states

	#############################################################

	def Preflight(self, sigprops):
		if int(self.params['FreeChoice']) and int(self.params['PerceptualOnly']):
			raise EndUserError, "the FreeChoice and PerceptualOnly options are incompatible"
		if len(self.params['ERPClassifierWeights'].val) == 0:
			if int(self.params['FreeChoice']): raise EndUserError, "ERPClassifierWeights should not be empty when FreeChoice is activated"
		else:
			if int(self.params['PerceptualOnly']): raise EndUserError, "ERPClassifierWeights should be empty when PerceptualOnly is activated"

		self.nstreams = self.params['NumberOfStreams'].val
		if self.nstreams < 2: raise EndUserError, "NumberOfStreams must be at least 2"
		if self.nstreams > self.maxstreams: raise EndUserError, "NumberOfStreams exceeds current hard-coded limit of %d"%self.maxstreams
		
		self.StimulusMaker = AudioStream.Maker(self.params)
		
		windowsize = float(self.params['WindowSize'])
		if AppTools.Displays.number_of_monitors() > 1: windowsize = 1.0
		AppTools.Displays.fullscreen(id=-1, scale=windowsize)
		self.screen.setup(frameless_window=(windowsize==1.0))

		if 'ConsoleRenderer' in sys.modules: self.screen.fake()

		if int(self.params['DirectSound']):
			import DirectSoundInterface
		elif 'DirectSoundInterface' in sys.modules and sys.modules['DirectSoundInterface'].loaded:
			raise EndUserError, "once turned on, the DirectSound setting cannot be turned off without restarting BCI2000"
		
		self.audiofs = 44100
		self.audiobits = 16
		self.audiochannels = self.params['AudioMixingMatrix'].val.shape[0]
		
		self.chanvol = [float(x) for x in self.params['ChannelVolumesDB']]
		if len(self.chanvol) == 1:
			self.chanvol = self.chanvol * self.audiochannels
		if len(self.chanvol) != self.audiochannels:
			raise EndUserError("ChannelVolumesDB parameter must have either one element, or one element per audio channel (= number of rows of AudioMixingMatrix parameter = %d)" % self.audiochannels)
		for x in self.chanvol:
			if x > 0.0: raise EndUserError("elements of the ChannelVolumesDB parameter must be negative, or zero")

	#############################################################

	def Initialize(self, indim, outdim):
		
		self.transient('PredictedStream', manual=True)
		self.transient('Response', manual=True)

		self.screen.color = (0.2,0.2,0.2)

		if int(self.params['HeadPhones']): starttext = 'HEADPHONES'
		else: starttext = 'SPEAKERS'
		
		if int(self.params['PerceptualOnly']): starttext += '   one-sided (perception)'
		else:
			starttext += '   two-sided (attention)'
			if len(self.params['ERPClassifierWeights'].val) == 0:
				starttext += '   no classifier loaded'

		w,h = self.screen.size
		t = VisualStimuli.Text(text=starttext, position=(w/2,h/2), anchor='top', on=True)
		self.stimulus('cue', t)
		
		self.reset_count()
		self.count_feedback_stimuli = []
		for istream in range(self.nstreams):
			x = w/4 + (w/2) * istream/float(self.nstreams-1)
			t = VisualStimuli.Text(text='?', position=(x,h/2), anchor='center', on=False)
			stim = self.stimulus('count%d'%(istream+1), t)
			self.count_feedback_stimuli.append(stim)
			self.stimulus('StreamVolume%d'%(istream+1), VisualStimuli.Text, text='?', position=(x,h), anchor='top', on=False)
		
		self.factory = WavTools.background_queue()
		self.streams = []
		self.current_stream = None
		if int(self.params['ShowSignalTime']):
			self.addphasemonitor()
			for istream in range(self.nstreams): self.addstatemonitor('Stream%d'%(istream+1))
			self.addstatemonitor('CurrentTrial')
			self.addstatemonitor('TargetStream')
			self.addstatemonitor('PredictedStream')
				
		self.player = WavTools.player(WavTools.wav(fs=self.audiofs, bits=self.audiobits, nchan=self.audiochannels))
		self.player.set_preplay_hook(self.start_stimulus)
		self.player.set_postplay_hook(self.finish_stimulus)
		self.UpdateChannelVolumes()
		
		
		self.ding = WavTools.player('ding.wav')
		self.chimes = WavTools.player('chimes.wav')
		
		self.freechoice = int(self.params['FreeChoice'])
		self.last_prediction = 0
		vol = float(self.params['SystemMasterVolume'])
		self.init_volume(vol)
		
		self.arm()
		
	#############################################################
	
	def StartRun(self):
		self.user_responses = []
		self.predictions = []
		self.target = 0
		self.targetorder = []
		self.arm()
		self.last.pop('stream', None) # AAA
		#self.streamstart = None       # BBB
		self.remember('volchange')

	#############################################################
	
	def Phases(self):
		
		if self.freechoice: cuelen = 3000
		else: cuelen = 2000
		
		self.phase(  duration=None,   name='stimprep',       next='cue',       )
		self.phase(  duration=cuelen, name='cue',            next='stimulus',  )
		self.phase(  duration=None,   name='stimulus',       next='classify',  )
		self.phase(  duration=1000,   name='classify',       next='respond',   )
		self.phase(  duration=5000,   name='respond',        next='feedback',  )
		self.phase(  duration=2000,   name='feedback',       next='stimprep',  )
		
		self.design(start='stimprep', new_trial='stimprep')
		
	#############################################################

	def Transition(self, phase):
		
		self.states['CueOn'] = phase in ['cue']
		
		if phase == 'stimprep':
			self.states['TargetStream'] = 0
			self.target = self.states['TargetStream']
		if phase == 'cue':
			self.current_stream = self.streams.pop(0) # TODO:  problem with playback
			self.player.open(self.current_stream.sound)
			if self.freechoice:
				self.states['TargetStream'] = 0
			else:
				self.states['TargetStream'] = self.nexttarget()
			self.target = self.states['TargetStream']
			if self.target:
				self.states['CorrectResponse'] = self.current_stream.ntargets[self.target-1]
			else:
				self.states['CorrectResponse'] = 0
			self.stimuli['cue'].text = {0:'CHOOSE', 1:'LEFT', 2:'RIGHT'}.get(self.target, 'stream #%d'%self.target)
			self.stimuli['cue'].on = True
		elif phase == 'respond':
			if self.states['CurrentTrial'] < int(self.params['TrialsPerBlock']):
				self.arm() # stimulus preparation happens here, later than originally scheduled at line marked CCCC - may ease CPU bottleneck and allow increase in number of eeg channels?
			if self.freechoice and self.last_prediction:
				self.stimuli['cue'].text = {1:'interpreted as LEFT', 2:'interpreted as RIGHT'}.get(self.last_prediction, '?')
			else:
				self.stimuli['cue'].text = '?'
				self.screen.RaiseWindow()      # BB
			self.stimuli['cue'].on = True
		else:
			self.stimuli['cue'].on = True
			self.stimuli['cue'].text = '+'
		
		if phase == 'stimulus':
			self.reset_count()
			self.player.play()
		
		if phase == 'feedback':
			response = self.states['Response']
			correct = self.states['CorrectResponse']
			if correct == 0 and self.target > 0:
				correct = self.count['targets'][self.target-1]
				# this is good for playing back older experiments where there was no CorrectResponse state
			self.acknowledge('Response')
			self.user_responses.append([response, correct])
			
			for istream in range(self.nstreams):
				#nt = self.current_stream.ntargets[istream]
				nt = self.count['targets'][istream]
				stim = self.count_feedback_stimuli[istream]
				stim.text = str(nt)
				stim.on = True
				stim.color = (1,1,1)
				if istream+1 == self.target and correct != 0:
					if response == correct: stim.color = (0,1,0)
					else:                   stim.color = (1,0,0)
		else:
			for istream in range(self.nstreams):
				self.count_feedback_stimuli[istream].on = False
		
		# TODO:
		#    to communicate probabilistic prediction to app module, use sigproc module output signal?
		#    change "bing" to something less distracting
		
	#############################################################

	def Process(self, sig):
		sv = [0] * self.nstreams
		
		#if self.streamstart == None and self.player != None and len(self.player.timestamps['after']): # BBB
		#	self.streamstart = self.player.timestamps['after'][0] - self._prectime_zero  # BBB
		#streamstart = self.streamstart  # BBB
		
		streamstart = self.last.get('stream',{}).get('msec') # AAA
		
		if streamstart != None:
			t = (self.last['cycle']['msec'] - streamstart) / 1000.0
			begin = max([0, t - self.nominal['SecondsPerPacket']])
			end   = max([0, t])
			if end > begin:
				st = self.current_stream.states[begin:end]
				if st.samples(): sv = st.y.max(axis=WavTools.across_samples)
		
		for istream in range(self.nstreams):
			self.states['Stream%d'%(istream+1)] = int(sv[istream])
			
		self.update_count()
		
		if self.in_phase('stimprep', 2):
			#if len(self.streams)==0 and not self.factory.busy: self.arm()  # CCCC arming postponed to later ('respond' phase)
			if len(self.streams): self.change_phase()
		
		if self.in_phase('classify', 2):
			if self.states['PredictedStream']:
				self.decide(self.states['PredictedStream'], self.target)
				self.acknowledge('PredictedStream')
				self.states['TargetStream'] = 0
				self.change_phase()
		
		for i in range(self.nstreams):
			stim = self.stimuli['StreamVolume%d'%(i+1)]
			stim.on = (self.since('volchange')['msec'] < 1000)
		
	#############################################################

	def Event(self, phase, event):
		if phase == 'respond' and event.type == pygame.locals.KEYUP:
			key = event.key
			if key in range(256, 266): self.states['Response'] = key - 256
			elif key in range(48,58):  self.states['Response'] = key - 48
			#print self.states['Response']
			self.change_phase()
		
		if phase != 'respond' and event.type == pygame.locals.KEYUP:
			key = event.key
			vc = self.volctrl = getattr(self, 'volctrl', {})
			chan = vc['channel'] = vc.get('channel', 0)
			if key in (pygame.locals.K_LEFT, pygame.locals.K_RIGHT, pygame.locals.K_UP, pygame.locals.K_DOWN):
				if key == pygame.locals.K_LEFT:  vc['channel'] = max(0, vc['channel'] - 1)
				if key == pygame.locals.K_RIGHT: vc['channel'] = min(self.nstreams, vc['channel'] + 1)
				if key == pygame.locals.K_UP:    self.chanvol[chan] = min(   0.0, self.chanvol[chan] + 3.0 )
				if key == pygame.locals.K_DOWN:  self.chanvol[chan] = max(-100.0, self.chanvol[chan] - 3.0 )
				vc['string'] = "PythonApp  floatlist ChannelVolumesDB= %d    %s  // adjusting channel %d" % ( len(self.chanvol),  '  '.join(["%g"%x for x in self.chanvol]), vc['channel']+1 )
				print vc['string']
				for i in range(self.nstreams):
					stim = self.stimuli['StreamVolume%d'%(i+1)]
					if i == vc['channel']: stim.color = (1, 1, 1)
					else: stim.color = (0.75,0.75,0.75)
					stim.text = '%gdB' % self.chanvol[i]
				self.remember('volchange')
				self.UpdateChannelVolumes()
			
	#############################################################

	def StopRun(self):
		if not isinstance(self.data_file, str): raise 'StopRun failed in PythonApp because self.data_file is not valid'
		
		fn = self.data_file.rstrip('.dat') + '_responses.txt'
		m = repr(self.user_responses)
		m = m.replace('], [', '],\n\t[').replace('[[', '[\n\t[').replace(']]', '],\n]\n')
		f = open(fn, 'w'); f.write(m); f.close()

		fn = self.data_file.rstrip('.dat') + '_predictions.txt'
		m = repr(self.predictions)
		m = m.replace('], [', '],\n\t[').replace('[[', '[\n\t[').replace(']]', '],\n]\n')
		f = open(fn, 'w'); f.write(m); f.close()
		
		
		vcstring = getattr(self, 'volctrl', {}).get('string', None)
		if vcstring != None:
			fn = os.path.realpath("ChannelVolumesDB.prm")
			print "writing ChannelVolumesDB parameter to "+fn
			open(fn, 'w').write(vcstring + '\n')
			
			
		self.chimes.play()
			
	#############################################################
	
	def UpdateChannelVolumes(self):
		self.player.pan = [1.0 for x in self.chanvol]
		for i,x in enumerate(self.chanvol):
			if x <= 0.0: x = 10.0**(x/20.0)
			self.player.pan[i] = x

	#############################################################
	
	def start_stimulus(self):
		self.lpt(255)
		self.remember('stream')
		
	#############################################################
	
	def finish_stimulus(self):
		self.lpt(0)
		self.change_phase()
		self.last.pop('stream', None)  # AAA
		#self.streamstart = None        # BBB
		#self.player.reset_timestamps() # BBB
	
	#############################################################

	def arm(self):
		if len(self.streams) == 0:
			self.factory.append(self.make, store=True)
	
	#############################################################

	def make(self, store=False):
		import cProfile, pstats
		prof = cProfile.Profile()
		a = prof.runcall(AudioStream.Stimulus, self.StimulusMaker, self.nexttarget(pop=False))
		self.lastprof = pstats.Stats(prof).sort_stats('time', 'cumulative')
		if store: self.streams.append(a)
		return a
		
	#############################################################
	
	def decide(self, predicted, target):
		self.predictions.append([predicted, target])
		self.last_prediction = predicted
		if target:
			if predicted == target: self.ding.play()
		
	#############################################################
	
	def reset_count(self):
		self.count = {
			'beats':    [0]*self.nstreams,
			'standards':[0]*self.nstreams,
			'targets':  [0]*self.nstreams,
			'deviants': [0]*self.nstreams,
		}

	#############################################################
	
	def update_count(self):
		encoding = {'standards':2, 'targets':3, 'deviants':4}
		streamstates = [0] * self.nstreams
		prev = getattr(self, 'previous_streamstates', streamstates)
		for istream in range(self.nstreams):
			streamstates[istream] = self.states['Stream%d'%(istream+1)]
			for k,v in encoding.items():
				if prev[istream] == v and streamstates[istream] != v:
					self.count[k][istream] += 1
					self.count['beats'][istream] += 1
		self.previous_streamstates = streamstates

	#############################################################
	
	def nexttarget(self, pop=True):
		if not hasattr(self, 'targetorder'): self.targetorder = []
		if len(self.targetorder) == 0:
			tpb = int(self.params['TrialsPerBlock'])
			while len(self.targetorder) < tpb: self.targetorder += range(1, self.nstreams+1)
			random.shuffle(self.targetorder)
		if pop: return self.targetorder.pop(0)
		else: return self.targetorder[0]
		
#################################################################
	def play(self, snd, side=None):		
		if side == None: side =random.choice(['left','right'])
		
		if   side == 'right': (snd.sound[:,:2] * [0,1]).play()
		elif side == 'left':  (snd.sound[:,:2] * [1,0]).play() 
		else:                 (snd.sound[:,:2] * [1,1]).play() 
		
#################################################################
#################################################################

