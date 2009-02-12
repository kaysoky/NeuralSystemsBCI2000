////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: This header file #defines a number of macros that absorb
//   differences between VCL versions.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef VCLDEFINES_H
#define VCLDEFINES_H

#ifdef __BORLANDC__

# if __BORLANDC__ >= 0x600 // RAD Studio 2009
#  define VCLSTR(x)      (UnicodeString(x).w_str())
   typedef wchar_t       VclCharType;
   typedef UnicodeString VclStringType;
# else
#  define VCLSTR(x)   (x)
   typedef char       VclCharType;
   typedef AnsiString VclStringType;
# endif

#endif // def __BORLANDC__

#endif // VCLDEFINES_H
