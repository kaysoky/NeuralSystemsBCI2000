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
#include "Mutex.h"

#if _WIN32
# include <windows.h>
#endif // _WIN32

#if _WIN32
Mutex::Mutex()
: mHandle( ::CreateMutex( NULL, false, NULL ) )
{
}

Mutex::~Mutex()
{
  if( mHandle != NULL )
    ::CloseHandle( mHandle );
}

bool
Mutex::Acquire() const
{
  return ( WAIT_OBJECT_0 == ::WaitForSingleObject( mHandle, INFINITE ) );
}

bool
Mutex::Release() const
{
  return ::ReleaseMutex( mHandle );
}

#else // _WIN32

Mutex::Mutex()
{
  ::pthread_mutexattr_t attributes;
  if( !::pthread_mutexattr_init( &attributes ) )
  {
    ::pthread_mutexattr_settype( &attributes, PTHREAD_MUTEX_RECURSIVE );
    ::pthread_mutex_init( &mMutex, &attributes );
    ::pthread_mutexattr_destroy( &attributes );
  }
}

Mutex::~Mutex()
{
  ::pthread_mutex_destroy( &mMutex );
}

bool
Mutex::Acquire() const
{
  return 0 == ::pthread_mutex_lock( &mMutex );
}

bool
Mutex::Release() const
{
  return 0 == ::pthread_mutex_unlock( &mMutex );
}

#endif // _WIN32

Mutex::Lock::Lock( const Mutex& inMutex )
: mpMutex( &inMutex )
{
  DoAcquire();
}

// Mutex::Lock class
Mutex::Lock::Lock( const Mutex* inpMutex )
: mpMutex( inpMutex )
{
  DoAcquire();
}

Mutex::Lock::~Lock()
{
  if( mpMutex )
    mpMutex->Release();
}

void
Mutex::Lock::DoAcquire()
{
  if( mpMutex && !mpMutex->Acquire() )
    mpMutex = NULL;
}

// Mutex::Unlock class
Mutex::Unlock::Unlock( const Mutex& inMutex )
: mpMutex( &inMutex )
{
  DoRelease();
}

Mutex::Unlock::Unlock( const Mutex* inpMutex )
: mpMutex( inpMutex )
{
  DoRelease();
}

void
Mutex::Unlock::DoRelease()
{
  if( mpMutex && !mpMutex->Release() )
    mpMutex = NULL;
}

Mutex::Unlock::~Unlock()
{
  if( mpMutex )
    mpMutex->Acquire();
}

