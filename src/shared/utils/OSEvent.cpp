//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for event objects.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "OSEvent.h"

#include <windows.h>

OSEvent::OSEvent()
: mHandle( ::CreateEvent( NULL, true, true, NULL ) )
{
}

OSEvent::~OSEvent()
{
  if( mHandle != NULL )
    ::CloseHandle( mHandle );
}

bool
OSEvent::Set() const
{
  return ::SetEvent( mHandle );
}

bool
OSEvent::Reset() const
{
  return ::ResetEvent( mHandle );
}

bool
OSEvent::Wait() const
{
  return ( WAIT_OBJECT_0 == ::WaitForSingleObject( mHandle, INFINITE ) );
}

