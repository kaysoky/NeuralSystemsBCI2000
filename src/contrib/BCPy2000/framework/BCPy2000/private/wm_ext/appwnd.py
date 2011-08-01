"""
Copyright (C) 2007 John Popplewell

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Contact : John Popplewell
Email   : john@johnnypops.demon.co.uk
Web     : http://www.johnnypops.demon.co.uk/python/

If you have any bug-fixes, enhancements or suggestions regarding this 
software, please contact me at the above email address.

$RCSfile: appwnd.py,v $
$Id: appwnd.py,v 1.1.1.1 2007/10/21 22:02:04 jfp Exp $
"""

import pygame
import wm_ext

from pygame.locals  import *


class AttrDict(dict):
    def __init__(self, *args, **kwargs):
        dict.__init__(self, *args, **kwargs)
    def __getattr__(self, name):
        return self[name]
    def __setattr__(self, name, value):
        self[name] = value


class AppWnd:
    APPMOUSEFOCUS       = 1         # The mouse is inside the app window
    APPINPUTFOCUS       = 2         # The app has input focus
    APPACTIVE           = 4         # The app is active

    def __init__(self, opts):
        pygame.init()
        self.initialised     = 0
        self.screen          = None
        self._opts           = opts
        self.videoflags      = 0
        self.screen_size     = None
        self.windowed_size   = opts.size
        self.maximized       = opts.maximized
        self.depth           = opts.depth
        self.pos             = None
        self._nf_maximized   = 0
        self.app_active      = 1
        self.app_input_focus = 1
        self.app_mouse_focus = 0

    def _setupDisplay(self, size):
        # fixes display corruption with ATI driver when fullscreen and windowed 
        # are the same size e.g. 800x600 but only if the bpp are different!
        #if self.IsFullscreen() and self.IsOpenGL():
        #    pygame.display.quit()   
        if not self.maximized and not self.IsFullscreen():
            self.windowed_size = size
        self.screen = pygame.display.set_mode(size, self.videoflags, self.depth)
        self.screen_size = self.screen.get_size()
        if not self.maximized and not self.IsFullscreen():
            self.windowed_size = self.screen_size
        pygame.mouse.set_visible(not self._opts.nomouse)
        if not self.IsFullscreen():
            wm_ext.EXT_AlwaysOnTop(self._opts.alwaysontop)

    def _resize(self, size):
        self._setupDisplay(size)
        if self.initialised:
            self.doResize(self.screen_size)

    def _noframeRestore(self):
        self._nf_maximized = 0
        size, pos = self.old_size, self.old_pos
        self.SetWindowSize(size)
        self.SetWindowPosition(pos)

    def _noframeMaximize(self):
        self._nf_maximized = 1
        self.old_pos  = self.GetWindowPosition()
        self.old_size = self.GetWindowSize()
        pos, size = self.getDesktopSize(1)
        self.SetWindowSize(size)
        self.SetWindowPosition(pos)

    def init(self, title=None, icon=None):
        info = pygame.display.Info()
        self.desktop_size = (info.current_w, info.current_h)

        x, y = self._opts.pos
        if x == -1:
            x = max(0, (self.desktop_size[0] - self.windowed_size[0])/2)
        if y == -1:
            y = max(0, (self.desktop_size[1] - self.windowed_size[1])/2)
        self.pos = (x, y)

        self.app_active = 1
        self.app_input_focus = 1
        self.app_mouse_focus = 0

        if self._opts.opengl:
            self.videoflags |= OPENGL
        if self._opts.doublebuff:
            self.videoflags |= DOUBLEBUF
        if self._opts.hardware:
            self.videoflags |= HWSURFACE
        if not self._opts.frame:
            self.videoflags |= NOFRAME
        elif not self._opts.noresize:
            self.videoflags |= RESIZABLE

        if icon:
            pygame.display.set_icon(icon)
        if self._opts.desktop and self._opts.fullscreen:
            size = self.desktop_size
        else:
            size = self.windowed_size
        self._setupDisplay(size)
        if title:
            pygame.display.set_caption(title)
        self.SetWindowPosition(self.pos)
        if self.maximized:
            wm_ext.EXT_Maximize()
        if self._opts.fullscreen:
            self.toggleFullscreen()
        self.initialised = 1

    def quit(self):
        pygame.quit()

    @staticmethod
    def getDefaultOptions():
        opts = dict(pos=(-1, -1), size=(640, 480), depth=0, maximized=0, 
                    alwaysontop=0, nomouse=0, hardware=0, doublebuff=0, frame=1, opengl=0, 
                    fullscreen=0, desktop=0, noresize=0)
        return AttrDict(opts)

    def IsFullscreen(self):
        return bool(self.videoflags & FULLSCREEN)

    def IsOpenGL(self):
        return bool(self.videoflags & OPENGL)

    def IsFrameless(self):
        return bool(self.videoflags & NOFRAME)

    def IsMaximized(self):
        if self.IsFrameless():
            return self._nf_maximized
        return bool(wm_ext.EXT_IsMaximized())

    def IsIconic(self):
        return bool(wm_ext.EXT_IsIconic())

    def Restore(self):
        if self.IsFrameless():
            self._noframeRestore()
        else:
            wm_ext.EXT_Restore()

    def Maximize(self):
        if self.IsFrameless():
            self._noframeMaximize()
        else:
            wm_ext.EXT_Maximize()

    def Minimize(self):
        wm_ext.EXT_Minimize()

    def Client2Screen(self, pt):
        return wm_ext.EXT_Client2Screen(*pt)

    def SetWindowPosition(self, pos):
        wm_ext.EXT_SetWindowPos(*pos)

    def SetWindowSize(self, size):
        self.maximized = wm_ext.EXT_IsMaximized()
        self._resize(size)

    def GetWindowPosition(self):
        return wm_ext.EXT_GetWindowPos()

    def GetWindowSize(self):
        return self.windowed_size

    def GetParentWindowSize(self):
        return wm_ext.EXT_GetWindowSize()

    def Show(self, state):
        wm_ext.EXT_Show(state)

    def Activate(self):
        wm_ext.EXT_Activate()

    def toggleFullscreen(self):
        if not pygame.display.toggle_fullscreen():
            self.videoflags ^= FULLSCREEN
            if self.IsFullscreen():
                self.maximized = wm_ext.EXT_IsMaximized()
                if not self.maximized:
                    self.pos = wm_ext.EXT_GetWindowPos()
                self._resize(self.screen_size)
            else:
                self._resize(self.windowed_size)
                if not self.maximized:
                    self.SetWindowPosition(self.pos)
                else:
                    wm_ext.EXT_Maximize()

    def getDesktopSize(self, workarea=0):
        if workarea:
            return wm_ext.EXT_GetWorkArea()
        return (0, 0), self.desktop_size

    def preset2pos(self, preset):
        pos = ((0, 1), (-1, 1), (1, 1), (0, -1), (-1, -1), (1, -1), (0, 0), (-1, 0), (1, 0))
        nx, ny = 0, 0
        x, y = pos[preset-1]
        dt_pos, dt_size = self.getDesktopSize(1)
        size = self.GetParentWindowSize()
        if x: nx = dt_size[0] - size[0]
        if y: ny = dt_size[1] - size[1]
        if x == -1:
            nx /= 2
        if y == -1:
            ny /= 2
        return nx + dt_pos[0], ny + dt_pos[1]

    def SetWindowShapePolygon(self, points, winding=wm_ext.EXT_POLYGON_WINDING, redraw=1):
        wm_ext.EXT_SetWindowShapePolygon(points, winding, redraw)

    def CreateDefaultShapeMask(self):
        # The actual mask colours are irrelevant, it is the palette indexes of 0 and 255
        # that matter here.
        # Makes sure that the width is a multiple of 8
        trans_colour = (255, 255, 0)
        mask_colour  = (0, 0, 255)
        width = 8*((self.screen_size[0]+7)/8)
        height = self.screen_size[1]
        mask = pygame.Surface((width, height), depth=8)
        palette = 256*[(0, 0, 0)]
        palette[0], palette[255] = trans_colour, mask_colour
        # seems to be a bug in Pygame <= 1.8.0pre triggered here: use SVN
        mask.set_palette(palette)
        mask.fill(trans_colour)
        return mask, trans_colour, mask_colour

    def SetWindowShapeMask(self, image, redraw=1):
        wm_ext.EXT_SetWindowShapeMask(image, redraw)

    def ClearWindowShape(self):
        wm_ext.EXT_ClearWindowShape()

    def run(self):
        self.doExpose()     # patch round missing event with X11 backend
        while 1:
            for event in pygame.event.get():
                if event.type == QUIT:
                    return
                elif event.type == VIDEORESIZE:
                    self.SetWindowSize(event.size)
                elif event.type == VIDEOEXPOSE:
                    self.doExpose()
                elif event.type == ACTIVEEVENT:
                    if event.state & self.APPACTIVE:
                        self.app_active = event.gain
                    if event.state & self.APPINPUTFOCUS:
                        self.app_input_focus = event.gain
                    if event.state & self.APPMOUSEFOCUS:
                        self.app_mouse_focus = event.gain
                    self.doActivate()
                elif event.type != NOEVENT:
                    if self.doEvent(event):
                        return
            self.doRun()

    def doActivate(self):
        pass

    def doResize(self, size):
        pass

    def doExpose(self):
        pass

    def doEvent(self, event):
        return 0

    def doRun(self):
        pygame.time.wait(1)


if __name__ == "__main__":
    opts = AppWnd.getDefaultOptions()
    app = AppWnd(opts)
    app.init(title="Empty Application")
    app.run()
    app.quit()

