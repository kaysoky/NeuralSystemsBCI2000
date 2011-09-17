from AppTools.Boxes import box, point, size
from AppTools.Displays import fullscreen, number_of_monitors
from AppTools.Shapes import PolygonTexture, Disc

import numpy
import pygame
import WavTools

class BciApplication(BciGenericApplication):
	
	def Construct(self):
		self.define_param("PythonApp floatlist Angles=          4    0 90 180 270    0      %     %     // ")
		self.define_param("PythonApp float     Eccentricity=         0.8             0.8    0     1     // ")
		self.define_param("PythonApp float     InvalidProbability=   0.1             0.1    0     1     // ")
		self.define_param("PythonApp float     NeutralProbability=   0.0             0.1    0     1     // ")

		self.define_param("PythonApp floatlist BackgroundColor= 3    0.5 0.5 0.5     0.0    0.0   1.0   // ")
		self.define_param("PythonApp floatlist FixationColor=   3    0.0 0.0 0.0     0.0    0.0   1.0   // ")
		self.define_param("PythonApp floatlist CueColor=        3    1.0 0.0 0.0     0.0    0.0   1.0   // ")
		self.define_param("PythonApp floatlist TargetColor=     3    0.0 0.0 1.0     0.0    0.0   1.0   // ")
		
		self.define_param("PythonApp int       WindowSize=           0.8             0.8    0.0   1.0   // ")
		
		self.define_state("CorrectAngle 9 0 0 0")
		self.define_state("CuedAngle    9 0 0 0")
		self.define_state("Invalid      1 0 0 0")
		self.define_state("Neutral      1 0 0 0")
		self.define_state("ResponseTimeMsec  16 0 0 0")
		
	def Phases(self):
		
		rd = numpy.random.randint
		self.phase(name='leadin',   duration=3000, next='fixation')
		self.phase(name='fixation', duration=1000, next='cue')
		self.phase(name='cue',      duration=1000, next='delay')
		self.phase(name='delay',    duration=rd(2000,3000), next='target')
		self.phase(name='target', duration=5000, next='pause')
		self.phase(name='pause',    duration=rd(1000,2000), next='fixation')
		
		self.design(start='leadin', new_trial='fixation')
	
	def Preflight(self, inprops):
		windowsize = float(self.params['WindowSize'])
		if number_of_monitors() > 1: windowsize = 1.0
		fullscreen(id=-1, scale=windowsize)
		self.screen.setup(frameless_window=(windowsize==1.0))
		
	def Initialize(self, indims, outdims):

		scrw,scrh = self.screen.size
		center=point((scrw/2.0,scrh/2.0))
		scrsiz = min(scrw,scrh)
		
		b = box(size=(scrw*0.1,scrh*0.05), position=center, sticky=True, anchor='center')
		vert = ((0.22,0.35),(0,0.35),(0.5,0),(1,0.35),(0.78,0.35),(0.78,0.75),(0.22,0.75),)
		vert = ((0.65,0.22),(0.65,0),(1,0.5),(0.65,1),(0.65,0.78),(0.25,0.78),(0.25,0.22),)
		self.screen.color = self.params.BackgroundColor.val
		self.stimulus('fixation', VisualStimuli.Text, text='+', font_size=200, position=center, anchor='center', color=self.params.FixationColor.val, on=True)
		self.stimulus('arrow', PolygonTexture, frame=b, vertices=vert, position=center, color=self.params.CueColor.val, on=False)
		self.stimulus('target', Disc, position=center, radius=scrsiz*0.05, color=self.params.TargetColor.val, on=False)
		
		ntrials = int(self.params.TrialsPerBlock)
		self.angles = list(self.params['Angles'].val) 
		self.angles *= max(1, ntrials/len(self.angles))
		numpy.random.shuffle(self.angles)
		
		self.neutral=False
		self.center = center
		self.eccentricity = float(self.params.Eccentricity) * scrsiz / 2.0
		self.ding = WavTools.player('ding.wav')
		
	def Transition(self, phase):
		
		self.stimuli.fixation.on = phase in ['fixation', 'delay', 'target']
		self.stimuli.arrow.on = phase in ['cue'] and not self.neutral
		self.stimuli.target.on = phase in ['target']
	
		if phase == 'fixation':
			self.states.CorrectAngle = self.angles.pop(0)
			
			self.neutral = False
			self.invalid = False
			r = numpy.random.random()
			r -= self.params.InvalidProbability.val
			if r < 0: 
				self.invalid = True
			else:
				r -= self.params.NeutralProbability.val
				if r < 0: self.neutral = True
			
			if self.invalid:
				angles = list(self.params.Angles.val)
				numpy.random.shuffle(angles)
				self.states.CuedAngle = angles[0]
			else:
				self.states.CuedAngle = self.states.CorrectAngle
			
			self.stimuli.arrow.angle = self.states.CorrectAngle
			theta = self.states.CuedAngle * numpy.pi / 180.0
			self.stimuli.target.position = self.center + self.eccentricity * point([numpy.cos(theta),numpy.sin(theta)])
			self.ding.play()
	
	def Event(self, phase, event):		
		if phase == 'target' and event.type == pygame.locals.KEYDOWN:
			rt = self.since('transition')['msec']
			self.states.ResponseTime = max(1, min(65535, int(round(rt))))
			self.change_phase()
