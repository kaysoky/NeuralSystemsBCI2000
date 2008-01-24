/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef GL_HEADERS_H
#define GL_HEADERS_H

#include <windows.h>    // Header file for windows
#include <gl/gl.h>      // Header file for the OpenGL32 library
#include <gl/glu.h>     // Header file for the GLu32 library
#include <gl/glaux.h>   // Header file for the GLaux library

#include <string>
#include <vector>
#include <cstdio>
#include <exception>

#ifndef _USE_MATH_DEFINES
# define _USE_MATH_DEFINES 1
# include <cmath>
# undef _USE_MATH_DEFINES
#else
# include <cmath>
#endif

#ifdef __BORLANDC__
# include <vcl.h>
#endif // __BORLANDC__

#define GL_CHECK \
  {int err=glGetError();if(err!=GL_NO_ERROR)MessageBox(NULL,gluErrorString(err),"GL_CHECK",MB_OK);}

#endif // GL_HEADERS_H
