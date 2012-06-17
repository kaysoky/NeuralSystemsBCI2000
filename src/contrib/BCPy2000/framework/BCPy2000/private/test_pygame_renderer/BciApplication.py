
#import os; os.environ["SDL_VIDEODRIVER"] = "directx"
import PygameRenderer
		
#################################################################
#################################################################

class BciApplication(BciGenericApplication):
				
	#############################################################

	def Preflight(self, inprops):
		self.screen.setup(left=50,top=50,width=800,height=600,frameless_window=1, coordinate_mapping='pixels from center', always_on_top=1)
		
	#############################################################

	def Initialize(self, indim, outdim):
		self.mork()
		self.foo()
		self.bar()
		self.baz()
		self.bork()
		pass
				
	#############################################################
	
	def StartRun(self):
		for stim in self.stimuli.values():
			if isinstance(stim.obj, VisualStimuli.Movie): stim.rewind(); stim.play()

	#############################################################

	def Frame(self, phase):
		msec = self.since('run')['msec']
		#print msec
		for i,(k,v) in enumerate(self.stimuli.items()):
			angle = round(i + (i+1)/2.0 * 360.0 * msec / 2000.0) % 360
			try: v.angle = angle
			except: pass

	#############################################################
	def mork(self, name='mork', filename='delta.mpg'):
		try: import pygame.movie
		except: print 'cannot create a movie on this platform'
		else: return self.stimulus(name, VisualStimuli.Movie, filename=filename, position=(0,0))
	
	#############################################################
	
	def bork(self, name='bork'):
		s = self.stimulus(name, VisualStimuli.Text, position=(50,250))
		c = s.content; c[0,0,:] = 1.0; s.content = c
		return s
		
	#############################################################
	
	def baz(self, name='baz'):
		s = self.stimulus(name, VisualStimuli.Disc, position=(-100,300), size=(50,50))
		c = s.content; c[0,0,:] = 1.0; s.content = c
		return s
		
	#############################################################
	
	def bar(self, name='bar'):	
		import Image
		s = self.stimulus(name, VisualStimuli.ImageStimulus, texture=Image.new('RGBA', (50,100), (255,255,255,255)), position=(200,200))
		c = s.content; c[:20,:20,1] = 0.0; s.content = c
		return s
		
	#############################################################
	
	def foo(self, name='foo', filename='../RocketSMR/python/rocket.png', **kwargs):	
		kwargs['position'] = kwargs.get('position', (-200,-200))
		s = self.stimulus(name, VisualStimuli.ImageStimulus, texture=filename, **kwargs)	
		return s
	
	
#################################################################
#################################################################

