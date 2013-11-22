//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Debugging support.
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
///////////////////////////////////////////////////////////////////////
#ifndef TINY_DEBUGGING_H
#define TINY_DEBUGGING_H

#include <string>

#ifndef TINY_DEBUG
# ifdef NDEBUG
#  define TINY_DEBUG 0
# else
#  define TINY_DEBUG 1
# endif
#endif

#if _WIN32
# include <Windows.h>
#endif
#include "Exception.h"

namespace Tiny
{

// When SuggestDebugging_() is called in debug mode, it displays a user
// dialog to allow for attaching a debugger.
// In release builds, this function does nothing.
void SuggestDebugging_( const std::string& why, const std::string& where = "" );

} // namespace

#if TINY_DEBUG

namespace {
  // The id_() function is there to avoid "condition is always false" and
  // "unreachable code" compiler warnings.
  inline bool id_( bool x )
    { return x; }
}

# define Assert(x) { if( !id_(x) ) throw std_logic_error( "Assertion failed: " << #x << "\n" ); }
# define SuggestDebugging(x) { Tiny::SuggestDebugging_( "DebugBreak statement", EXCEPTION_ARG_( x ) ); }
# define DebugBreak() { Tiny::SuggestDebugging_( "DebugBreak statement", EXCEPTION_ARG_( "DebugBreak()\n" << EXCEPTION_CONTEXT_ ) ); }

#else // TINY_DEBUG

# define Assert(x) (0)
# define SuggestDebugging(x) (0)
# define DebugBreak() (0)

#endif // TINY_DEBUG

#endif // TINY_DEBUGGING_H
