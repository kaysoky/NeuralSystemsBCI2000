////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: This file contains the header files that need to be included
//              before #pragma hdrstop in place of vcl.h to speed up compiles.
//              Compile with the NO_PCHINCLUDES flag defined to compile without
//              all these headers.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#ifndef PCHINCLUDES_H
#define PCHINCLUDES_H

#ifdef __BORLANDC__
#pragma warn -8004 // suppress warning on proper initialization of variables
#pragma warn -8027 // suppress warning on inline expansion failure
#endif

#ifdef __GNUC__
#include "gccprefix.h"
#endif

#ifndef NO_PCHINCLUDES

#ifdef __BORLANDC__
#include <vcl.h>
#endif

// STL headers
#include <map>
#include <set>
#include <list>
#include <vector>
#include <string>

#endif // NO_PCHINCLUDES

// M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif // M_PI

#endif // PCHINCLUDES_H
