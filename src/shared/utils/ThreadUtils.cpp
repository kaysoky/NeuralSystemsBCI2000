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
#include "OSMutex.h"

#if _WIN32
# include <MMSystem.h>
# include <Process.h>
#else // _WIN32
# include <unistd.h>
#endif // !_WIN32

#include <cmath>

using namespace std;
using namespace ThreadUtils;

#if _WIN32
#else // _WIN32
static pthread_t sMainThread = ::pthread_self();
#endif // _WIN32

#if _WIN32

static unsigned int sMainThreadID = ::GetCurrentThreadId();
static bool sInitialized = false;
static OSMutex sMutex;

#ifdef __BORLANDC__
# pragma option push
# pragma warn -8104 // local static with constructor warning
#endif // __BORLANDC__
void
ThreadUtils::PrecisionSleepFor( double inMs )
{
  if( inMs < 0 )
    return;

  // MSDN warns against frequent calls to timeBeginPeriod(), so we use a static RAAI object to set
  // timing precision once, and clear it on application exit.
  // We also don't want to call timeBeginPeriod() unless PrecisionSleepFor() is actually used,
  // so we use RAAI object that's local to the function.
  static class SetHighestPrecision
  {
   public:
    SetHighestPrecision()
    : mPrecision( 0 )
    {
      OSMutex::Lock lock( sMutex );
      if( !sInitialized )
      {
        TIMECAPS tc = { 0, 0 };
        ::timeGetDevCaps( &tc, sizeof( tc ) );
        mPrecision = tc.wPeriodMin;
        ::timeBeginPeriod( mPrecision );
        sInitialized = true;
      }
    }
    ~SetHighestPrecision()
    {
      ::timeEndPeriod( mPrecision );
    }
   private:
    UINT mPrecision;
    bool mInitialized;
    OSMutex mMutex;
  } setHighestPrecision;

  // Use "dithering" to achieve sub-millisecond accuracy of mean sleeping time.
  int sleepTime = static_cast<int>( ::floor( inMs ) ) + static_cast<int>( ::rand() < ( RAND_MAX + 1 ) * ::fmod( inMs, 1 ) );
  ::Sleep( sleepTime );
}
#ifdef __BORLANDC__
# pragma option pop
#endif // __BORLANDC__

void
ThreadUtils::SleepFor( int inMs )
{
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

void
ThreadUtils::PrecisionSleepFor( double inMs )
{
  if( inMs >= 0 )
    ::usleep( inMs * 1000 );
}

void
ThreadUtils::SleepFor( int inMs )
{
  PrecisionSleepFor( inMs );
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
ThreadUtils::PrecisionSleepUntil( PrecisionTime inWakeupTime )
{
  PrecisionSleepFor( PrecisionTime::SignedDiff( inWakeupTime, PrecisionTime::Now() ) );
}

