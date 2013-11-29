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
#include "ThreadUtils.h"

#include "Atomic.h"
#include "StaticObject.h"

#if _WIN32
# include <Windows.h>
#else // _WIN32
# include <unistd.h>
#endif // !_WIN32

#include <cmath>

using namespace std;

namespace Tiny {
  namespace ThreadUtils {

#if _WIN32

void
Yield_()
{
  ::Sleep( 0 );
}

void
SleepFor( int inMs )
{
  if( inMs >= 0 )
    ::Sleep( inMs );
}

int
NumberOfProcessors()
{
  SYSTEM_INFO info;
  ::GetSystemInfo( &info );
  return info.dwNumberOfProcessors;
}

ThreadID::ThreadID( bool initFromCurrentThread )
: mData( initFromCurrentThread ? reinterpret_cast<void*>( ::GetCurrentThreadId() ) : 0 ),
  mValid( initFromCurrentThread )
{
}

ThreadID::~ThreadID()
{
}

bool
ThreadID::operator==( const ThreadID& inOther ) const
{
  MemoryFence();
  return mValid && inOther.mValid && mData == inOther.mData;
}

#else // _WIN32

void
Yield_()
{
  ::pthread_yield();
}

void
SleepFor( int inMs )
{
  if( inMs >= 0 )
    ::usleep( inMs * 1000 );
}

int
NumberOfProcessors()
{
  int result = 1;
  result = ::sysconf( _SC_NPROCESSORS_ONLN );
  return result;
}

ThreadID::ThreadID( bool initFromCurrentThread )
: mData( initFromCurrentThread ? reinterpret_cast<void*>( ::pthread_self() ) : 0 ),
  mValid( initFromCurrentThread )
{
}

ThreadID::~ThreadID()
{
}

bool
ThreadID::operator==( const ThreadID& inOther ) const
{
  pthread_t self = reinterpret_cast<const pthread_t>( mData ),
            other = reinterpret_cast<const pthread_t>( inOther.mData );
  return mValid && inOther.mValid && ::pthread_equal( self, other );
}

#endif // _WIN32

namespace {
  ThreadID sMainThreadID;
}

bool
InMainThread()
{
  return ThreadID() == sMainThreadID;
}

void
SleepUntil( PrecisionTime inWakeupTime )
{
  SleepFor( PrecisionTime::SignedDiff( inWakeupTime, PrecisionTime::Now() ) );
}

bool
ThreadID::operator!=( const ThreadID& inOther ) const
{
  return !operator==( inOther ) && mValid && inOther.mValid;
}

  } // namespace
} // namespace
