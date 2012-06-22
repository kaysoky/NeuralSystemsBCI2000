#import ConsoleRenderer
#import PygameRenderer
#import DummyRenderer

import os
import sys
import random
import numpy
import time
import WavTools
import AppTools.Displays
import AppTools.StateMonitors
import pygame.locals

		
#################################################################
#################################################################

class BciApplication(BciGenericApplication):
				
	#############################################################

	def Description(self):
		return "attention to auditory streams $Id$"

	#############################################################

	def Construct(self):
		self.require_version(41340)
		
		self.maxstreams = 2
		params = [
			"PythonApp:System  int       ScreenID=                                -1                     -1     % % // monitor id (0,1,2..., or -1 for last)",
			"PythonApp:System  float     WindowSize=                               0.8                    1.0   0 1 // subject window size from 0 to 1, when ScreenID is not -1",
			"PythonApp:System  float     FontSize=                                80                     80     5 % // font size for cue text",
			"PythonApp:System  float     SystemMasterVolume=                       1.0                    1.0   0 1 // operating-system volume setting from 0 to 1",
			"PythonApp:Task    int       HeadPhones=                               1                      0     0 1 // use headphones or not? (boolean)",
			"PythonApp:Task    int       TestEyeTracker=                           0                      0     0 1 // display gaze feedback stimulus? (boolean)",
			"PythonApp:Task    int       FreeChoice=                               0                      0     0 1 // allow user to choose freely? (boolean)",
			"PythonApp:Task    int       ExpectResponse=                           1                      1     0 1 // should the user press a key to indicate the target count? (boolean)",
			"PythonApp:Task    int       ShowCountFeedback=                        1                      0     0 1 // show correct counts after each trial? (boolean)",
			"PythonApp:Task    string    CountSounds=                              %                      %     % % // user's auditory feedback about the correct count ",
			"PythonApp:Task    int       ScreenIsForOperator=                      0                      0     0 1 // check this if the subject cannot see the screen (boolean)",
			"PythonApp:Task    int       InvertCount=                              0                      0     0 1 // check if the subject should count standards rather than oddballs (boolean)",
			"PythonApp:Task    intlist   BeatsPerTrial=                         1  7                      7     1 % // ",
			"PythonApp:Task    matrix    Cues=                                  { FocusOnText FocusOnAudio AnswerText AnswerAudio } 2    <<<%20LEFT RIGHT%20>>> % % NO YES ../sounds/application/no.wav ../sounds/application/yes.wav                  %     % % // ",
			"PythonApp:Task    matrix    SoundEffects=                          { Success EndOfRun } 1    ../sounds/application/ding.wav ../sounds/application/chimes.wav       %     % % // ",
			"PythonApp:Task    string    BackgroundSound=                          %                      %     % % // ",
			"PythonApp:Task    float     Hurry=                                    1.0                    1.0   0 5 // decrease to slow down pauses",
		]
		nbits = numpy.ceil(numpy.log2(self.maxstreams + 1.0))
		states = [
			"TargetStream    %d 0 0 0"%nbits,
			"PredictedStream %d 0 0 0"%nbits,
			"CueOn            1 0 0 0",
			"Response         4 0 0 0",
			"CorrectResponse  4 0 0 0",
		]		

		return params,states

	#############################################################

	def Preflight(self, sigprops):

		if 'NumberOfStreams' not in self.params: raise ValueError("the SignalProcessing module must define the NumberOfStreams parameter to be compatible with this module")
		if 'StreamingRequired' not in self.states: raise ValueError("the SignalProcessing module must define the StreamingRequired state to be compatible with this module")
		if 'StreamingFinished' not in self.states: raise ValueError("the SignalProcessing module must define the StreamingFinished state to be compatible with this module")
		if 'StimulusCode' not in self.states: raise ValueError("the SignalProcessing module must define the StimulusCode state to be compatible with this module")
		if 'StimulusVariant' not in self.states: raise ValueError("the SignalProcessing module must define the StimulusVariant state to be compatible with this module")
			
		self.nstreams = self.params['NumberOfStreams'].val
		if self.nstreams < 2: raise EndUserError, "NumberOfStreams must be at least 2"
		if self.nstreams > self.maxstreams: raise EndUserError, "NumberOfStreams exceeds current hard-coded limit of %d"%self.maxstreams
		
		self.nbeats = [x for x in self.params['BeatsPerTrial'].val]
		if len(self.nbeats) == 1: self.nbeats *= self.nstreams
		if len(self.nbeats) != self.nstreams: raise EndUserError("BeatsPerTrial parameter must have %d elements because NumberOfStreams=%d" % (self.nstreams,self.nstreams))
		for istream,n in enumerate(self.nbeats):
			if n < 1 or n != round(n): raise EndUserError("BeatsPerTrial elements must be integers > 0")
		
		if 'DirectSound' in self.params:
			if int(self.params['DirectSound']):
				import DirectSoundInterface
			elif 'DirectSoundInterface' in sys.modules and sys.modules['DirectSoundInterface'].loaded:
				raise EndUserError, "once turned on, the DirectSound setting cannot be turned off without restarting BCI2000"

		cues = self.params['Cues']
		if len(cues) != 4:
			raise EndUserError("Cues parameter must have 4 rows")
		if len(cues[0]) != self.nstreams:
			raise EndUserError("Cues parameter must have %d columns (one per stream)" % self.nstreams)
		
		self.FocusOnText = {0:'READY...'}
		self.FocusOnAudio = {0:WavTools.player(None)}
		self.AnswerText = {}
		self.AnswerAudio = {}
		for istream in range(self.nstreams):
			for irow in range(len(cues)):
				label = cues.matrixlabels()[0][irow]
				val = cues.val[irow,istream]
				if   label == 'FocusOnText':  self.FocusOnText[istream+1]  = val
				elif label == 'FocusOnAudio': self.FocusOnAudio[istream+1] = self.PrepareFeedback(val)
				elif label == 'AnswerText':   self.AnswerText[istream+1]   = val
				elif label == 'AnswerAudio':  self.AnswerAudio[istream+1]  = self.PrepareFeedback(val, delay=1.0)
				else: raise EndUserError("unrecognized row label '%s' in Cues matrix" % label)
		
		self.expect_response = int(self.params['ExpectResponse'])
		cs = self.params['CountSounds']
		self.count_feedback_audio = {}
		self.playcount = len(cs) > 0
		if self.playcount:
			for x in range(10):
				if os.path.isfile( cs % x ):
					self.count_feedback_audio[x] = self.PrepareFeedback( cs%x, lettercode='B' )
		
		
		
		monitor = int(self.params['ScreenID'])
		windowsize = float(self.params['WindowSize'])
		if AppTools.Displays.number_of_monitors() > 1 and monitor == -1: windowsize = 1.0
		AppTools.Displays.fullscreen(id=monitor, scale=windowsize)
		self.screen.setup(frameless_window=(windowsize==1.0), hide_mouse=(windowsize==1.0))
		self.screen.setup(frameless_window=True, hide_mouse=(windowsize==1.0))
		#self.screen.setup(coordinate_mapping='pixels from lower left') # for PygameRenderer
		
		self.hurry = float(self.params['Hurry'])

		if 'ConsoleRenderer' in sys.modules: self.screen.fake()
		
	#############################################################

	def Initialize(self, indim, outdim):
		
		self.screen.SetDefaultFont(size=int(self.params['FontSize']))
		self.transient('PredictedStream', manual=True) # for some reason this does not work (problem xxx)
		self.transient('Response', manual=True)
		self.transient('CorrectResponse', manual=True)

		self.screen.color = (0.2,0.2,0.2)

		txt = os.path.split(self.data_dir)[1]
		
		w,h = self.screen.size
		self.stimulus('cue', VisualStimuli.Text, text=txt, position=(w/2,h/2), anchor='center', on=True)
		
		txt = 'no classifier loaded'
		if len(self.params['ERPClassifierWeights'].val):
			try:
				d = ' '.join(self.params.SignalProcessingDescription.split(' ')[-2:])
				txt = 'weights created ' + d
				txt += ' (%s)' % self.InformalTime(d)
			except:
				txt = 'weights loaded (date unknown)'
			
		self.stimulus('weights', VisualStimuli.Text, text=txt, position=(w/2,h/2-50), anchor='center', on=True, font_size=24)
		
		self.ding = self.PrepareFeedback(self.params['SoundEffects']['Success']['1'], lettercode='F')
		self.chimes = self.PrepareFeedback(self.params['SoundEffects']['EndOfRun']['1'], lettercode='F')
		self.background_noise = self.PrepareFeedback(self.params['BackgroundSound'], lettercode='B')
		
		self.reset_count()
		self.count_feedback_stimuli = []
		for istream in range(self.nstreams):
			x = w/4 + (w/2) * istream/float(self.nstreams-1)
			t = VisualStimuli.Text(text='?', position=(x,h/2), anchor='center', on=False)
			stim = self.stimulus('count%d'%(istream+1), t)
			self.count_feedback_stimuli.append(stim)
		
		self.stimulus('answer', VisualStimuli.Text, position=(w-4,h-4), anchor='upperright', text=' ', on=False)
		
		if int(self.params['ShowSignalTime']):
			self.addphasemonitor()
			for istream in range(self.nstreams): self.addstatemonitor('Stream%d'%(istream+1))
			self.addstatemonitor('CurrentTrial')
			self.addstatemonitor('TargetStream')
			self.addstatemonitor('PredictedStream')
			
		elif int(self.params['ScreenIsForOperator']):
			self.addstatemonitor('CurrentTrial')
			for istream in range(self.nstreams):
				self.addstatemonitor('stream-%d targets'%(istream+1), func=self.ReportNumberOfTargets, kwargs={'istream':istream})
			
		self.freechoice = int(self.params['FreeChoice'])
		self.showcounts = int(self.params['ShowCountFeedback'])
		self.invertcount = int(self.params['InvertCount'])
		self.last_prediction = 0
		vol = float(self.params['SystemMasterVolume'])
		self.init_volume(vol)
				
		if int(self.params.TestEyeTracker):
			self.stimulus('eye', VisualStimuli.Text, text='o', anchor='center', on=False)
		
		self.logging = int(self.params.get('PythonAppShell', 1)) == 0 or self.params.get('PythonAppLog','') not in ['','-']
		
		if self.freechoice:
			if len(self.params.get('ERPClassifierWeights', [])) == 0:
				raise EndUserError("cannot enter FreeChoice mode without weights loaded")
			if int(self.params.get('PerceptualOnly', 0)):
				raise EndUserError("cannot enter FreeChoice mode if PerceptualOnly is true")
		
	#############################################################

	def ReportNumberOfTargets(self, istream):
		nt = self.count['targets'][istream]
		if self.invertcount: nt = self.nbeats[istream] - nt
		nt = str(nt)
		if istream+1 == self.states['TargetStream']: nt += ' <- should be counting this one'
		return nt
		
	#############################################################
	
	def PrepareFeedback(self, w, delay=0, lettercode='F', fs=None, returnWav=False):
		if w == '' or w == None: return WavTools.player(None)
		if isinstance(w, basestring): w = WavTools.wav(w)
		if fs != None and fs != w.fs: w = w.resample(fs)
		scparam = self.params.get('SoundChannels', None)
		if scparam != None and len(scparam)>= 2 and len(scparam[0]) >= 3:
			scparam = [[{'':'0'}.get(x,x) for x in row[:-1]] for row in scparam if row[-1].upper() == lettercode]
			ww = 0
			for q,row in enumerate(scparam):
				try: row = [float(x) for x in row]
				except: raise EndUserError("failed to interpret %s as floating-point numbers" % str(row))
				ww = ww + w[:,q%w.channels()] * row
			if len(scparam) == 0: w *=0; print "WARNING: no %s code specified in SoundChannels: sounds on channel %s will not be heard" % (lettercode,lettercode)
			else: w = ww
		w = delay % w
		if returnWav: return w
		return WavTools.player(w)
		
	#############################################################
	
	def MakeFocusOn(self,
	         pre=('ATT-Mike-FocusOn.wav', 'ATT-Crystal-FocusOn.wav'),
	         count=('ATT-Mike-AndCount.wav', 'ATT-Crystal-AndCount.wav'),
	         post=('ATT-Mike-ToSayNo.wav', 'ATT-Crystal-ToSayYes.wav'),
	         directory='../sounds/prompts'
	    ):
		ww = [None for i in range(self.nstreams)]
		if not isinstance(pre,  (list,tuple)): pre  = [pre]  * self.nstreams
		if not isinstance(count, (list,tuple)): count = [count] * self.nstreams
		if not isinstance(post, (list,tuple)): post = [post] * self.nstreams
		for istream in range(self.nstreams):
			w = 0
			promptcode = stimcode = 'S%d' % (istream+1)
			if 'B' in [row[-1] for row in self.params['SoundChannels']]: promptcode = 'B' # use the "background sound" channel to deliver audio prompts if the streaming output goes to non-audio devices
			standard = self.PrepareFeedback(self.params['StreamStimuli'][istream][0], lettercode=stimcode, returnWav=True)
			target   = self.PrepareFeedback(self.params['StreamStimuli'][istream][1], lettercode=stimcode, returnWav=True)
			filename = pre[istream]
			if filename not in (None,''):
				if directory != None: filename = os.path.join(directory, filename)
				w = w % self.PrepareFeedback(filename, lettercode=promptcode, fs=standard.fs, returnWav=True) % 0.2
			w = w % standard
			
			filename = count[istream]
			if filename not in (None,''):
				if directory != None: filename = os.path.join(directory, filename)
				w = w % 0.2 % self.PrepareFeedback(filename, lettercode=promptcode, fs=standard.fs, returnWav=True) % 0.2
				w = w % target % 0.2
				
			filename = post[istream]
			if filename not in (None,''):
				if directory != None: filename = os.path.join(directory, filename)
				w = w % 0.2 % self.PrepareFeedback(filename, lettercode=promptcode, fs=standard.fs, returnWav=True)
			w *= float(self.params['StreamVolumes'][istream])
			ww[istream] = w
		return ww
		
	#############################################################
	
	def StartRun(self):
		
		self.user_responses = []
		self.predictions = []
		self.target = 0
		self.targetorder = []
		self.background_noise.play(-1)
		self.stimuli['weights'].on = False
		if self.freechoice:
			self.operator('set button 1 Go "set state StreamingRequired 1"')
			self.operator('set button 2 Stop "set state StreamingRequired 0"')

	#############################################################
	
	def Phases(self):
		
		if self.freechoice: cuelen = None # 3000
		else:               cuelen = 2000
		
		if len(self.FocusOnAudio) > 1 and not self.freechoice:
			cuelen += 1000 * max([ self.FocusOnAudio[i].wav.duration() for i in range(1, self.nstreams+1)]) + 500 / self.hurry
		
		if self.expect_response: resplen = 5000
		else:                    resplen = 1000
		
		if self.showcounts or self.playcount: after_reponse = 'feedback'
		else:                                 after_reponse = 'pause'

		pauselen = numpy.random.rand() * 1000 + 500 / self.hurry
		thinklen = 500 + 500 / self.hurry
		
		self.phase(  duration=pauselen, name='pause',         next='cue',  )
		self.phase(  duration=cuelen,   name='cue',           next='stimulus',  )
		self.phase(  duration=None,     name='stimulus',      next='listen',  )
		self.phase(  duration=1000,     name='listen',        next='respond',   )    # listen for classification result and react to it when it arrives (give BCI feedback)
		self.phase(  duration=resplen,  name='respond',       next=after_reponse,  ) # tidy up wait for key press (healthy subjects counting in calibration mode)
		self.phase(  duration=2000,     name='feedback',      next='pause',  )       # give feedback about the counting task
		
		self.phase(  duration=thinklen, name='thinking-time', next='stimulus',  )
		
		self.design(start='pause', new_trial='cue')
		
	#############################################################

	def Transition(self, phase):
		
		self.states['CueOn'] = phase in ['cue']
		self.stimuli['cue'].on = phase in ['cue', 'stimulus', 'respond']

		if phase == 'pause':
			self.states['TargetStream'] = 0
			self.states['PredictedStream'] = 0 # necessary because of problem xxx
			self.target = self.states['TargetStream']
			
		if phase == 'cue':
			self.reset_count()
			self.stimuli['answer'].on = False
			self.stimuli['answer'].text = ' '
			if self.freechoice:
				self.states['TargetStream'] = 0
			else:
				self.states['TargetStream'] = self.nexttarget()
			self.target = self.states['TargetStream']
			self.stimuli['cue'].text = self.FocusOnText[self.target]
			self.stimuli['cue'].on = True
			self.FocusOnAudio[self.target].play()

		if phase == 'stimulus':
			self.reset_count()
			self.states['StreamingRequired'] = 1
			self.log('\n%04d-%02d-%02d %02d:%02d:%02d  Start stimuli for trial %d' % ((time.localtime()[:6])+(self.states['CurrentTrial'],)))
			if self.target: self.log('Focusing on stream %d' % self.target)
			self.stimuli['cue'].text = '+'
		
		# 'listen' phase is dealt with in self.Process()
		
		if phase == 'respond':
			if self.target > 0:
				correct = self.count['targets'][self.target-1]
				if self.invertcount: correct = self.nbeats[self.target-1] - correct
				self.states['CorrectResponse'] = correct
			else:
				self.states['CorrectResponse'] = 0
				
			if self.expect_response:
				self.stimuli['cue'].text = '?'
				self.screen.RaiseWindow()
			else:
				self.stimuli['cue'].on = False
				
		if phase == 'feedback':
			correct = self.states['CorrectResponse']
			response = self.states['Response']
			self.acknowledge('Response')
			self.acknowledge('CorrectResponse')
			
			if self.playcount:
				if correct:
					a = self.count_feedback_audio.get(correct, None)
					if a != None: a.play()
								
			if self.showcounts:
				if self.expect_response:
					self.user_responses.append([response, correct])
					if response == correct: self.log('Response correct')
					else: self.log('Response incorrect')
				for istream in range(self.nstreams):
					#nt = self.current_stream.ntargets[istream]
					nt = self.count['targets'][istream]
					if self.invertcount: nt = self.nbeats[istream] - nt
					stim = self.count_feedback_stimuli[istream]
					stim.text = str(nt)
					stim.on = True
					stim.color = (1,1,1)
					if istream+1 == self.target and correct != 0:
						if response == correct:
							stim.color = (0,1,0)
							if response: stim.text += ' is correct'
						else: 
							stim.color = (1,0,0)
							if response: stim.text += ', not %d' % response
			
								
		if phase not in ['feedback']:
			for istream in range(self.nstreams):
				self.count_feedback_stimuli[istream].on = False
		
	#############################################################

	def Process(self, sig):
			
		self.update_count()
		
		if self.in_phase('cue', 2) and self.states['StreamingRequired'] == 1:
			self.change_phase()
		
		if self.in_phase('stimulus'):
			if True not in [n < self.nbeats[istream] for istream,n in enumerate(self.count['beats'])]: 
				self.states['StreamingRequired'] = 0
			if self.states['StreamingRequired'] == 0:
				self.change_phase()
				
		if self.changed('StreamingFinished', fromVals=0):
			self.log(self.count)
			if self.nstreams == 2:
				result = numpy.sign(sig.flat[0])
				self.states['PredictedStream'] = {-1.0:1, 0.0:0, 1.0:2}.get(result)
			else:
				raise RuntimeError("do not know how to handle multiple streams") # TODO
		
		if self.in_phase('listen', 2):
			if self.states['PredictedStream']:
				self.decide(self.states['PredictedStream'], self.target)
				self.acknowledge('PredictedStream') # TODO: for some reason this does not work (problem xxx)
				if (self.freechoice or not self.expect_response) and self.last_prediction:
					self.stimuli['answer'].text = self.AnswerText.get(self.last_prediction, '??? %s ???' % str(self.last_prediction))
					self.stimuli['answer'].on = True
					if self.freechoice: self.stimuli['answer'].color = (1,1,1)
					else: self.stimuli['answer'].color = {True:(0,1,0),  False:(1,0,0)}.get( self.last_prediction == self.target )
				if self.freechoice and self.last_prediction and self.last_prediction in self.AnswerAudio:
					self.AnswerAudio[self.last_prediction].play()
				self.change_phase()
								
		if 'eye' in self.stimuli and 'EyetrackerLeftEyeGazeX' in self.states:
			x = 0.5 * self.states.EyetrackerLeftEyeGazeX + 0.5 * self.states.EyetrackerRightEyeGazeX
			y = 0.5 * self.states.EyetrackerLeftEyeGazeY + 0.5 * self.states.EyetrackerRightEyeGazeY
			x = x/65535.0 * self.screen.size[0]
			y = (1.0 - y/65535.0) * self.screen.size[1]
			self.stimuli.eye.on = True
			self.stimuli.eye.position = x,y
			
	#############################################################

	def Event(self, phase, event):
		if phase == 'respond' and int(self.params['ExpectResponse']) and event.type == pygame.locals.KEYUP:
			key = event.key
			if key in range(256, 266): self.states['Response'] = key - 256
			elif key in range(48,58):  self.states['Response'] = key - 48
			self.change_phase()
			self.log('Response: %d' % self.states['Response'])
		if self.freechoice and phase == 'cue' and event.type == pygame.locals.KEYUP:
			self.change_phase('thinking-time')
			
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
		
		for p in self.FocusOnAudio.values(): p.stop()
		for p in self.AnswerAudio.values(): p.stop()
		self.background_noise.stop()
		self.chimes.play()
		self.stimuli['cue'].text = 'system paused'
		self.stimuli['cue'].on = True
		
		if self.freechoice:
			self.operator('set button 1 "" ""')
			self.operator('set button 2 "" ""')
								
	#############################################################
	
	def decide(self, predicted, target):
		self.predictions.append([predicted, target])
		self.last_prediction = predicted
		self.log('BCI predicted stream %d' % predicted)
		if target:
			if predicted == target:
				self.ding.play()
				self.log('BCI correct')
			else:
				self.log('BCI incorrect')
		
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
		encoding = {1:'standards', 2:'targets', 3:'deviants'}
		
		if self.changed('StimulusCode', fromVals=0):
			istream = self.states['StimulusCode'] - 1
			variant = self.states['StimulusVariant']
			self.count['beats'][istream] += 1
			self.count[encoding.get(variant)][istream] += 1
			
	#############################################################
	
	def nexttarget(self, pop=True):
		if not hasattr(self, 'targetorder'): self.targetorder = []
		if len(self.targetorder) == 0:
			tpb = int(self.params['TrialsPerBlock'])
			while len(self.targetorder) < tpb: self.targetorder += range(1, self.nstreams+1)
			random.shuffle(self.targetorder)
		if pop: return self.targetorder.pop(0)
		else: return self.targetorder[0]
			
	#############################################################
	
	def ViewRun(self, fn=None, states='', **kwargs):
		ind = -1
		if isinstance(fn, int): fn,ind = None,fn
		if fn == None: fn = self.data_dir
		import BCI2000Tools.FileReader
		b = BCI2000Tools.FileReader.bcistream(filename=fn,ind=ind)
		x,st=b.decode('all')
		if isinstance(states, basestring): states = states.split()
		b.plotstates(st, 'TargetStream PredictedStream CorrectResponse Response'.split() + states, **kwargs)
		
	#############################################################
	
	def performance(self, condition=None, type='predictions', labels=None, directory=None):
		import SigTools
		if directory == None: directory = self.data_dir
		if condition != None: directory = directory[:-3] + ('%03d'% condition)
		files = [os.path.join(directory,x) for x in os.listdir(directory) if x.endswith('_' + type + '.txt')]
		arrays = [numpy.array(eval('\n'.join(open(x).readlines()))) for x in files] 
		confmat = 0
		if labels == None: labels = {'predictions':[1,2], 'responses':[1,2,3]}[type]
		for f,a in zip(files,arrays):
			print f
			if len(a) == 0 or numpy.all(a[:,1]==0):
				print "no entries"
			else:
				target = a[:,1]
				achieved = a[:,0]
				good = [int(x in labels) for x in achieved]
				achieved *= good
				if False in good: print "%d bad trials" % (len(good)-sum(good))
				c,x = SigTools.confuse(a[:,1],a[:,0], labels=[0]+labels)
				pc,stc = SigTools.class_loss(confusion_matrix=c)
				pb,stb = SigTools.balanced_loss(confusion_matrix=c)
				print c
				print ' overall accuracy of %s = %3.1f%% +/- %3.1f from %d trials' % (type, 100-100*pc, 100*stc, c.sum())
				print 'balanced accuracy of %s = %3.1f%% +/- %3.1f from %d trials' % (type, 100-100*pb, 100*stb, c.sum())
				confmat = confmat + c
			print
		print "collated over %s" % directory
		c = confmat
		if c is 0:
			print "no relevant entries found"
		else:			
			pc,stc = SigTools.class_loss(confusion_matrix=c)
			pb,stb = SigTools.balanced_loss(confusion_matrix=c)
			print confmat
			print ' overall accuracy of %s = %3.1f%% +/- %3.1f from %d trials' % (type, 100-100*pc, 100*stc, c.sum())
			print 'balanced accuracy of %s = %3.1f%% +/- %3.1f from %d trials' % (type, 100-100*pb, 100*stb, c.sum())
	
	#############################################################
	
	def log(self, string):
		if self.logging: print string
			
	#############################################################
		
	def InformalTime(self, d):
		try: then = time.strptime( d, '%Y-%m-%d %H:%M:%S' )
		except: return '-'
		now = time.localtime()
		seconds = float( time.mktime( now ) ) - float( time.mktime( then ) )
		day  = int( time.mktime( then ) / ( 60.0 * 60.0 * 24.0 ) ) 
		today = int( time.mktime( now ) / ( 60.0 * 60.0 * 24.0 ) )
		days = today - day
		weeks = days / 7.0
		years = days / 365.25
		months = years * 12.0
		
		if   seconds < 50.0: return '%d seconds ago' % round( seconds )
		elif seconds < 90.0: return 'about a minute ago'
		elif seconds < 50*60.0: return '%d minutes ago' % round( seconds / 60.0 )
		elif seconds < 90*60.0: return 'about an hour ago'
		elif days == 0: return '%d hours ago' % round( seconds / 3600.0 )
		elif days == 1: return 'yesterday'
		elif days < 31: return '%d days ago' % days
		elif round(months) == 1: return 'about a month ago'
		elif months < 21: return  'about %d months ago' % round( months )
		elif round(years) == 1: return 'about a year ago'
		else: return  'about %d years ago' % round( years )

#################################################################
#################################################################

