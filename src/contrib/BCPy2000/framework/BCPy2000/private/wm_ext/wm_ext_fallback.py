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

$RCSfile: wm_ext_fallback.py,v $
$Id: wm_ext_fallback.py,v 1.2 2007/10/21 22:22:17 jfp Exp $
"""

import pygame

EXT_POLYGON_WINDING, EXT_POLYGON_ALTERNATE = range(2)

def EXT_IsIconic():
    return 0

def EXT_IsMaximized():
    return 0

def EXT_Minimize():
    pass

def EXT_Maximize():
    pass

def EXT_Restore():
    pass

def EXT_Show(state):
    pass

def EXT_Activate():
    pass

def EXT_GetWindowPos():
    return (-1, -1)

def EXT_GetWindowSize():
    return (800, 600)

def EXT_SetWindowPos(x, y):
    pass

def EXT_AlwaysOnTop(state):
    pass

def EXT_Client2Screen(x, y):
    return x, y

def EXT_GetWorkArea():
    return (0, 0), (800, 600)

def EXT_SetWindowShapePolygon(points, winding, redraw):
    pass

def EXT_SetWindowShapeMask(image, redraw):
    pass

def EXT_ClearWindowShape():
    pass

