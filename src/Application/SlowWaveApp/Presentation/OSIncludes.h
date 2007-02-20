/////////////////////////////////////////////////////////////////////////////
//
// File: OSIncludes.h
//
// Date: Oct 30, 2001
//
// Author: Juergen Mellinger
//
// Description: A wrapper for non-standard headers.
//
// Changes:
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef OS_INCLUDES_H
#define OS_INCLUDES_H

#ifdef __WIN32__
# ifndef WIN32
#  define WIN32
# endif // ndef WIN32
# ifndef LITTLE_ENDIAN
#  define LITTLE_ENDIAN 1
# endif // ndef LITTLE_ENDIAN
# ifdef __BCPLUSPLUS__
#  include <vcl.h>
#  ifndef VCL
#   define VCL
#  endif // ndef VCL
# else
#  include <windows.h>
# endif
#endif

#endif // OS_INCLUDES_H
 
