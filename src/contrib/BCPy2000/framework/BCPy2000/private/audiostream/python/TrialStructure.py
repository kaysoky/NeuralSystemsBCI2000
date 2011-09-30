#import ConsoleRenderer

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
		return "attention to auditory streams $Id $"

	#############################################################

	def Construct(self):
		self.maxstreams = 2
		params = [
			"PythonApp:System  int       ScreenID=                                -1                     -1     % % // monitor id (0,1,2..., or -1 for last)",
			"PythonApp:System  float     WindowSize=                               0.8                    1.0   0 1 // subject window size from 0 to 1, when ScreenID is not -1",
			"PythonApp:System  float     FontSize=                                80                     80     5 % // font size for cue text",
			"PythonApp:System  float     SystemMasterVolume=                       1.0                    1.0   0 1 // operating-system volume setting from 0 to 1",
			"PythonApp:Task    int       HeadPhones=                               1                      0     0 1 // use headphones or not? (boolean)",
			"PythonApp:Task    int       TestEyeTracker=                           0                      0     0 1 // display gaze feedback stimulus? (boolean)",
			"PythonApp:Task    int       FreeChoice=                               0                      0     0 1 // allow user to choose freely? (boolean)",
			"PythonApp:Task    int       ShowCountFeedback=                        1                      0     0 1 // show correct counts after each trial? (boolean)",
			"PythonApp:Task    intlist   BeatsPerTrial=                         1  7                      7     1 % // ",
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

		monitor = int(self.params['ScreenID'])
		windowsize = float(self.params['WindowSize'])
		if AppTools.Displays.number_of_monitors() > 1 and monitor == -1: windowsize = 1.0
		AppTools.Displays.fullscreen(id=monitor, scale=windowsize)
		self.screen.setup(frameless_window=(windowsize==1.0), hide_mouse=(windowsize==1.0))

		if 'ConsoleRenderer' in sys.modules: self.screen.fake()
		
	#############################################################

	def Initialize(self, indim, outdim):
		
		self.screen.SetDefaultFont(size=int(self.params['FontSize']))
		self.transient('PredictedStream', manual=True) # for some reason this does not work (problem xxx)
		self.transient('Response', manual=True)
		self.transient('CorrectResponse', manual=True)

		self.screen.color = (0.2,0.2,0.2)

		if int(self.params['HeadPhones']): starttext = 'HEADPHONES'
		else: starttext = 'SPEAKERS'
		
		if len(self.params['ERPClassifierWeights'].val) == 0:
			starttext += '   no classifier loaded'

		w,h = self.screen.size
		t = VisualStimuli.Text(text=starttext, position=(w/2,h/2), anchor='center', on=True)
		self.stimulus('cue', t)
		
		self.ding = self.PrepareFeedback('ding.wav')
		self.chimes = self.PrepareFeedback('chimes.wav')
		self.answers = {
			'YES': self.PrepareFeedback(1 % WavTools.wav('yes.wav')/2.0),
			'NO':  self.PrepareFeedback(1 % WavTools.wav('no.wav')/2.0),
		}
		
		self.reset_count()
		self.count_feedback_stimuli = []
		for istream in range(self.nstreams):
			x = w/4 + (w/2) * istream/float(self.nstreams-1)
			t = VisualStimuli.Text(text='?', position=(x,h/2), anchor='center', on=False)
			stim = self.stimulus('count%d'%(istream+1), t)
			self.count_feedback_stimuli.append(stim)
		
		if int(self.params['ShowSignalTime']):
			self.addphasemonitor()
			for istream in range(self.nstreams): self.addstatemonitor('Stream%d'%(istream+1))
			self.addstatemonitor('CurrentTrial')
			self.addstatemonitor('TargetStream')
			self.addstatemonitor('PredictedStream')
						
		self.freechoice = int(self.params['FreeChoice'])
		self.showcounts = int(self.params['ShowCountFeedback'])
		self.last_prediction = 0
		vol = float(self.params['SystemMasterVolume'])
		self.init_volume(vol)
				
		if int(self.params.TestEyeTracker):
			self.stimulus('eye', VisualStimuli.Text, text='o', anchor='center', on=False)
		
	#############################################################
	
	def PrepareFeedback(self, w):
		if isinstance(w, basestring): w = WavTools.wav(w)
		scparam = self.params.get('SoundChannels', None)
		if scparam != None and len(scparam)>= 2 and len(scparam[0]) >= 3:
			scparam = [[{'':'0'}.get(x,x) for x in row[:-1]] for row in scparam if row[-1].upper() == 'F']
			ww = 0
			for q,row in enumerate(scparam):
				try: row = [float(x) for x in row]
				except: raise EndUserError("failed to interpret %s as floating-point numbers" % str(row))
				ww = ww + w[:,q%w.channels()] * row
			if len(scparam) == 0: w *=0
			else: w = ww
				
		return WavTools.player(w)
		
	#############################################################
	
	def StartRun(self):
		
		self.user_responses = []
		self.predictions = []
		self.target = 0
		self.targetorder = []

	#############################################################
	
	def Phases(self):
		
		if self.freechoice:
			cuelen = None # 3000
			resplen = 2000
		else:
			cuelen = 2000
			resplen = 5000
		
		if self.showcounts:
			after_reponse = 'feedback'
		else:
			after_reponse = 'pause'

		pauselen = numpy.random.rand() * 1000 + 500
		
		self.phase(  duration=pauselen, name='pause',      next='cue',  )
		self.phase(  duration=cuelen,   name='cue',        next='stimulus',  )
		self.phase(  duration=None,     name='stimulus',   next='classify',  )
		self.phase(  duration=1000,     name='classify',   next='respond',   )
		self.phase(  duration=resplen,  name='respond',    next=after_reponse,  )
		self.phase(  duration=2000,     name='feedback',   next='pause',  )
		
		self.design(start='pause', new_trial='cue')
		
	#############################################################

	def Transition(self, phase):
		
		self.states['CueOn'] = phase in ['cue']
		
		if phase == 'pause':
			self.states['TargetStream'] = 0
			self.states['PredictedStream'] = 0 # necessary because of problem xxx
			self.target = self.states['TargetStream']
			
		if phase == 'cue':
			if self.freechoice:
				self.states['TargetStream'] = 0
			else:
				self.states['TargetStream'] = self.nexttarget()
			self.target = self.states['TargetStream']
			self.stimuli['cue'].text = {0:'READY...', 1:'<<< LEFT', 2:'RIGHT >>>'}.get(self.target, 'stream #%d'%self.target)
			self.stimuli['cue'].on = True
		elif phase == 'respond':
			if self.target > 0:
				self.states['CorrectResponse'] = self.count['targets'][self.target-1]
			else:
				self.states['CorrectResponse'] = 0
			if self.freechoice and self.last_prediction:
				answer = {1:'NO', 2:'YES'}.get(self.last_prediction, '?')
				self.stimuli['cue'].text = answer
				if answer in self.answers: self.answers[answer].play()
			else:
				self.stimuli['cue'].text = '?'
				self.screen.RaiseWindow()
			self.stimuli['cue'].on = True
		else:
			self.stimuli['cue'].on = True
			self.stimuli['cue'].text = '+'
		
		if phase == 'stimulus':
			self.reset_count()
			self.states['StreamingRequired'] = 1
		
		if phase == 'feedback':
			correct = self.states['CorrectResponse']
			response = self.states['Response']
			self.acknowledge('Response')
			self.acknowledge('CorrectResponse')
			self.user_responses.append([response, correct])
			
			for istream in range(self.nstreams):
				#nt = self.current_stream.ntargets[istream]
				nt = self.count['targets'][istream]
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
		else:
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
			if self.nstreams == 2:
				result = numpy.sign(sig.flat[0])
				self.states['PredictedStream'] = {-1.0:1, 0.0:0, 1.0:2}.get(result)
			else:
				raise RuntimeError("do not know how to handle multiple streams") # TODO
		
		if self.in_phase('classify', 2):
			if self.states['PredictedStream']:
				self.decide(self.states['PredictedStream'], self.target)
				self.acknowledge('PredictedStream') # TODO: for some reason this does not work (problem xxx)
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
		if phase == 'respond' and event.type == pygame.locals.KEYUP:
			key = event.key
			if key in range(256, 266): self.states['Response'] = key - 256
			elif key in range(48,58):  self.states['Response'] = key - 48
			self.change_phase()
		if self.freechoice and phase == 'cue' and event.type == pygame.locals.KEYUP:
			self.change_phase()
			
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
			
		self.chimes.play()
		self.stimuli['cue'].on = False
								
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
			
#################################################################
#################################################################

