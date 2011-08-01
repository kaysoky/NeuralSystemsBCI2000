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

$RCSfile: __init__.py,v $
$Id: __init__.py,v 1.2 2007/10/21 22:22:17 jfp Exp $
"""

import sys

if sys.platform == "win32":
    from wm_ext_win32    import EXT_IsIconic, EXT_IsMaximized, EXT_Minimize, EXT_Maximize, EXT_Restore
    from wm_ext_win32    import EXT_Show, EXT_Activate, EXT_GetWindowPos, EXT_GetWindowSize, EXT_SetWindowPos
    from wm_ext_win32    import EXT_AlwaysOnTop, EXT_Client2Screen, EXT_GetWorkArea, EXT_SetWindowShapePolygon
    from wm_ext_win32    import EXT_SetWindowShapeMask, EXT_ClearWindowShape
    from wm_ext_win32    import EXT_POLYGON_WINDING, EXT_POLYGON_ALTERNATE
elif sys.platform == "linux2":
    from wm_ext_x11      import EXT_IsIconic, EXT_IsMaximized, EXT_Minimize, EXT_Maximize, EXT_Restore
    from wm_ext_x11      import EXT_Show, EXT_Activate, EXT_GetWindowPos, EXT_GetWindowSize, EXT_SetWindowPos
    from wm_ext_x11      import EXT_AlwaysOnTop, EXT_Client2Screen, EXT_GetWorkArea, EXT_SetWindowShapePolygon
    from wm_ext_x11      import EXT_SetWindowShapeMask, EXT_ClearWindowShape
    from wm_ext_x11      import EXT_POLYGON_WINDING, EXT_POLYGON_ALTERNATE
else:
    from wm_ext_fallback import EXT_IsIconic, EXT_IsMaximized, EXT_Minimize, EXT_Maximize, EXT_Restore
    from wm_ext_fallback import EXT_Show, EXT_Activate, EXT_GetWindowPos, EXT_GetWindowSize, EXT_SetWindowPos
    from wm_ext_fallback import EXT_AlwaysOnTop, EXT_Client2Screen, EXT_GetWorkArea, EXT_SetWindowShapePolygon
    from wm_ext_fallback import EXT_SetWindowShapeMask, EXT_ClearWindowShape
    from wm_ext_fallback import EXT_POLYGON_WINDING, EXT_POLYGON_ALTERNATE

