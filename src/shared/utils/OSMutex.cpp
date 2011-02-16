//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for mutex objects.
//
// (C) 2000-2008, BCI2000 Project
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "OSMutex.h"

#if _WIN32
# include <windows.h>
#endif // _WIN32

#if _WIN32
OSMutex::OSMutex()
: mHandle( ::CreateMutex( NULL, false, NULL ) )
{
}

OSMutex::~OSMutex()
{
  if( mHandle != NULL )
    ::CloseHandle( mHandle );
}

bool
OSMutex::Acquire() const
{
  return ( WAIT_OBJECT_0 == ::WaitForSingleObject( mHandle, INFINITE ) );
}

bool
OSMutex::Release() const
{
  return ::ReleaseMutex( mHandle );
}

#else // _WIN32

OSMutex::OSMutex()
{
  ::pthread_mutexattr_t attributes;
  if( !::pthread_mutexattr_init( &attributes ) )
  {
    ::pthread_mutexattr_settype( &attributes, PTHREAD_MUTEX_RECURSIVE );
    ::pthread_mutex_init( &mMutex, &attributes );
    ::pthread_mutexattr_destroy( &attributes );
  }
}

OSMutex::~OSMutex()
{
  ::pthread_mutex_destroy( &mMutex );
}

bool
OSMutex::Acquire() const
{
  return 0 == ::pthread_mutex_lock( &mMutex );
}

bool
OSMutex::Release() const
{
  return 0 == ::pthread_mutex_unlock( &mMutex );
}

#endif // _WIN32

OSMutex::Lock::Lock( const OSMutex& inMutex )
: mpMutex( &inMutex )
{
  DoAcquire();
}

// OSMutex::Lock class
OSMutex::Lock::Lock( const OSMutex* inpMutex )
: mpMutex( inpMutex )
{
  DoAcquire();
}

OSMutex::Lock::~Lock()
{
  if( mpMutex )
    mpMutex->Release();
}

void
OSMutex::Lock::DoAcquire()
{
  if( mpMutex && !mpMutex->Acquire() )
    mpMutex = NULL;
}

// OSMutex::Unlock class
OSMutex::Unlock::Unlock( const OSMutex& inMutex )
: mpMutex( &inMutex )
{
  DoRelease();
}

OSMutex::Unlock::Unlock( const OSMutex* inpMutex )
: mpMutex( inpMutex )
{
  DoRelease();
}

void
OSMutex::Unlock::DoRelease()
{
  if( mpMutex && !mpMutex->Release() )
    mpMutex = NULL;
}

OSMutex::Unlock::~Unlock()
{
  if( mpMutex )
    mpMutex->Acquire();
}

