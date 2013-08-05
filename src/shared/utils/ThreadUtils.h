//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Thread-related utility functions.
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
#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include "PrecisionTime.h"

#ifdef Yield
# undef Yield
#endif

namespace ThreadUtils
{

bool InMainThread();

 // yield to other threads
void Yield();
 // sleep for milliseconds
void SleepFor( int ms );
// sleep until absolute wakeup time
void SleepUntil( PrecisionTime wakeup );

int NumberOfProcessors();

} // namespace ThreadUtils

#endif // THREAD_UTILS_H

