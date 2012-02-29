//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A header file that presents OSThread's static member
//   functions inside an additional namespace. This is to work around a
//   compiler bug that disallows the use of public static members
//   of a base class when there is private inheritance present in
//   the inheritance chain.
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

#include "OSThread.h"

namespace ThreadUtils
{

bool InMainThread()
    { return OSThread::InMainThread(); }

void SleepFor( int ms ) // sleep for milliseconds
	{ OSThread::SleepFor( ms ); }

void PrecisionSleepFor( double ms ) // sleep for milliseconds
    { OSThread::PrecisionSleepFor( ms ); }

void PrecisionSleepUntil( PrecisionTime wakeup ) // sleep until absolute wakeup time
    { OSThread::PrecisionSleepUntil( wakeup ); }

int NumberOfProcessors()
    { return OSThread::NumberOfProcessors(); }

// Deprecated because of ambiguous name.
bool IsMainThread()
    { return InMainThread(); }

} // namespace ThreadUtils

#endif // THREAD_UTILS_H

