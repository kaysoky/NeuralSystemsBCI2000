__all__ = ['ConsoleRenderer', 'Text', 'Block', 'Disc']

import numpy
from AppTools.Boxes import box

try: from Console import Console
except: raise ImportError, "need to install the Console package for Python, by Fredrik Lundh: http://effbot.org/downloads/#console"

try:    from BCI2000PythonApplication    import BciGenericRenderer # development copy
except: from BCPy2000.GenericApplication import BciGenericRenderer # installed copy

#################################################################
#################################################################

class AsciiArt(object):
	def __init__(self, text='*', position=(0,0), anchor='center', size=None, color=(1,1,1), transparent='', on=True, scale=1, font_name=None, font_size=None):
		self.text = text
		self.position = numpy.asarray(position, dtype=numpy.float64)
		self.anchor = anchor
		if size != None: size = numpy.asarray(size, dtype=numpy.float64)
		self.size = size
		self.color = numpy.asarray(color, dtype=numpy.float64)
		self.transparent = transparent
		self.on = on
		self.scale = scale
		# ignore font_name, font_size
		
	def __getattr__(self, key):
		if key == 'parameters': return self # pretend to be a VisionEgg stimulus if anyone asks
		if not hasattr(self, key): raise AttributeError, "'%s' object has no attribute '%s'" % (self.__class__.__name__, key)

#################################################################
#################################################################

default_size = {} # duck-food
		
#################################################################
#################################################################

class ConsoleRenderer(BciGenericRenderer):
	def __init__(self, anchor='lower left'):
		self.size = numpy.array((0,0), dtype=numpy.float64)
		self.bgcolor = numpy.array((0,0,0), dtype=numpy.float64)
		self.colors = numpy.array([[0.25,0.25,0.25], [0,0,1], [0,1,0], [0,1,1], [1,0,0], [1,0,1], [1,1,0], [1,1,1]], dtype=numpy.float64)
		self.colors = numpy.r_[self.colors*0.75,self.colors]
		self.colors[0,:] = 0
		self.cursor = 0
		self.anchor = anchor
						
	def __setattr__(self, key, val):
		if isinstance(self.__dict__.get(key), numpy.ndarray): val = numpy.asarray(val)
		self.__dict__[key] = val
		if key == 'bgcolor': self.__dict__['color'] = val
		if key == 'color': self.__dict__['bgcolor'] = val
		
	def match_color(self, c):
		return ((numpy.matrix(c).A - self.colors)**2).sum(axis=1).argmin()
			
	def Initialize(self, bci):
		self._bci = bci
		setup = getattr(self, '_setup', {})
		self.console = Console()
		cw,ch = self.console.size()
		self.box = box(left=0, top=0, right=cw, bottom=ch, anchor='upper left') # console coordinates
		sw,sh = setup.get('width'),setup.get('height')
		if sw == None: sw = default_size.get('width', cw)
		if sh == None: sh = default_size.get('height', ch)
		self.size = (sw,sh)
		self.box.internal = box(position=(0,0), size=self.size, anchor=self.anchor) # init_screen coordinates
		self.framerate = setup.get('framerate')
		if self.framerate == None: self.framerate = 4.0
		
	def GetFrameRate(self):
		return float(self.framerate)
	
	def RaiseWindow(self):
		try:
			import ctypes  # !! Windows-specific code.
			stimwin = ctypes.windll.user32.FindWindowA(0, "PythonApp")
			self._bci._raise_window(stimwin)
		except:
			pass
	
	def StartFrame(self, objlist):
		bgcolor = self.match_color(self.bgcolor) * 16
		self.console.page(bgcolor, ' ')
		self.cursor = 0
		self.box.right,self.box.bottom = self.console.size()
		for a in objlist:
			if not a.on: continue
			lines = a.text.split('\n')
			nr = len(lines)
			nc = max([len(x) for x in lines])
			position = self.box.map(a.position, 'position')
			if a.size == None: size = (nc,nr)
			else: size = self.box.map(a.size, 'size')
			size = numpy.asarray(size) * float(a.scale)
			b = box(left=0,top=0,right=nc,bottom=nr, anchor='upperleft') # stimulus content
			b.internal = box(position=position, width=size[0], height=-size[1], anchor=a.anchor) # console coordinates
			color = self.match_color(a.color)
			for y in numpy.arange(b.internal.top, b.internal.bottom):
				for x in numpy.arange(b.internal.left, b.internal.right):
					c,r = map(lambda x: int(x+0.01), b.map((x,y), 'position'))
					if r < 0 or r >= len(lines): continue
					line = lines[r]
					if c < 0 or c >= len(line): continue
					ch = line[int(c)]
					if not ch in a.transparent:
						self.console.text(int(x),int(y), ch, int(color + bgcolor))


	def p(self, *pargs, **kwargs):
		if len(pargs): self.write(' '.join([str(x) for x in pargs]))
		for k,v in kwargs.items(): self.write('%s = %s' % (str(k),repr(v)))
	def write(self, s):
		for x in s.split('\n'): self.console.text(0, self.cursor, x, 7); self.cursor += 1

# and now, some fun with duck-typing:

	def __getattr__(self, key):
		if key == 'parameters': return self
		if not hasattr(self, key): raise AttributeError, "'%s' object has no attribute '%s'" % (self.__class__.__name__, key)

	def get_size(self):
		return self.size # quack like a VisionEgg.Core.Screen

	def fake(self):
		"""
		Defers opening the console until you call self.screen.real()
		Call Fake during Preflight. Works best if you also specify the screen
		size during Preflight by calling self.init_screen(), and then work
		with stimuli in the units of the coordinates you have specified.
		"""###
		global Console
		Console = FakeConsole

	def real(self):
		if isinstance(self.console, FakeConsole): import Console; self.console = Console.Console()

#################################################################
#################################################################

class FakeConsole(object):
	def page(self, *pargs): pass
	def text(self, *pargs): pass
	def size(self): return (125,50)
	
#################################################################
#################################################################

BciGenericRenderer.subclass = ConsoleRenderer

Text = AsciiArt
Block = AsciiArt
Disc = AsciiArt

#################################################################
#################################################################
