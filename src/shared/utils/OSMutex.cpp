//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for mutex objects.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "OSMutex.h"

#include <windows.h>

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

