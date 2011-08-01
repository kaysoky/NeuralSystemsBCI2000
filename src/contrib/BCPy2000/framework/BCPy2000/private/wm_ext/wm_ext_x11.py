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

$RCSfile: wm_ext_x11.py,v $
$Id: wm_ext_x11.py,v 1.2 2007/10/22 00:13:32 jfp Exp $
"""

import time
import pygame
import Numeric as N

from ctypes import CDLL, pythonapi, py_object, Structure, Union, create_string_buffer
from ctypes import byref, sizeof, cast, POINTER
from ctypes import c_void_p, c_byte, c_char, c_char_p, c_short, c_int, c_long, c_ulong

libX11 = CDLL("libX11.so")
XIconifyWindow              = libX11.XIconifyWindow
XInternAtom                 = libX11.XInternAtom
XRootWindow                 = libX11.XRootWindow
XSendEvent                  = libX11.XSendEvent
XFlush                      = libX11.XFlush
XSync                       = libX11.XSync
XGetWindowProperty          = libX11.XGetWindowProperty
XFree                       = libX11.XFree
XListProperties             = libX11.XListProperties
XGetAtomName                = libX11.XGetAtomName
XGetWindowAttributes        = libX11.XGetWindowAttributes
XTranslateCoordinates       = libX11.XTranslateCoordinates
XMoveWindow                 = libX11.XMoveWindow
XRaiseWindow                = libX11.XRaiseWindow
XMapWindow                  = libX11.XMapWindow
XUnmapWindow                = libX11.XUnmapWindow
XQueryTree                  = libX11.XQueryTree
XFreePixmap                 = libX11.XFreePixmap
XCreatePixmapFromBitmapData = libX11.XCreatePixmapFromBitmapData
XCreateRegion               = libX11.XCreateRegion
XDestroyRegion              = libX11.XDestroyRegion
XPolygonRegion              = libX11.XPolygonRegion

libXext= CDLL("libXext.so")
XShapeQueryExtension    = libXext.XShapeQueryExtension
XShapeCombineRegion     = libXext.XShapeCombineRegion
XShapeCombineMask       = libXext.XShapeCombineMask

PyCObject_AsVoidPtr     = pythonapi.PyCObject_AsVoidPtr

# constants
ClientMessage               = 33

AnyPropertyType             = 0
XA_ATOM                     = 4
XA_CARDINAL                 = 6
XA_WM_HINTS                 = 35
StructureNotifyMask         = (1<<17)
ResizeRedirectMask          = (1<<18)
SubstructureNotifyMask      = (1<<19)
SubstructureRedirectMask    = (1<<20)
PropertyChangeMask          = (1<<22)
CurrentTime                 = 0

_NET_WM_STATE_REMOVE        = 0
_NET_WM_STATE_ADD           = 1
_NET_WM_STATE_TOGGLE        = 2

IsUnmapped                  = 0
IsUnviewable                = 1
IsViewable                  = 2

EvenOddRule                 = 0
WindingRule                 = 1

ShapeSet                    = 0
ShapeUnion                  = 1
ShapeIntersect              = 2
ShapeSubtract               = 3
ShapeInvert                 = 4

ShapeBounding               = 0
ShapeClip                   = 1
ShapeInput                  = 2

# custom types
t_Atom    = c_ulong
t_Bool    = c_int
t_Display = c_void_p
t_Window  = c_ulong
t_Colormap= c_ulong

class XPoint(Structure):
    _fields_ = [
        ('x', c_short),
        ('y', c_short),
    ]

class MessageEventData(Union):
    _fields_ = [
        ('b', c_char * 20),
        ('s', c_short* 10),
        ('l', c_long * 5),
    ]
    def __str__(self):
        return "0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X" % (self.l[0], self.l[1], self.l[2], self.l[3], self.l[4])

class XClientMessageEvent(Structure):
    _fields_ = [
        ('type',         c_int),
        ('serial',       c_ulong),
        ('send_event',   t_Bool),
        ('display',      t_Display),
        ('window',       t_Window),
        ('message_type', t_Atom),
        ('format',       c_int),
        ('data',         MessageEventData),
    ]
    def __str__(self):
        return "type:%s, serial:%s, send_event:%s, display:%s, window:%s, message_type:%s, format:%s\ndata:%s" % (self.type, self.serial, self.send_event, self.display, self.window, self.message_type, self.format, self.data)

class XWindowAttributes(Structure):
    _fields_ = [
        ('x',                     c_int),
        ('y',                     c_int),
        ('width',                 c_int),
        ('height',                c_int),
        ('border_width',          c_int),
        ('depth',                 c_int),
        ('visual',                c_void_p),
        ('root',                  t_Window),
        ('class',                 c_int),
        ('bit_gravity',           c_int),
        ('win_gravity',           c_int),
        ('backing_store',         c_int),
        ('backing_planes',        c_ulong),
        ('backing_pixel',         c_ulong),
        ('save_under',            t_Bool),
        ('colormap',              t_Colormap),
        ('map_installed',         t_Bool),
        ('map_state',             c_int),
        ('all_event_masks',       c_long),
        ('your_event_masks',      c_long),
        ('do_not_propagate_mask', c_long),
        ('override_redirect',     t_Bool),
        ('screen',                c_void_p),
    ]


EXT_POLYGON_WINDING     = WindingRule
EXT_POLYGON_ALTERNATE   = EvenOddRule

# private globals
_Initialised = 0
_ShapeExtension = 0
_RootWindow = None
_DefaultScreen = 0
_NET_WM_STATE                = None
_NET_WM_STATE_MAXIMIZED_VERT = None
_NET_WM_STATE_MAXIMIZED_HORZ = None
_NET_WORKAREA                = None
_NET_WM_DESKTOP              = None
_NET_WM_STATE_ABOVE          = None
_NET_FRAME_EXTENTS           = None


def _XShapeQueryExtension(display):
    shape_event_base, shape_error_base = c_int(), c_int()
    return bool(XShapeQueryExtension(display, byref(shape_event_base), byref(shape_error_base)))

def getSDLWindowInfo():
    global _Initialised, _ShapeExtension, _RootWindow
    global _NET_WM_STATE, _NET_WM_STATE_MAXIMIZED_VERT, _NET_WM_STATE_MAXIMIZED_HORZ 
    global _NET_WORKAREA, _NET_WM_DESKTOP, _NET_WM_STATE_ABOVE, _NET_FRAME_EXTENTS

    info = pygame.display.get_wm_info()
    #print info
    display = PyCObject_AsVoidPtr(py_object(info['display']))
    wmwindow, window, fswindow = info['wmwindow'], info['window'], info['fswindow']
    if not _Initialised:
        _ShapeExtension = _XShapeQueryExtension(display)
        _RootWindow = XRootWindow(display, _DefaultScreen)
        _NET_WM_STATE = XInternAtom(display, "_NET_WM_STATE", 1)
        _NET_WM_STATE_MAXIMIZED_VERT = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", 1)
        _NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", 1)
        _NET_WORKAREA = XInternAtom(display, "_NET_WORKAREA", 1)
        _NET_WM_DESKTOP = XInternAtom(display, "_NET_WM_DESKTOP", 1)
        _NET_WM_STATE_ABOVE = XInternAtom(display, "_NET_WM_STATE_ABOVE", 1)
        _NET_FRAME_EXTENTS = XInternAtom(display, "_NET_FRAME_EXTENTS", 1)
        _Initialised = 1
    return display, wmwindow, window, fswindow

def _XGetAtomName(display, atom):
    p_name = XGetAtomName(display, atom)
    name = c_char_p(p_name).value
    XFree(p_name)
    return name

def _XListProperties(display, window):
    nitems = c_int()
    p_atoms = XListProperties(display, window, byref(nitems))
    atoms = []
    p = cast(p_atoms, POINTER(t_Atom))
    for i in range(nitems.value):
        atoms.append(p[i])
    XFree(p)
    return atoms

def _XGetWindowAttributes(display, window):
    xwa = XWindowAttributes()
    XGetWindowAttributes(display, window, byref(xwa))
    return xwa

def _XGetWindowProperty(display, window, property, type_):
    actual_type   = t_Atom()
    actual_format = c_int()
    nitems = c_long()
    nleft  = c_long()
    data   = c_void_p()
    res = XGetWindowProperty(display, window, property, 0, ~0, 0, type_, \
                             byref(actual_type), byref(actual_format), byref(nitems), \
                             byref(nleft), byref(data))
    return res, actual_type.value, actual_format.value, nitems.value, data

def _XGetProperty(display, window, property, type_):
    props = []
    res, atype, aformat, nitems, data = _XGetWindowProperty(display, window, property, type_)
    #print "res:%s, atype:%s, aformat:%s, nitems:%s" %(res, atype, aformat, nitems)
    if res or atype != type_:
        return props
    p = cast(data, POINTER(c_ulong))
    for i in range(nitems):
        props.append(p[i])
    if nitems:
        XFree(data.value)
    return props

def _XGetAtoms(display, window, property):
    return _XGetProperty(display, window, property, XA_ATOM)

def _XGetCardinals(display, window, property):
    return _XGetProperty(display, window, property, XA_CARDINAL)

def _XGetWmHints(display, window, property):
    return _XGetProperty(display, window, property, XA_WM_HINTS)

def _XGetDesktop(display, window):
    cardinals = _XGetCardinals(display, window, _NET_WM_DESKTOP)
    return cardinals[0]

def _XWMState(display, window, action, prop0, prop1=0):
    xev = XClientMessageEvent()
    xev.type   = ClientMessage
    xev.serial = 0
    xev.send_event = 1
    xev.window = window
    xev.message_type = _NET_WM_STATE
    xev.format = 32
    if action:
        xev.data.l[0] = _NET_WM_STATE_ADD
    else:
        xev.data.l[0] = _NET_WM_STATE_REMOVE
    xev.data.l[1] = prop0
    xev.data.l[2] = prop1
    xev.data.l[3] = 0
    xev.data.l[4] = 0
    event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ResizeRedirectMask
    res = XSendEvent(display, _RootWindow, 0, event_mask, byref(xev))
    return res

def _XTranslateCoordinates(display, from_, to_, pos):
    x_ret, y_ret = c_int(), c_int()
    child_ret = t_Window()
    XTranslateCoordinates(display, from_, to_, pos[0], pos[1],
                          byref(x_ret), byref(y_ret), byref(child_ret))
    return x_ret.value, y_ret.value

def _XQueryTree(display, window):
    root = t_Window()
    parent = t_Window()
    kids = t_Window()
    nkids = c_int()
    XQueryTree(display, window, byref(root), byref(parent), byref(kids), byref(nkids))
    XFree(kids)
    return root.value, parent.value

def _hasFrame(display, window):
    cardinals = _XGetCardinals(display, window, _NET_FRAME_EXTENTS)
    return bool(len(cardinals))

def _getToplevelWindow(display, window):
    toplevel = window
    while 1:
        root, window = _XQueryTree(display, window)
        if window == root:
            return toplevel
        toplevel = window

def EXT_IsIconic():
    display, wmwindow, window, fswindow = getSDLWindowInfo()
    xwa = _XGetWindowAttributes(display, wmwindow)
    return xwa.map_state == IsUnmapped

def EXT_IsMaximized():
    display, wmwindow, _, _ = getSDLWindowInfo()
    atoms = _XGetAtoms(display, wmwindow, _NET_WM_STATE)
    return (_NET_WM_STATE_MAXIMIZED_VERT in atoms) and (_NET_WM_STATE_MAXIMIZED_HORZ in atoms)

def EXT_Minimize():
    display, wmwindow, _, _ = getSDLWindowInfo()
    XIconifyWindow(display, wmwindow, _DefaultScreen)

def EXT_Maximize():
    display, wmwindow, window, _ = getSDLWindowInfo()
    # HACK: work-round for buggy Gnome WM (or SDL bug?) - still not 100% reliable
    # Try using ALT-F5/ALT-F10 on the Gnome terminal ...
    # Should be just the following two lines, which work reliably on KDE:
    #_XWMState(display, wmwindow, 1, _NET_WM_STATE_MAXIMIZED_VERT, _NET_WM_STATE_MAXIMIZED_HORZ)
    #return
    for i in range(20):
        _XWMState(display, wmwindow, 1, _NET_WM_STATE_MAXIMIZED_VERT, _NET_WM_STATE_MAXIMIZED_HORZ)
        atoms = _XGetAtoms(display, wmwindow, _NET_WM_STATE)
        if (_NET_WM_STATE_MAXIMIZED_VERT in atoms) and (_NET_WM_STATE_MAXIMIZED_HORZ in atoms):
            return
        time.sleep(0.1)

def EXT_Restore():
    import time
    display, wmwindow, window, _ = getSDLWindowInfo()
    # HACK: work-round for buggy Gnome WM (or SDL bug?) - still not 100% reliable
    # Try using ALT-F5/ALT-F10 on the Gnome terminal ...
    # Should be just the following two lines, which work reliably on KDE:
    #_XWMState(display, wmwindow, 0, _NET_WM_STATE_MAXIMIZED_VERT, _NET_WM_STATE_MAXIMIZED_HORZ)
    #return
    for i in range(20):
        _XWMState(display, wmwindow, 0, _NET_WM_STATE_MAXIMIZED_VERT, _NET_WM_STATE_MAXIMIZED_HORZ)
        atoms = _XGetAtoms(display, wmwindow, _NET_WM_STATE)
        if not ((_NET_WM_STATE_MAXIMIZED_VERT in atoms) or (_NET_WM_STATE_MAXIMIZED_HORZ in atoms)):
            return
        time.sleep(0.1)

def EXT_Show(state):
    display, wmwindow, _, _ = getSDLWindowInfo()
    if state:
        XMapWindow(display, wmwindow)
    else:
        XUnmapWindow(display, wmwindow)

def EXT_Activate():
    display, wmwindow, _, _ = getSDLWindowInfo()
    XRaiseWindow(display, wmwindow)

def EXT_GetWindowPos():
    display, wmwindow, window, fswindow = getSDLWindowInfo()
    toplevel = _getToplevelWindow(display, wmwindow)
    xwa = _XGetWindowAttributes(display, toplevel)
    return xwa.x, xwa.y

def EXT_GetWindowSize():
    display, wmwindow, window, fswindow = getSDLWindowInfo()
    if _hasFrame(display, wmwindow):
        xwa = _XGetWindowAttributes(display, wmwindow)
    else:
        xwa = _XGetWindowAttributes(display, window)
    return (xwa.width + xwa.x*2, xwa.height + xwa.y + xwa.x)

def EXT_SetWindowPos(x, y):
    display, wmwindow, window, fswindow = getSDLWindowInfo()
    XMoveWindow(display, wmwindow, x, y)
    XFlush(display)

def EXT_AlwaysOnTop(state):
    display, wmwindow, _, _ = getSDLWindowInfo()
    _XWMState(display, wmwindow, state, _NET_WM_STATE_ABOVE)

def EXT_Client2Screen(x, y):
    display, wmwindow, window, fswindow = getSDLWindowInfo()
    return _XTranslateCoordinates(display, wmwindow, _RootWindow, (x, y))

def EXT_GetWorkArea():
    display, wmwindow, _, _ = getSDLWindowInfo()
    desktop = _XGetDesktop(display, wmwindow)
    cardinals = _XGetCardinals(display, _RootWindow, _NET_WORKAREA)
    i = desktop*4
    x, y, width, height = cardinals[i:i+4]
    return (x, y), (width, height)

def EXT_SetWindowShapePolygon(points, winding, redraw):
    display, wmwindow, window, fswindow = getSDLWindowInfo()
    if not _ShapeExtension:
        raise NotImplementedError("XShapeExtension not available")
    point_array_type = XPoint * len(points)
    a = point_array_type()
    for i, point in enumerate(points):
        a[i] = point
    region = XPolygonRegion(byref(a), len(a), winding)
    XShapeCombineRegion(display, wmwindow, ShapeBounding, 0, 0, region, ShapeSet)
    XDestroyRegion(region)

def EXT_SetWindowShapeMask(image, redraw):
    # The image should be 8-bit and only contain palette entries 0 and 255.
    # The image width must be a multiple of 8.
    # This speeds up the packing into the 1-bit/pixel bitmap passed
    # into XCreatePixmapFromBitmapData().
    #
    display, wmwindow, window, fswindow = getSDLWindowInfo()
    if not _ShapeExtension:
        raise NotImplementedError("XShapeExtension not available")
    if image.get_bytesize() != 1:
        raise ValueError("Mask depth should be 8-bit")
    rect = image.get_rect()
    if rect[0] % 8:
        raise ValueError("Mask width must be a multiple of 8")
    # The following was faster with Numeric-23.7, but nearly 20 times slower with 24.2(!)
    # So much for fast pixel access ...
    #buffer = N.transpose(pygame.surfarray.pixels2d(image))
    buffer = N.transpose(pygame.surfarray.pixels2d(image)).tolist()
    data = create_string_buffer((rect.width/8)*rect.height)

    i = 0
    for y in range(rect.height):
        s = buffer[y]
        for x in range(0, rect.width, 8):
            # probably endian problems here
            data[i] = chr((s[x+7]&0x80)+(s[x+6]&0x40)+(s[x+5]&0x20)+(s[x+4]&0x10)+(s[x+3]&0x08)+(s[x+2]&0x04)+(s[x+1]&0x02)+(s[x+0]&0x01))
            i += 1

    fg, bg = 1, 0
    mask = XCreatePixmapFromBitmapData(display, wmwindow, data, rect.width, rect.height, fg, bg, 1)
    XShapeCombineMask(display, wmwindow, ShapeBounding, 0, 0, mask, ShapeSet)
    XFreePixmap(display, mask)

def EXT_ClearWindowShape():
    display, wmwindow, window, fswindow = getSDLWindowInfo()
    if not _ShapeExtension:
        raise NotImplementedError("XShapeExtension not available")
    point_array_type = XPoint * 4
    a = point_array_type()
    size = EXT_GetWindowSize()
    a[0] = XPoint(0, 0)
    a[1] = XPoint(size[0], 0)
    a[2] = XPoint(size[0], size[1])
    a[3] = XPoint(0, size[1])
    region = XPolygonRegion(byref(a), len(a), EXT_POLYGON_WINDING)
    XShapeCombineRegion(display, wmwindow, ShapeBounding, 0, 0, region, ShapeSet)
    XDestroyRegion(region)


