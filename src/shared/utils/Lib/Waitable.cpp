//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for event objects.
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
#include "Waitable.h"
#include "SysError.h"
#include "Exception.h"
#if !_WIN32
# include <sys/time.h>
#endif // _WIN32

Waitable::Waitable()
{
#if _WIN32
  // Create a manual-reset event in non-signaled state:
  mHandle = ::CreateEvent( NULL, true, false, NULL );
#else
  ::pthread_cond_init( &mCond, NULL );
  ::pthread_mutex_init( &mMutex, NULL );
  mSignaled = false;
#endif
}

Waitable::~Waitable()
{
#if _WIN32
  if( mHandle != NULL )
  {
    ::SetEvent( mHandle );
    ::CloseHandle( mHandle );
  }
#else // _WIN32
  ::pthread_cond_destroy( &mCond );
  ::pthread_mutex_destroy( &mMutex );
#endif // _WIN32
}

bool
Waitable::Set()
{
#if _WIN32
  if( !::SetEvent( mHandle ) )
    throw std_runtime_error( SysError().Message() );
  return true;
#else // _WIN32
  ::pthread_mutex_lock( &mMutex );
  mSignaled = true;
  int result = ::pthread_cond_broadcast( &mCond );
  ::pthread_mutex_unlock( &mMutex );
  return result == 0;
#endif // _WIN32
}

bool
Waitable::Reset()
{
#if _WIN32
  if( !::ResetEvent( mHandle ) )
    throw std_runtime_error( SysError().Message() );
  return true;
#else // _WIN32
  ::pthread_mutex_lock( &mMutex );
  mSignaled = false;
  ::pthread_mutex_unlock( &mMutex );
  return true;
#endif // _WIN32
}

bool
Waitable::Wait( int inTimeoutMs ) const
{
#if _WIN32
  DWORD timeout = inTimeoutMs >= 0 ? inTimeoutMs : INFINITE;
  bool result = false;
  switch( ::WaitForSingleObject( mHandle, timeout ) )
  {
    case WAIT_OBJECT_0:
    case WAIT_ABANDONED:
      result = true;
      break;
    case WAIT_TIMEOUT:
      result = false;
      break;
    default:
      throw std_runtime_error( SysError().Message() );
  }
  return result;
#else // _WIN32
  int result = 0;
  if( inTimeoutMs < 0 )
  {
    while( !mSignaled )
    {
     ::pthread_mutex_lock( &mMutex );
      result = ::pthread_cond_wait( &mCond, &mMutex );
     ::pthread_mutex_unlock( &mMutex );
    }
  }
  else
  {
    struct timeval now;
    ::gettimeofday( &now, NULL );
    struct timespec timeout;
    timeout.tv_sec = now.tv_sec + inTimeoutMs / 1000;
    timeout.tv_nsec = now.tv_usec * 1000 + ( inTimeoutMs % 1000 ) * 1000000;
    while( result == 0 && !mSignaled )
    {
     ::pthread_mutex_lock( &mMutex );
      result = ::pthread_cond_timedwait( &mCond, &mMutex, &timeout );
     ::pthread_mutex_unlock( &mMutex );
    }
  }
  return result == 0;
#endif // _WIN32
}

const Waitable*
Waitables::Wait( const Waitable* const* inEvents, size_t inCount, int inTimeout )
{
#if _WIN32
  HANDLE handles[8];
  bool many = ( inCount > sizeof( handles ) / sizeof( *handles ) );
  HANDLE* pHandles = ( many ? new HANDLE[inCount] : handles );
  for( size_t i = 0; i < inCount; ++i )
    pHandles[i] = inEvents[i]->mHandle;
  DWORD count = static_cast<DWORD>( inCount );
  DWORD result = ::WaitForMultipleObjects( count, pHandles, false, inTimeout );
  if( many )
    delete[] pHandles;
  if( result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + inCount )
    return inEvents[result - WAIT_OBJECT_0];
  if( result == WAIT_TIMEOUT )
    return 0;
  if( result == WAIT_FAILED )
    throw std_runtime_error( SysError().Message() );
  throw std_runtime_error( "Unexpected result" );
  return 0;
#else
# error
#endif
}

