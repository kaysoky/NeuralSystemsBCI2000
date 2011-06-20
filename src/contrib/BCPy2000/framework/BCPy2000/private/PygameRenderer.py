__all__ = ['Text', 'Block', 'Disc', 'ImageStimulus']

import os, sys
import numpy
import pygame
import pygame.locals

pygame.init()

import AppTools.Coords  as Coords 
import AppTools.CurrentRenderer as CurrentRenderer


try:    from BCI2000PythonApplication    import BciGenericRenderer,BciStimulus   # development copy
except: from BCPy2000.GenericApplication import BciGenericRenderer,BciStimulus   # installed copy

#################################################################
#################################################################

Block = Disc = None

class PygameRenderer(BciGenericRenderer):
	"""
	This is a subclass of BciGenericRenderer that renders stimuli, and
	polls for mouse and keyboard events, via pygame.  It is one example
	of a custom renderer with which one can replace BCPy2000's default
	VisionEggRenderer (see the documentation for the BciGenericRenderer
	class).
		
	The following attributes (only accessible after the window has opened)
	are most useful:
	
		.size    (read-only) contains the window's (width,height) in pixels.
		.bgcolor is used to get and set the background colour of the window.
		.color   is an alias for bgcolor.
		
	"""###

	#############################################################

	def __init__(self):
		self.__dict__['monofont'] = self.findfont(('lucida console', 'monaco', 'monospace', 'courier new', 'courier'))
		self.__dict__['_defaultfont'] = self.__dict__['monofont']
		self.__dict__['_defaultfontsize'] = 16
		self.__dict__['_windowtitle'] = 'stimuli'
		self.__dict__['_screen'] = None
		self.__dict__['_bgcolor'] = (0.5,0.5,0.5)
		
	@apply
	def color():
		def fget(self): return self.__dict__.get('_bgcolor')
		def fset(self, val): self.__dict__['_bgcolor'] = val
		return property(fget, fset, doc="screen background RGB color: a sequence of 3 floats in the range 0.0 to 1.0")

	bgcolor = color;
	
	@apply
	def size():
		def fget(self):
			try: info = pygame.display.Info()
			except: return (0,0)
			else: return Coords.Size((info.current_w, info.current_h))
		return property(fget, doc="the current screen or window size (read-only)")
		
	
	#############################################################
	
	def findfont(self, fontnames):
		"""
		Tries to find a system font file corresponding to one of the
		supplied list of names. Returns None if no match is found.
		"""###
		if not isinstance(fontnames, (list,tuple)): fontnames = [fontnames]
		f = (filter(None, map(pygame.font.match_font, fontnames)) + [None])[0]
		if f == None and sys.platform == 'darwin': # pygame on OSX doesn't seem even to try to find fonts...
			f = (filter(os.path.isfile, map(lambda x:os.path.realpath('/Library/Fonts/%s.ttf'%x),fontnames)) + [None])[0]
		return f

	#############################################################

	def setup(self, left=None,top=None,width=None,height=None,changemode=None,framerate=None,bitdepth=None, **kwargs):
		"""
		Call this to set certain commonly-defined parameters for the screen
		during BciApplication.Preflight(). The renderer object will read
		these parameters in order to initialize the stimulus window, before
		BciApplication.Initialize() is called.
		"""###
		BciGenericRenderer.setup(self, left=left,top=top,width=width,height=height,changemode=changemode,framerate=framerate,bitdepth=bitdepth,**kwargs)
		
		pos = os.environ.get('SDL_VIDEO_WINDOW_POS','').split(',')
		if len(pos)==2: prevleft,prevtop = int(pos[0]),int(pos[1])
		else:           prevleft,prevtop = 160,120
		if left != None and top  == None: top  = prevtop
		if top  != None and left == None: left = prevleft
		if left != None and top != None:
			if sys.platform != 'darwin': # yup, yet another thing that's broken in pygame under OSX
				os.environ['SDL_VIDEO_WINDOW_POS'] = '%d,%d' % (int(left), int(top))
		
	#############################################################
	
	def SetDefaultFont(self, name=None, size=None):
		"""
		Set the name and/or size of the font that is used by
		default for Text stimuli. Returns True if the named font
		can be found, False if not.
		"""###
		if name != None:
			if os.path.isabs(name) and os.path.isfile(name):
				font = name
			else: 
				font = pygame.font.match_font(name)
				if font == None: return False
			self.__dict__['_defaultfont'] = font
		if size != None:
			self.__dict__['_defaultfontsize'] = size
		return True
	
	#############################################################

	def Initialize(self, bci):
		self.__dict__['_bci'] = bci # this is a mutual reference loop, but what the hell: self and bci only die when the process dies
		
		pygame.quit()
		pygame.init()
		w = self.opt('width', 0); h = self.opt('height', 0);
		depth = self.opt('bitdepth', 0);
		# TODO: how to handle framerate, and is this the right way of handling bitdepth?
		flags = 0
		if self.opt(('FULLSCREEN', 'changemode'), False): flags |= pygame.FULLSCREEN
		if self.opt(('NOFRAME', 'frameless_window'), False): flags |= pygame.NOFRAME
		if self.opt(('OPENGL'),    False): flags |= pygame.OPENGL
		if self.opt(('RESIZABLE'), False): flags |= pygame.RESIZABLE
		if self.opt(('DOUBLEBUF'), True):  flags |= pygame.DOUBLEBUF
		if self.opt(('HWSURFACE'), True):  flags |= pygame.HWSURFACE
		self._screen = pygame.display.set_mode((w,h), flags, depth)
		pygame.display.set_caption(self._windowtitle)
		
	#############################################################

	def opt(self, key, default=None):
		if not isinstance(key, (list,tuple)): key = (key,)
		setup = self.__dict__.get('_setup', {})
		for k in key:
			for v in (setup.get(k), setup.get(k.lower()), setup.get(k.upper())):
				if v != None: return v
		else:
			return default
		
	#############################################################

	def GetFrameRate(self):
		return 60.0 # TODO
	
	#############################################################

	def RaiseWindow(self):
		try:
			import ctypes  # !! Windows-specific code.
			stimwin = ctypes.windll.user32.FindWindowA(0, pygame.display.get_caption()[0])
			self._bci._raise_window(stimwin)
		except:
			pass
		
	#############################################################

	def GetEvents(self):
		return pygame.event.get()

	#############################################################

	def DefaultEventHandler(self, event):
		return (event.type == pygame.locals.QUIT) or (event.type == pygame.locals.KEYDOWN and event.key == pygame.locals.K_ESCAPE)
		
	#############################################################

	def StartFrame(self, objlist):
		bci = self._bci
		screen = self._screen
		bgcolor = self._bgcolor
		if bci: bci.ftdb(label='screen.clear')  #--------------------
		screen.fill(tuple([int(round(255 * x)) for x in bgcolor]))
		if bci: bci.ftdb(label='viewport.draw') #--------------------
		for x in objlist: x.draw(screen)

	#############################################################

	def FinishFrame(self):
		bci = self._bci		
		if bci: bci.ftdb(label='swap_buffers')  #--------------------
		self.HardWait()
		pygame.display.flip()

	#############################################################

	def Cleanup(self):
		self._screen = None
		pygame.quit()

#################################################################
#################################################################


class ImageStimulus(Coords.Box):
	def __init__(self, content=None, size=None, position=None, anchor='center', angle=0.0, smooth=True, color=(1,1,1,1), texture=None):
		
		Coords.Box.__init__(self)
		self.__dict__['__props'] = p = {}
		p['filename'] = None
		p['original'] = None
		p['changed'] = False
		p['last_transformation'] = None
		p['transformed'] = None
		p['last_coloring'] = None
		p['colored'] = None
		
		if content == None: content = texture
		if content == None:
			if size == None: size = Coords.Size(100,100)
			content = pygame.Surface(size, flags=pygame.HWSURFACE)
		if isinstance(content, str):
			p['filename'] = content
			content = pygame.image.load(p['filename'])
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
		
	def transform(self, force=False):
		p = self.__dict__['__props']
		srcsize  = tuple([int(round(x)) for x in self.original_size])
		dstsize  = tuple([int(round(x)) for x in self.size])
		angle = float(p['angle']) % 360.0
		smooth = bool(p['smooth'])
		changed = bool(p['changed'])
		flipx = bool(p['flipx'])
		flipy = bool(p['flipy'])
		color = tuple(p['color'])
		if len(color) == 3: color = color + (1.0,)
		athresh = 0.2
		if not flipx and not flipy and abs(angle - 180.0) < athresh:
			flipx = flipy = True; angle = 0.0
		if flipx and flipy: 
			if abs(angle - 180.0) < athresh: angle = 0.0; flipx = flipy = False
			elif abs(angle) >= athresh: angle = (angle + 180.0) % 360.0; flipx = flipy = False
		tr = (srcsize,dstsize,angle,flipx,flipy,smooth)
		pos = Coords.Point((self.left, self.bottom))
		# TODO: transform pos as necessary below: flips and rotations should occur around anchor
		#       store transformed_pos with transformed
		if force or changed or tr != p['last_transformation']:
			t = p['original']
			if flipx or flipy: t = pygame.transform.flip(flipx, flipy)
			if smooth:
				scaling = [float(dstsize[i]) / float(srcsize[i]) for i in (0,1)]
				proportional = (abs(scaling[0]-scaling[1]) < 1e-2)
				if dstsize != srcsize and not proportional: t = pygame.transform.smoothscale(t, dstsize)
				elif  dstsize != srcsize  and proportional: t = pygame.transform.rotozoom(t, angle, scaling[0])
				elif  abs(angle) > athresh:                 t = pygame.transform.rotozoom(t, angle, 1.0)
			else:
				if dstsize != srcsize: t = pygame.transform.scale(t, dstsize)
				if abs(angle) > athresh:   t = pygame.transform.rotate(t, angle)
			p['colored'] = p['transformed'] = t
			changed = True
		p['last_transformation'] = tr
		if force or changed or color != p['last_coloring']:
			t = p['transformed']
			if color != (1.0,1.0,1.0,1.0):
				# TODO: check this and the above aren't being done every time...
				t = to_numpy(t)
				a = numpy.array(color[:t.shape[2]])
				a.shape = (1,1,a.size)
				t = to_surface(t * a)
			p['colored'] = t
		p['last_coloring'] = color
		p['changed'] = False
		return p['colored'], pos

	def draw(self, screen):
		t, pos = self.transform()  # anchor still doesn't work quite right
		pos.y = screen.get_height() - pos.y
		screen.blit(t, pos)

	@apply
	def original_size():
		def fget(self):
			p = self.__dict__['__props']
			orig = p['original']
			return Coords.Size((orig.get_width(), orig.get_height()))
		return property(fget, doc="the width and height of the original image (read only)")
	
	@apply
	def content():
		def fget(self):
			p = self.__dict__['__props']
			return to_numpy(p['original'])
		def fset(self, val):
			p = self.__dict__['__props']
			p['original'] = to_surface(val).convert_alpha()
			p['changed'] = True			
		return property(fget, fset, doc='the content of the image stimulus as a numpy array')

	@apply
	def color():
		def fget(self):  p = self.__dict__['__props']; return p['color']
		def fset(self, val):  p = self.__dict__['__props']; p['color'] = list(val)
		return property(fget, fset, doc='3- or 4-element sequence denoting RGB or RGBA colour')
	
	@apply
	def angle():
		def fget(self):  p = self.__dict__['__props']; return p['angle']
		def fset(self, val):  p = self.__dict__['__props']; p['angle'] = float(val)
		return property(fget, fset, doc='rotation angle in degrees')
	
	@apply
	def smooth():
		def fget(self):  p = self.__dict__['__props']; return p['smooth']
		def fset(self, val):  p = self.__dict__['__props']; p['smooth'] = bool(val)
		return property(fget, fset, doc='whether or not pygame transformations are smooth')

	@apply
	def flipx():
		def fget(self):  p = self.__dict__['__props']; return p['flipx']
		def fset(self, val):  p = self.__dict__['__props']; p['flipx'] = bool(val)
		return property(fget, fset, doc='whether to display image flipped left-to-right')
	
	@apply
	def flipy():
		def fget(self):  p = self.__dict__['__props']; return p['flipy']
		def fset(self, val):  p = self.__dict__['__props']; p['flipy'] = bool(val)
		return property(fget, fset, doc='whether to display image flipped top-to-bottom')
		
class Text(object):
	pass # TODO  ...fontname, fontsize, text

from AppTools.Shapes import Disc, Block
		
BciGenericRenderer.subclass = PygameRenderer

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
		if src.get_alpha():
			raw_data = pygame.image.tostring(src,'RGBA',1)
			arr = numpy.fromstring( raw_data, dtype=numpy.uint8 ) / 255.0
			arr.shape = (height,width,4)
			return arr
		else:
			raw_data = pygame.image.tostring(src,'RGB',1)
			arr = numpy.fromstring( raw_data, dtype=numpy.uint8 ) / 255.0
			arr.shape = (height,width,3)
			return arr
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


#################################################################
#################################################################
