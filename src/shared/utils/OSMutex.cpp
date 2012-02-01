//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for mutex objects.
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

