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

$RCSfile: wm_ext_win32.py,v $
$Id: wm_ext_win32.py,v 1.1.1.1 2007/10/21 22:02:04 jfp Exp $
"""

import pygame
try:
    import Numeric as N
except ImportError:
    import numpy as N

from ctypes import windll, Structure, c_long, c_ulong, sizeof, byref

SW_HIDE             =   0
SW_SHOWNORMAL       =   1
SW_NORMAL           =   1
SW_SHOWMINIMIZED    =   2
SW_SHOWMAXIMIZED    =   3
SW_MAXIMIZE         =   3
SW_SHOWNOACTIVATE   =   4
SW_SHOW             =   5
SW_MINIMIZE         =   6
SW_SHOWMINNOACTIVE  =   7
SW_SHOWNA           =   8
SW_RESTORE          =   9
SW_SHOWDEFAULT      =   10
SW_FORCEMINIMIZE    =   11
SW_MAX              =   11

SWP_NOSIZE          =   0x0001
SWP_NOMOVE          =   0x0002
SWP_NOZORDER        =   0x0004
SWP_NOREDRAW        =   0x0008
SWP_NOACTIVATE      =   0x0010
SWP_FRAMECHANGED    =   0x0020
SWP_SHOWWINDOW      =   0x0040
SWP_HIDEWINDOW      =   0x0080
SWP_NOCOPYBITS      =   0x0100
SWP_NOOWNERZORDER   =   0x0200
SWP_NOSENDCHANGING  =   0x0400
SWP_DRAWFRAME       =   SWP_FRAMECHANGED
SWP_NOREPOSITION    =   SWP_NOOWNERZORDER

HWND_TOP            =   0
HWND_BOTTOM         =   1
HWND_TOPMOST        =  -1
HWND_NOTOPMOST      =  -2

SPI_GETWORKAREA     =   48

ALTERNATE           =   1
WINDING             =   2

RGN_AND             =   1
RGN_OR              =   2
RGN_XOR             =   3
RGN_DIFF            =   4
RGN_COPY            =   5


user32              = windll.user32
gdi32               = windll.gdi32
IsIconic            = user32.IsIconic
IsZoomed            = user32.IsZoomed
ShowWindow          = user32.ShowWindow
GetWindowRect       = user32.GetWindowRect
SetWindowPos        = user32.SetWindowPos
GetForegroundWindow = user32.GetForegroundWindow
SetForegroundWindow = user32.SetForegroundWindow
MapWindowPoints     = user32.MapWindowPoints
SystemParametersInfo= user32.SystemParametersInfoA
SetWindowRgn        = user32.SetWindowRgn
CreatePolygonRgn    = gdi32.CreatePolygonRgn
CreateRectRgn       = gdi32.CreateRectRgn
CombineRgn          = gdi32.CombineRgn
DeleteObject        = gdi32.DeleteObject

class RECT(Structure):
    _fields_ = [
        ('left',    c_long),
        ('top',     c_long),
        ('right',   c_long),
        ('bottom',  c_long),
    ]
    def width(self):  return self.right  - self.left
    def height(self): return self.bottom - self.top

class POINT(Structure):
    _fields_ = [
        ('x',    c_long),
        ('y',    c_long),
    ]

EXT_POLYGON_WINDING     = WINDING
EXT_POLYGON_ALTERNATE   = ALTERNATE

def getSDLWindow():
    return pygame.display.get_wm_info()['window']

def EXT_IsIconic():
    return IsIconic(getSDLWindow())

def EXT_IsMaximized():
    return IsZoomed(getSDLWindow())

def EXT_Minimize():
    ShowWindow(getSDLWindow(), SW_MINIMIZE)

def EXT_Maximize():
    ShowWindow(getSDLWindow(), SW_MAXIMIZE)

def EXT_Restore():
    ShowWindow(getSDLWindow(), SW_RESTORE)

def EXT_Show(state):
    state = (SW_HIDE, SW_SHOW)[bool(state)]
    ShowWindow(getSDLWindow(), state)

def EXT_Activate():
    hWnd = getSDLWindow()
    if GetForegroundWindow() != hWnd:
       SetForegroundWindow(hWnd)

def EXT_GetWindowPos():
    rc = RECT()
    GetWindowRect(getSDLWindow(), byref(rc))
    return rc.left, rc.top

def EXT_GetWindowSize():
    rc = RECT()
    GetWindowRect(getSDLWindow(), byref(rc))
    return rc.width(), rc.height()

def EXT_SetWindowPos(x, y):
    SetWindowPos(getSDLWindow(), 0, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE)

def EXT_AlwaysOnTop(state):
    zorder = (HWND_NOTOPMOST, HWND_TOPMOST)[state]
    SetWindowPos(getSDLWindow(), zorder, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE)

def EXT_Client2Screen(x, y):
    pt = POINT(x, y)
    MapWindowPoints(getSDLWindow(), 0, byref(pt), 1)
    return pt.x, pt.y

def EXT_GetWorkArea():
    rc = RECT()
    SystemParametersInfo(SPI_GETWORKAREA, 0, byref(rc), 0);
    return (rc.left, rc.top), (rc.width(), rc.height())

def EXT_SetWindowShapePolygon(points, winding, redraw):
    point_array_type = POINT * len(points)
    a = point_array_type()
    for i, point in enumerate(points):
        a[i] = point
    region = CreatePolygonRgn(byref(a), len(a), winding)
    SetWindowRgn(getSDLWindow(), region, redraw)

def EXT_SetWindowShapeMask(image, redraw):
    # The image should be 8-bit and only contain palette entries 0 and 255.
    # The image width must be a multiple of 8.
    #
    if image.get_bytesize() != 1:
        raise ValueError("Mask depth should be 8-bit")
    rect = image.get_rect()
    if rect[0] % 8:
        raise ValueError("Mask width must be a multiple of 8")
    buffer = N.transpose(pygame.surfarray.pixels2d(image))

    # create an empty region
    region = CreateRectRgn(0, 0, 0, 0)
    # scan a row at a time looking for runs of filled pixels
    for y in range(rect.height):
        s = buffer[y]
        x = 0
        while 1:
            while x < rect.width and not s[x]:
                x += 1
            if x == rect.width:
                break
            left = x
            while x < rect.width and s[x]:
                x += 1
            span = CreateRectRgn(left, y, x, y+1)
            CombineRgn(region, region, span, RGN_OR)
            DeleteObject(span)
    SetWindowRgn(getSDLWindow(), region, redraw)

def EXT_ClearWindowShape():
    SetWindowRgn(getSDLWindow(), 0, 1)


