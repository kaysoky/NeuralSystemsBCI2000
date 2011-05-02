//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for event objects.
//
// (C) 2000-2008, BCI2000 Project
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "OSEvent.h"
#if !_WIN32
# include <sys/time.h>
#endif // _WIN32

OSEvent::OSEvent()
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

OSEvent::~OSEvent()
{
#if _WIN32
  if( mHandle != NULL )
    ::CloseHandle( mHandle );
#else // _WIN32
  ::pthread_cond_destroy( &mCond );
  ::pthread_mutex_destroy( &mMutex );
#endif // _WIN32
}

bool
OSEvent::Set()
{
#if _WIN32
  return ::SetEvent( mHandle );
#else // _WIN32
  ::pthread_mutex_lock( &mMutex );
  mSignaled = true;
  int result = ::pthread_cond_broadcast( &mCond );
  ::pthread_mutex_unlock( &mMutex );
  return result == 0;
#endif // _WIN32
}

bool
OSEvent::Reset()
{
#if _WIN32
  return ::ResetEvent( mHandle );
#else // _WIN32
  ::pthread_mutex_lock( &mMutex );
  mSignaled = false;
  ::pthread_mutex_unlock( &mMutex );
  return true;
#endif // _WIN32
}

bool
OSEvent::Wait( int inTimeoutMs )
{
#if _WIN32
  DWORD timeout = inTimeoutMs >= 0 ? inTimeoutMs : INFINITE;
  return ( WAIT_OBJECT_0 == ::WaitForSingleObject( mHandle, timeout ) );
#else // _WIN32
  int result = 0;
  ::pthread_mutex_lock( &mMutex );
  if( inTimeoutMs < 0 )
  {
    while( !mSignaled )
      result = ::pthread_cond_wait( &mCond, &mMutex );
  }
  else
  {
    struct timeval now;
    ::gettimeofday( &now, NULL );
    struct timespec timeout;
    timeout.tv_sec = now.tv_sec + inTimeoutMs / 1000;
    timeout.tv_nsec = now.tv_usec * 1000 + ( inTimeoutMs % 1000 ) * 1000000;
    while( result == 0 && !mSignaled )
      result = ::pthread_cond_timedwait( &mCond, &mMutex, &timeout );
  }
  ::pthread_mutex_unlock( &mMutex );
  return result == 0;
#endif // _WIN32
}

