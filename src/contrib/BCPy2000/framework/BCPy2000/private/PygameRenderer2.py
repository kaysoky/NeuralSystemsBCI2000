#!/usr/bin/python

"""
This may eventually become the second version of the pygame renderer.
As of yet, it is unfinished.

"""

__all__ = ['Text', 'Block', 'Disc', 'ImageStimulus']

import os
import sys
if sys.platform == "win32":
    os.environ["SDL_VIDEODRIVER"] = "windib"

import Image
import pygame
pygame.font.init()

import wm_ext

import AppTools.Coords as Coords 

try:    from BCI2000PythonApplication    import BciGenericRenderer, BciStimulus   # development copy
except: from BCPy2000.GenericApplication import BciGenericRenderer, BciStimulus   # installed copy

class PyGameRenderer(BciGenericRenderer):

    def __init__(self):
        self.monofont = self.findfont(
            ('lucida console', 'monaco', 'monospace', 'courier new', 'courier')
        )
        self.font_name = pygame.font.get_default_font()
        self.font_size = 3
        self.font = pygame.font.Font(self.font_name, self.font_size)
        self.left = 100
        self.top = 100
        self.width = 300
        self.height = 300
        self.bgcolor = (0.5, 0.5, 0.5)
        self.framerate = 60.
        self.changemode = False
        self.frameless_window = False
        self.title = "stimuli"
        self.screen = None

    def setup(self, left = None, top = None, width = None, height = None,
        bgcolor = None, framerate = None, changemode = None,
        frameless_window = None, **kwds):
        """
        Call this to set certain commonly-defined parameters for the screen
        during BciApplication.Preflight(). The renderer object will read
        these parameters in order to initialize the stimulus window, before
        BciApplication.Initialize() is called.
        """###
        # `**kwds` is used for compatibility with the `VisionEggRenderer`:
        # the `bitdepth` parameter is ignored.
        self.left = left if left != None else self.left
        self.top = top if top != None else self.top
        self.width = width if width != None else self.width
        self.height = height if height != None else self.height
        self.bgcolor = bgcolor if bgcolor != None else self.bgcolor
        self.framerate = float(framerate) if framerate != None else self.framerate
        self.changemode = changemode if changemode != None else self.changemode
        self.frameless_window = frameless_window if frameless_window != None else self.frameless_window

    def Initialize(self, bci):
        # Takes `bci` as an argument for compatibility purposes with `VisionEggRenderer`.
        # It is ignored.
        print "initialize" #TODO
        pygame.display.quit()
        os.environ["SDL_VIDEO_WINDOW_POS"] = "%i,%i" % (int(self.left), int(self.top))
        pygame.display.init()
        flags = \
            (self.changemode and (pygame.FULLSCREEN | pygame.DOUBLEBUF)) | \
            (self.frameless_window and pygame.NOFRAME) | 0
        print flags
        self.screen = pygame.display.set_mode((int(self.width), int(self.height)), flags)
        wm_ext.EXT_AlwaysOnTop(True)

    def GetFrameRate(self):
        return self.framerate

    def RaiseWindow(self):
        # Since the window is already set to be always on top, this method should do nothing.
        pass

    def GetEvents(self):
        return pygame.event.get()

    def DefaultEventHandler(self):
        return (event.type == pygame.locals.QUIT) or (event.type == pygame.locals.KEYDOWN and event.key == pygame.locals.K_ESCAPE)

    def StartFrame(self, objlist):
        for obj in objlist:
            obj.draw(self.screen)

    def FinishFrame(self):
        pygame.display.flip()

    def findfont(self, fontnames):
        """
        Tries to find a system font file corresponding to one of the
        supplied list of names. Returns None if no match is found.
        """###
        def matchfont(fontname):
            bold = italic = False
            for i in range(0,1):
                if fontname.lower().endswith(' italic'): italic = True; fontname = fontname[:-len(' italic')]
                if fontname.lower().endswith(' bold'): bold = True; fontname = fontname[:-len(' bold')]
            return pygame.font.match_font(fontname, bold=int(bold), italic=int(italic))
            
        if not isinstance(fontnames, (list,tuple)): fontnames = [fontnames]
        fontnames = [f for f in fontnames if f != None]
        f = (filter(None, map(matchfont, fontnames)) + [None])[0]
        if f == None and sys.platform == 'darwin': # pygame on OSX doesn't seem even to try to find fonts...
            f = (filter(os.path.isfile, map(lambda x:os.path.realpath('/Library/Fonts/%s.ttf'%x),fontnames)) + [None])[0]
        return f

    def SetDefaultFont(self, name = None, size = None):
        """
        Set the name and/or size of the font that is used by
        default for Text stimuli. Returns True if the named font
        can be found, False if not.
        """###
        if name != None:
            if os.path.isabs(name) and os.path.isfile(name):
                font = name
            else: 
                font = self.findfont(name)
                if font == None: return False
        self.font_name = font
        self.font_size = size if size != None else self.font_size
        self.font = pygame.font.Font(self.font_name, self.font_size)
        return True

    def GetDefaultFont(self):
        return self.font_name, self.font_size

    @property
    def size(self):
        try:
            info = pygame.display.Info()
        except:
            return (0,0)
        return Coords.Size((info.current_w, info.current_h))

    @apply
    def color():
        def fget(self):
            return self.bgcolor
        def fset(self, value):
            self.bgcolor = value
        return property(fget, fset)

    def cleanup(self):
        self.screen = None
        pygame.display.quit()

BciGenericRenderer.subclass = PyGameRenderer

class ImageStimulus(Coords.Box):
    def __init__(self, content=None, size=None, position=None, anchor='center',
        angle=0.0, smooth=True, color=(1,1,1,1), texture=None):
        
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
            content = pygame.Surface(size)
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
        # TODO: Create a surface with width and height (a few pixels more than) double the
        #       maximum distance from the anchor to the furthest corner. Blit this surface
        #       to it such that the anchor is in the center of the new surface. Apply transformations.
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
