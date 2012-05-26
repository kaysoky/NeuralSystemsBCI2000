#!/usr/bin/python

__all__ = ['Text', 'Block', 'Disc', 'ImageStimulus']

import os
import sys
import numpy

if os.environ.get("SDL_VIDEODRIVER", "") == "":
	if sys.platform == "win32": os.environ["SDL_VIDEODRIVER"] = "windib"

import pygame
import pygame.gfxdraw
pygame.font.init()

import AppTools.Coords as Coords
import AppTools.CurrentRenderer as CurrentRenderer # TODO: remove dependency on this

try:    from BCI2000PythonApplication    import BciGenericRenderer, BciStimulus   # development copy
except: from BCPy2000.GenericApplication import BciGenericRenderer, BciStimulus   # installed copy

class PygameRenderer(BciGenericRenderer):

	def __init__(self):
		self.monofont = FindFont(
			('lucida console', 'monaco', 'monospace', 'courier new', 'courier')
		)
		self.coords = Coords.Box(left=100, top=100, width=300, height=300, sticky=True, anchor='top left')
		self.bgcolor = (0.5, 0.5, 0.5)
		self.framerate = 60.
		self.changemode = False
		self.frameless_window = False
		self.always_on_top = False
		self.title = 'stimuli'
		self.coordinate_mapping = 'pixels from lower left' # VisionEgg-like
		self.screen = None
		self._bci = None

	def setup(self, left = None, top = None, width = None, height = None,
		bgcolor = None, framerate = None, changemode = None,
		frameless_window = None, always_on_top = None, title=None,
		coordinate_mapping = None,
		**kwds):
		"""
		Call this to set certain commonly-defined parameters for the screen
		during BciApplication.Preflight(). The renderer object will read
		these parameters in order to initialize the stimulus window, before
		BciApplication.Initialize() is called.
		"""###
		# `**kwds` is used for compatibility with the `VisionEggRenderer`:
		# the `bitdepth` parameter is ignored.
		if left != None: self.coords.left = left
		if top != None: self.coords.top = top
		if width != None: self.coords.width = width
		if height != None: self.coords.height = height
		if bgcolor != None: self.bgcolor = bgcolor
		if framerate != None: self.framerate = framerate # TODO: unused
		if changemode != None: self.changemode = changemode
		if frameless_window != None: self.frameless_window = frameless_window
		if always_on_top != None: self.always_on_top = always_on_top
		if title != None: self.title = title
		if coordinate_mapping != None:  self.coordinate_mapping = coordinate_mapping
			
	def Initialize(self, bci=None):
		self._bci = bci
		pygame.display.quit()
		os.environ["SDL_VIDEO_WINDOW_POS"] = "%i,%i" % (int(self.coords.left), int(self.coords.top))
		pygame.display.init()
		pygame.display.set_caption(self.title)
		pygame.display.set_icon(pygame.image.load(os.path.join(Coords.__file__, '..', '..', '..', 'icon.bmp'))) # TODO - better way to locate
		flags = \
			(self.changemode and (pygame.FULLSCREEN | pygame.DOUBLEBUF)) | \
			(self.frameless_window and pygame.NOFRAME) | 0
		size = (int(self.coords.width), int(self.coords.height))
		self.screen = pygame.display.set_mode(size, flags)

		self.coords.sticky = True
		self.coords.anchor = 'top left'
		self.coords.position = [0,0]
		self.coords.size = [size[0], -size[1]]
		cm = self.coordinate_mapping.lower().replace('bottom', 'lower').replace('top', 'upper').replace(' ', '')
				
		if cm == 'pixelsfromlowerleft':
			#self.coords.internal = Coords.Box(left=0, bottom=0, width=size[0], height=size[1])
			pass # TODO: right now, setting internal seems to screw up the Box itself - clearly a bad bug in Coords
		elif cm == 'pixelsfromupperleft':
			#self.coords.internal = Coords.Box(left=0, top=0, width=size[0], height=-size[1])
			pass # TODO: right now, setting internal seems to screw up the Box itself - clearly a bad bug in Coords
		else:
			raise ValueError('coordinate_mapping "%s" is unsupported' % self.coordinate_mapping)
			
		print "done"
		print self.coords
			
		# windows-specific code
		try: import wm_ext  # TODO: third-party dependency
		except ImportError: pass
		else: wm_ext.EXT_AlwaysOnTop(self.always_on_top)

	def GetFrameRate(self):
		return self.framerate  # TODO: not the real framerate

	def RaiseWindow(self):
		try:
			import ctypes  # !! Windows-specific code.
			stimwin = ctypes.windll.user32.FindWindowA(0, pygame.display.get_caption()[0])
			self._bci._raise_window(stimwin)
		except:
			pass

	def GetEvents(self):
		return pygame.event.get()

	def DefaultEventHandler(self, event):
		return (event.type == pygame.QUIT) or (event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE)

	def StartFrame(self, objlist):
		bci = self._bci
		if bci: bci.ftdb(label='screen.clear')  #--------------------
		self.screen.fill(tuple([int(round(255 * x)) for x in self.bgcolor]))
		if bci: bci.ftdb(label='viewport.draw') #--------------------
		for obj in objlist: obj.draw(self.screen, self.coords)

	def FinishFrame(self):
		bci = self._bci		
		if bci: bci.ftdb(label='swap_buffers')  #--------------------
		pygame.display.flip()

	def SetDefaultFont(self, name = None, size = None):
		return SetDefaultFont(name=name, size=size)

	def GetDefaultFont(self):
		return GetDefaultFont()

	@property
	def size(self):
		try:
			info = pygame.display.Info()
		except:
			return (0,0)
		return Coords.Size((info.current_w, info.current_h))
	
	@property
	def width(self): return self.size[0]

	@property
	def height(self): return self.size[1]

	@apply
	def color():
		def fget(self):
			return self.bgcolor
		def fset(self, value):
			self.bgcolor = value
		return property(fget, fset)

	def Cleanup(self):
		self.screen = None
		pygame.display.quit()

BciGenericRenderer.subclass = PygameRenderer

class ImageStimulus(Coords.Box):
	def __init__(self, content=None, size=None, position=None, anchor='center',
		angle=0.0, smooth=True, color=(1,1,1,1), texture=None):
		
		Coords.Box.__init__(self)
		self._props = {}
		self.__filename = None
		self.__original_surface = None
		self.__content_changed = False
		self.__last_transformation = None
		self.__transformed_surface = None
		self.__last_coloring = None
		self.__colored_surface = None
		
		if content == None: content = texture
		if content == None:
			if size == None: size = Coords.Size((100,100))
			content = self.default_content(size)
			
		self.content = content
		
		if size == None: size = self.original_size
		
		if position == None:
			position = CurrentRenderer.get_screen().center()
		
		self.anchor = anchor
		self.position = position
		self.size = size	
			
		self.color = color
		self.angle = angle
		self.smooth = smooth
		self.flipx = False
		self.flipy = False
		
	def default_content(self, size):
		return pygame.Surface(size, flags=pygame.SRCALPHA)

		
	def transform(self, force=False):
		p = self._props
		srcsize  = tuple([int(round(x)) for x in self.original_size])
		dstsize  = tuple([int(round(x)) for x in self.size])
		angle = float(p['angle']) % 360.0
		smooth = bool(p['smooth'])
		changed = bool(self.__content_changed)
		flipx = bool(p['flipx'])
		flipy = bool(p['flipy'])
		color = tuple(p['color'])
		if len(color) == 3: color = color + (1.0,)
		athresh = 0.2 # degrees
		tr = (srcsize,dstsize,angle,flipx,flipy,smooth)
		pos = Coords.Point((self.left, self.bottom))
		# TODO: transform pos as necessary below: rotations should occur around anchor
		#       store transformed_pos with transformed
		if force or changed or tr != self.__last_transformation:
			t = self.__original_surface
			if flipx or flipy: t = pygame.transform.flip(t, flipx, flipy)
			if smooth:
				scaling = [float(dstsize[i]) / float(srcsize[i]) for i in (0,1)]
				proportional = (abs(scaling[0]-scaling[1]) < 1e-2)
				if dstsize != srcsize and not proportional: t = pygame.transform.smoothscale(t, dstsize)
				elif  dstsize != srcsize  and proportional: t = pygame.transform.rotozoom(t, angle, scaling[0])
				elif  abs(angle) > athresh:				 t = pygame.transform.rotozoom(t, angle, 1.0)
			else:
				if dstsize != srcsize: t = pygame.transform.scale(t, dstsize)
				if abs(angle) > athresh:   t = pygame.transform.rotate(t, -angle)
			self.__colored_surface = self.__transformed_surface = t
			changed = True
		self.__last_transformation = tr
		if force or changed or color != self.__last_coloring:
			t = self.__transformed_surface
			if color != (1.0,1.0,1.0,1.0):
				t = to_numpy(t)
				a = numpy.array(color[:t.shape[2]])
				a.shape = (1,1,a.size)
				t = to_surface(t * a)
			self.__colored_surface = t
		self.__last_coloring = color
		self.__content_changed = False
		return self.__colored_surface, pos

	def draw(self, screen, coords):
		t, pos = self.transform()  # TODO: anchor still doesn't work quite right
		#pos = pos.through(box=coords)
		pos.y = screen.get_height() - pos.y
		screen.blit(t, pos)

	@apply
	def original_size():
		def fget(self):
			orig = self.__original_surface
			if orig == None: return None
			return Coords.Size((orig.get_width(), orig.get_height()))
		return property(fget, doc="the width and height of the original image (read only)")
	
	@apply
	def content():
		def fget(self):
			return to_numpy(self.__original_surface)
		def fset(self, val):
			if isinstance(val, basestring):
				val = pygame.image.load(val)
				self.__filename = val
			else:
				val = to_surface(val)
			val = val.convert_alpha()
			self.__original_surface = val
			self.__content_changed = True
		return property(fget, fset, doc='the content of the image stimulus as a numpy array')

	@apply
	def color():
		def fget(self):  p = self._props; return p['color']
		def fset(self, val):
			p = self._props;
			try: val = [float(x) for x in val]
			except: raise ValueError('invalid color specification')
			if len(val) not in [3,4]: raise ValueError('color specification should have 3 or 4 elements')
			p['color'] = val
		return property(fget, fset, doc='3- or 4-element sequence denoting RGB or RGBA colour')
	
	@apply
	def angle():
		def fget(self):  p = self._props; return p['angle']
		def fset(self, val):
			try: val = float(val)
			except: raise TypeError('angle should be a floating-point scalar')
			p = self._props;
			p['angle'] = val % 360.0
		return property(fget, fset, doc='rotation angle in degrees')
	
	@apply
	def smooth():
		def fget(self):  p = self._props; return p['smooth']
		def fset(self, val):
			try: val = bool(val)
			except: raise TypeError('smooth should be a boolean')
			p = self._props;
			p['smooth'] = val
		return property(fget, fset, doc='whether or not pygame transformations are smooth')

	@apply
	def flipx():
		def fget(self):  p = self._props; return p['flipx']
		def fset(self, val):
			try: val = bool(val)
			except: raise TypeError('flipx should be a boolean')
			p = self._props;
			p['flipx'] = val
		return property(fget, fset, doc='whether to display image flipped left-to-right')
	
	@apply
	def flipy():
		def fget(self):  p = self._props; return p['flipy']
		def fset(self, val):
			try: val = bool(val)
			except: raise TypeError('flipy should be a boolean')
			p = self._props;
			p['flipy'] = val
		return property(fget, fset, doc='whether to display image flipped top-to-bottom')

class Disc(ImageStimulus):
	def __init__(self, position=(10,10), radius=10, size=None, color=(0,0,1), anchor='center', angle=0.0, smooth=True):
		if isinstance(radius, (float,int)): radius = (radius,radius)
		if size == None: size = [x * 2 for x in radius]
		if isinstance(size, (float,int)): size = (size,size)
		ImageStimulus.__init__(self, content=None, size=size, position=position, anchor=anchor, angle=angle, color=color, smooth=smooth)
		
	def default_content(self, size):
		size = [max(x,100) for x in size]
		surface = pygame.Surface(size, flags=pygame.SRCALPHA)
		x = int(size[0]/2)
		y = int(size[1]/2)
		pygame.gfxdraw.filled_ellipse(surface, x, y, x-1, y-1, (255,255,255,255))
		return surface
	
	@apply
	def radius():
		def fget(self):
			return sum(self.size)/float(len(self.size))
		def fset(self, val):
			if isinstance(val, (float,int)):
				self.size = [min(1,x) for x in self.size]
				prev = sum(self.size)/float(len(self.size))
				self.size *= val/prev
			else:
				self.size = [x*2 for x in val]
		return property(fget, fset, doc="radius of the circle")

class Block(ImageStimulus):
	def __init__(self, position=(10, 10), size=(10, 10), color=(0, 0, 1), anchor='center', angle=0.0, smooth=True):
		ImageStimulus.__init__(self, content=None, size=size, position=position, anchor=anchor, angle=angle, color=color, smooth=smooth)
		
	def default_content(self, size):
		surface = pygame.Surface(size, flags=pygame.SRCALPHA)
		surface.fill((255,255,255,255))
		return surface

class Text(ImageStimulus):
	def __init__(self, text='Hello world', font_name=None, font_size=None, position=(10,10), color=(1, 0, 0), anchor='center', angle=0.0, smooth=True):
		ImageStimulus.__init__(self, content=None, position=position, color=color, anchor=anchor, angle=angle, smooth=smooth)
		dfn,dfs = GetDefaultFont()
		font_name = dfn if font_name == None else font_name
		font_size = dfs if font_size == None else font_size
		p = self._props
		p['font_name'] = font_name
		p['font_size'] = font_size
		p['text'] = text
		p['value'] = None
		self.__font_changed = True
		self.__text_changed = True
	
	def transform(self, force=False):
		p = self._props
		if self.__font_changed:
			fn = FindFont(p['font_name'])
			if fn != None: self.__font_object = pygame.font.Font(fn, p['font_size'])
			self.__font_changed = False
			self.__text_changed = True
		if self.__text_changed:
			t = str(p['text'])
			if p['value'] != None:
				val = p['value']
				if isinstance(val, list): val = tuple(val)
				try: t = t % val
				except: pass
			self._ImageStimulus__original_surface = orig = self.__font_object.render(t, True, (255,255,255)) # TODO: multiline text....
			self.size = Coords.Size((orig.get_width(), orig.get_height()))
			self.__text_changed = False
			self.__content_changed = True
		return ImageStimulus.transform(self, force=force)
	
	@apply
	def value():
		def fget(self):  p = self._props; return p['value']
		def fset(self, val):
			if isinstance(val, (tuple,list,numpy.ndarray)): val = list(val)
			p = self._props;
			self.__text_changed = p['value'] != val
			p['value'] = val
		return property(fget, fset, doc='optional list of values for interpolation into text')
		
	@apply
	def text():
		def fget(self):  p = self._props; return p['text']
		def fset(self, val):
			if val == None or val == '': val = ' '
			p = self._props;
			self.__text_changed = p['text'] != val
			p['text'] = val
		return property(fget, fset, doc='text content')
		
	@apply
	def font_name():
		def fget(self):  p = self._props; return p['font_name']
		def fset(self, val):
			p = self._props;
			self.__font_changed = p['font_name'] != val
			p['font_name'] = val
		return property(fget, fset, doc='font name')
		
	@apply
	def font_size():
		def fget(self):  p = self._props; return p['font_size']
		def fset(self, val):
			p = self._props;
			self.__font_changed = p['font_size'] != val
			p['font_size'] = val
		return property(fget, fset, doc='font size')
	
	def _getAttributeNames(self):
		return self._props.keys()
	
def FindFont(fontnames):
	"""
	Tries to find a system font file corresponding to one of the
	supplied list of names. Returns None if no match is found.
	"""###
	def matchfont(fontname):
		if fontname.lower().endswith('.ttf'): return fontname
		bold = italic = False
		for i in range(2):
			if fontname.lower().endswith(' italic'): italic = True; fontname = fontname[:-len(' italic')]
			if fontname.lower().endswith(' bold'): bold = True; fontname = fontname[:-len(' bold')]
		return pygame.font.match_font(fontname, bold=int(bold), italic=int(italic))
		
	if not isinstance(fontnames, (list,tuple)): fontnames = [fontnames]
	fontnames = [f for f in fontnames if f != None]
	f = (filter(None, map(matchfont, fontnames)) + [None])[0]
	if f == None and sys.platform == 'darwin': # pygame on OSX doesn't seem even to try to find fonts...
		f = (filter(os.path.isfile, map(lambda x:os.path.realpath('/Library/Fonts/%s.ttf'%x),fontnames)) + [None])[0]
	return f
	
def SetDefaultFont(name = None, size = None):
	"""
	Set the name and/or size of the font that is used by
	default for Text stimuli. Returns True if the named font
	can be found, False if not.
	"""###
	if name != None:
		font = FindFont(name)
		if font == None: return False
		Text.default_font_name = font
	if size != None:
		Text.default_font_size = size	
	#pygame.font.Font(Text.default_font_name, Text.default_font_size) # would presumably throw an exception if invalid?
	return True
	
def GetDefaultFont():
	return Text.default_font_name, Text.default_font_size

SetDefaultFont(name=pygame.font.get_default_font(), size=20)

	
def to_surface(src):
	if isinstance(src, pygame.surface.Surface):
		return src
	elif isinstance(src, numpy.ndarray):
		if src.dtype in (numpy.float32, numpy.float64):
			src = src * 255.0 + 0.5
		if src.dtype != numpy.uint8 or not src.flags.carray:
			src = numpy.asarray(src, dtype=numpy.uint8, order='C')
		if src.ndim == 2: src = numpy.expand_dims(src, -1)
		if src.ndim != 3: raise NotImplementedError,"numpy array must be 2- or 3-dimensional"
		if src.shape[2] == 1: src = src.repeat(3, axis=2)
		if src.shape[2] == 3: format = 'RGB'
		elif src.shape[2] == 4: format = 'RGBA'
		else: raise NotImplementedError,"numpy array must be of extent 1, 3 or 4 in the third dimension"
		return pygame.image.fromstring(src.tostring(), (src.shape[1],src.shape[0]), format)
	else:
		return to_surface(to_numpy(src))
		
def to_numpy(src):
	# Ripped and adapted from VisionEgg.Textures VisionEgg 1.2.1 (c) by Andrew Straw
	if isinstance(src, numpy.ndarray):
		src = numpy.asarray(src)
	elif isinstance(src, pygame.surface.Surface):
		width, height = src.get_size()
		raw_data = pygame.image.tostring(src,'RGBA',1)
		arr = numpy.fromstring( raw_data, dtype=numpy.uint8 ) / 255.0
		arr.shape = (height,width,4)
		return arr[::-1]
	elif hasattr(src, 'tostring'):   # duck-type test for Image.Image
		width, height = src.size

		if src.mode == 'P':
			texel_data=src.convert('RGBA') # convert to RGBA from paletted
			data_format = 6408 # gl.GL_RGBA
		else:
			texel_data = src
	
		raw_data = texel_data.tostring('raw',texel_data.mode,0,-1)
		if texel_data.mode == 'L':
			shape = (height,width)
		elif texel_data.mode == 'RGB':
			shape = (height,width,3)
		elif texel_data.mode in ('RGBA','RGBX'):
			shape = (height,width,4)
		else:
			raise NotImplementedError('mode %s not supported'%(texel_data.mode,))
		arr = numpy.fromstring( raw_data, dtype=numpy.uint8 )
		arr.shape = shape
		return arr
	else:
		raise NotImplementedError("Don't know how to convert texel data %s to numpy array"%(src,))

