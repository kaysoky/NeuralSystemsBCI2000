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

#if _WIN32
# include <windows.h>
#else // _WIN32
# include <unistd.h>
# include <sys/time.h>
# include <pthread.h>
# include <cerrno>
# include <algorithm>
# include "Synchronized.h"
#endif // _WIN32


namespace
{
#if _WIN32
  HANDLE GetData( void* inHandle )
  { return static_cast<HANDLE>( inHandle ); }
  typedef HANDLE Data;

  void* NewHandle()
  { // Create a manual-reset event in non-signaled state:
    return ::CreateEvent( NULL, true, false, NULL );
  }
  void DeleteHandle( void* inHandle )
  {
    if( inHandle )
    {
      ::SetEvent( GetData( inHandle ) );
      ::CloseHandle( GetData( inHandle ) );
    }
  }
#else // _WIN32
  struct Data_
  {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    Synchronized<bool> signaled;
    int fd[2];
  };
  void CreatePipe( Data_* p )
  {
    if( *p->fd == -1 && 0 != ::pipe( p->fd ) )
      throw std_runtime_error( "Could not create pipe" );
  }
  void ClosePipe( Data_* p )
  {
    if( *p->fd != -1 )
      for( int i = 0; i < 2; ++i )
        while( ::close( p->fd[i] ) < 0 && errno == EINTR )
          ;
  }
  Data_* WaitForPipes( Data_* const* pp, size_t n, int timeoutMs )
  {
    int maxfd = 0;
    fd_set readfds;
    FD_ZERO(&readfds);
    for( Data_* const* p = pp; p < pp + n; ++p )
    {
      int fd = *(*p)->fd;
      maxfd = std::max( maxfd, fd );
      FD_SET(fd, &readfds);
    }
    struct timeval tv = { timeoutMs / 1000, 1000 * ( timeoutMs % 1000 ) },
      *pTv = (timeoutMs == Tiny::InfiniteTimeout) ? 0 : &tv;
    int r;
    while( ( r = ::select( maxfd + 1, &readfds, 0, 0, &tv ) < 0 ) && errno == EINTR )
      ;
    for( Data_* const* p = pp; p < pp + n; ++p )
      if( FD_ISSET(*(*p)->fd, &readfds) )
        return *p;
    return 0;
  }
  bool FillPipe( Data_* p )
  {
    if( *p->fd == -1 )
      return true;
    char c = 'x';
    int r = 0;
    while( ( r = ::write( *p->fd, &c, 1 ) ) < 0 && errno == EINTR )
      ;
    return r > 0;
  }
  bool ClearPipe( Data_* p )
  {
    if( *p->fd == -1 )
      return true;
    char c = 0;
    int r = 0;
    while( WaitForPipes( &p, 1, 0 ) )
      while( ( r = ::read( *p->fd, &c, 1 ) ) < 0 && errno == EINTR )
        ;
    return r >= 0;
  }
  
  Data_* GetData( void* inP )
  { return reinterpret_cast<Data_*>( inP ); }
  const Data_* GetData( const void* inP )
  { return reinterpret_cast<const Data_*>( inP ); }
  typedef Data_* Data;

  void* NewHandle()
  {
    Data_* p = new Data_;
    ::pthread_cond_init( &p->cond, 0 );
    ::pthread_mutex_init( &p->mutex, 0 );
    p->signaled = false;
    *p->fd = -1;
    return p;
  }
  void DeleteHandle( void* inHandle )
  {
    Data_* p = GetData( inHandle );
    ::pthread_cond_destroy( &p->cond );
    ::pthread_mutex_destroy( &p->mutex );
    ClosePipe( p );
  }
#endif // _WIN32
}


Waitable::Waitable()
: mData( NewHandle() )
{
}

Waitable::~Waitable()
{
  DeleteHandle( mData );
}

bool
Waitable::Set()
{
#if _WIN32
  if( !::SetEvent( GetData( mData ) ) )
    throw std_runtime_error( SysError().Message() );
  return true;
#else // _WIN32
  Data_* p = GetData( mData );
  ::pthread_mutex_lock( &p->mutex );
  p->signaled = true;
  int result = ::pthread_cond_broadcast( &p->cond );
  ::pthread_mutex_unlock( &p->mutex );
  return FillPipe( p ) && result == 0;
#endif // _WIN32
}

bool
Waitable::Reset()
{
#if _WIN32
  if( !::ResetEvent( GetData( mData ) ) )
    throw std_runtime_error( SysError().Message() );
  return true;
#else // _WIN32
  Data_* p = GetData( mData );
  ::pthread_mutex_lock( &p->mutex );
  p->signaled = false;
  ::pthread_mutex_unlock( &p->mutex );
  return ClearPipe( p );
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
  Data_* p = GetData( mData );
  int result = 0;
  if( inTimeoutMs < 0 )
  {
    while( !p->signaled )
    {
     ::pthread_mutex_lock( &p->mutex );
      result = ::pthread_cond_wait( &p->cond, &p->mutex );
     ::pthread_mutex_unlock( &p->mutex );
    }
  }
  else
  {
    struct timeval now;
    ::gettimeofday( &now, NULL );
    struct timespec timeout;
    timeout.tv_sec = now.tv_sec + inTimeoutMs / 1000;
    timeout.tv_nsec = now.tv_usec * 1000 + ( inTimeoutMs % 1000 ) * 1000000;
    while( result == 0 && !p->signaled )
    {
     ::pthread_mutex_lock( &p->mutex );
      result = ::pthread_cond_timedwait( &p->cond, &p->mutex, &timeout );
     ::pthread_mutex_unlock( &p->mutex );
    }
  }
  return result == 0;
#endif // _WIN32
}

const Waitable*
Waitables::Wait( const Waitable* const* inWaitables, size_t inCount, int inTimeout )
{
  Data handles[8];
  bool many = ( inCount > sizeof( handles ) / sizeof( *handles ) );
  Data* pHandles = ( many ? new Data[inCount] : handles );
  for( size_t i = 0; i < inCount; ++i )
    pHandles[i] = GetData( inWaitables[i]->mData );
  const Waitable* pResult = 0;
#if _WIN32
  DWORD count = static_cast<DWORD>( inCount );
  DWORD result = ::WaitForMultipleObjects( count, pHandles, false, inTimeout );
  if( result == WAIT_FAILED )
    throw std_runtime_error( SysError().Message() );
  if( result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + inCount )
    pResult = inWaitables[result - WAIT_OBJECT_0];
  else if( result != WAIT_TIMEOUT )
    throw std_runtime_error( "Unexpected result" );
#else
  for( size_t i = 0; i < inCount; ++i )
    CreatePipe( pHandles[i] );
  Data p = WaitForPipes( pHandles, inCount, inTimeout );
  for( size_t i = 0; i < inCount; ++i )
    if( pHandles[i] == p )
      pResult = inWaitables[i];
#endif
  if( many )
    delete[] pHandles;
  return pResult;
}

