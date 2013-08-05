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
#include "PCHIncludes.h"
#pragma hdrstop

#include "ThreadUtils.h"

#if _WIN32
# include <Windows.h>
#else // _WIN32
# include <unistd.h>
#endif // !_WIN32

#include <cmath>

using namespace std;
using namespace ThreadUtils;

#if _WIN32

static unsigned int sMainThreadID = ::GetCurrentThreadId();

#undef Yield
void
ThreadUtils::Yield()
{
  ::Sleep( 0 );
}

void
ThreadUtils::SleepFor( int inMs )
{
  if( inMs >= 0 )
    ::Sleep( inMs );
}

bool
ThreadUtils::InMainThread()
{
  return ::GetCurrentThreadId() == sMainThreadID;
}

int
ThreadUtils::NumberOfProcessors()
{
  SYSTEM_INFO info;
  ::GetSystemInfo( &info );
  return info.dwNumberOfProcessors;
}

#else // _WIN32

static pthread_t sMainThread = ::pthread_self();

void
ThreadUtils::Yield()
{
  ::pthread_yield();
}

void
ThreadUtils::SleepFor( double inMs )
{
  if( inMs >= 0 )
    ::usleep( inMs * 1000 );
}

bool
ThreadUtils::InMainThread()
{
  return ::pthread_equal( pthread_self(), sMainThread );
}

int
ThreadUtils::NumberOfProcessors()
{
  int result = 1;
  result = ::sysconf( _SC_NPROCESSORS_ONLN );
  return result;
}

#endif // _WIN32

void
ThreadUtils::SleepUntil( PrecisionTime inWakeupTime )
{
  SleepFor( PrecisionTime::SignedDiff( inWakeupTime, PrecisionTime::Now() ) );
}
