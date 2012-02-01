////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: This header file #defines a number of macros that absorb
//   differences between VCL versions.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef VCLDEFINES_H
#define VCLDEFINES_H

#ifdef __BORLANDC__

# include <vcl.h>

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
