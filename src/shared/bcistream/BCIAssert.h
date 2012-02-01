//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A bciassert macro that throws an exception when its
//   condition is false.
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
#ifndef BCI_ASSERT_H
#define BCI_ASSERT_H

#include "BCIException.h"

#if BCIDEBUG
# define bciassert(x) { if( !bci::id_(x) ) throw bciexception( "Assertion failed: " << #x << "\n" ); }
#else // BCIDEBUG
# define bciassert(x)
#endif // BCIDEBUG

namespace bci
{ // The id_() function is there to avoid "condition is always false" and
  // "unreachable code" compiler warnings.
  inline bool id_( bool x )
    { return x; }

}

// The function trick does not work with BCB.
#ifdef __BORLANDC__
# pragma warn -8008
# pragma warn -8066
#endif // __BORLANDC__

#endif // BCI_ASSERT_H

