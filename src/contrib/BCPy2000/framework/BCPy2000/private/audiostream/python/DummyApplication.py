import numpy
import VisionEgg

#################################################################
#################################################################

class BciApplication(BciGenericApplication):
				
	#############################################################

	def Description(self):
		return "I bet you won't bother to change this to reflect what the application is actually doing"

	#############################################################

	def Construct(self):
		params = [
		]
		states = [
			"SomeState 1 0 0 0",
		]
		return params,states

	#############################################################

	def Preflight(self, sigprops):
		# Here is where you would set VisionEgg.config parameters
		# In particular, if you don't fancy using the VisionEgg GUI
		# every time, do this:
		VisionEgg.config.VISIONEGG_GUI_INIT = 0
				
	#############################################################

	def Initialize(self, indim, outdim):
		# Set up stimuli. Visual stimuli use calls to
		# self.stimulus(). Attach whatever you like as attributes
		# of self, for easy access later on. Don't overwrite existing
		# attributes, however:  using names that start with a capital
		# letter is a good insurance against this.
		
		self.screen.SetDefaultFont('comic sans ms', 30)
		w,h = VisionEgg.config.VISIONEGG_SCREEN_W,VisionEgg.config.VISIONEGG_SCREEN_H
		t = VisionEgg.Text.Text(text='BCPy2000: Python bindings for your brain', position=(w/2,h/2), anchor='top')
		self.stimulus('SomeText', t)
		
	#############################################################
	
	def StartRun(self):
		pass
 			
	#############################################################
	
	def Phases(self):
		# define phase machine using calls to self.phase and self.design
		self.phase(name='flip', next='flop', duration=2000)
		self.phase(name='flop', next='flip', duration=2000)
 		self.design(start='flip')
 		
	#############################################################

	def Transition(self, phase):
		# present stimuli and update state variables to record what is going on
		if phase == 'flip':
			p = self.stimuli['SomeText'].anchor = 'top'
			self.states['SomeState'] = 1
		if phase == 'flop':
			p = self.stimuli['SomeText'].anchor = 'bottom'
			self.states['SomeState'] = 0
		
	#############################################################

	def Process(self, sig):
		# process the new signal packet
		pass

	#############################################################

	def Frame(self, phase):
		# update stimulus parameters if they need to be animated on a frame-by-frame basis
		red = 0.5 + 0.5 * numpy.sin(2.0 * numpy.pi * 0.5 * self.since('run')['msec']/1000.0)
		self.screen.color = (red, 0, 0)
		
	#############################################################

	def Event(self, phase, event):
		pass
		
	#############################################################

	def StopRun(self):
		pass
		
#################################################################
#################################################################
