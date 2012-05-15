//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A thread class similar to the VCL's TThread.
//   To implement your own thread, create a class that inherits from
//   OSThread, and put your own functionality into its
//   Execute() function.
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

#include "OSThread.h"
#include "OSEvent.h"
#include "BCIException.h"
#include "OSError.h"
#include "ClassName.h"
#include "ExceptionCatcher.h"

#if _WIN32
# include <Process.h>
#else // _WIN32
# include <unistd.h>
#endif // !_WIN32

#include <cstdlib>
#include <cstring>
#include <cerrno>

using namespace std;

#if _WIN32

OSThread::OSThread()
: mThreadID( 0 ),
  mResult( 0 ),
  mTerminating( false ),
  mTerminated( true ),
  mpTerminationEvent( new OSEvent )
{
  mpTerminationEvent->Set();
}

void
OSThread::Start()
{
  TerminateWait();
  OSMutex::Lock lock( mMutex );
  mTerminating = false;
  uintptr_t handle = ::_beginthreadex( NULL, 0, OSThread::StartThread, this, 0, &mThreadID );
  if( handle )
  {
    ::CloseHandle( HANDLE( handle ) );
    mTerminated = false;
    mpTerminationEvent->Reset();
  }
  else
    throw bciexception( ::strerror( errno ) );
}

bool
OSThread::InOwnThread() const
{
  return ::GetCurrentThreadId() == mThreadID;
}

#else // _WIN32

OSThread::OSThread()
: mTerminated( true ),
  mResult( 0 ),
  mTerminating( false ),
  mpTerminationEvent( new OSEvent )
{
  mpTerminationEvent->Set();
}

void
OSThread::Start()
{
  TerminateWait();
  OSMutex::Lock lock( mMutex );
  mTerminating = false;
  ::pthread_attr_t attributes;
  if( !::pthread_attr_init( &attributes ) )
  {
    // Create a thread in detached mode, i.e. thread resources are
    // auto-released when the thread terminates.
    ::pthread_attr_setdetachstate( &attributes, PTHREAD_CREATE_DETACHED );
    if( !::pthread_create( &mThread, &attributes, OSThread::StartThread, this ) )
    {
      mTerminated = false;
      mpTerminationEvent->Reset();
    }
    ::pthread_attr_destroy( &attributes );
  }
}

bool
OSThread::InOwnThread() const
{
  return ::pthread_equal( pthread_self(), mThread );
}

#endif // _WIN32

OSThread::~OSThread()
{
  if( !IsTerminated() )
    throw bciexception(
      "Thread still running when being destructed -- "
      "call OSThread::TerminateWait() from your derived class' destructor for a fix"
    );
}

SharedPointer<OSEvent>
OSThread::Terminate()
{
  OSMutex::Lock lock( mMutex );
  mTerminating = true;
  return mpTerminationEvent;
}

bool
OSThread::TerminateWait( int inTimeout )
{
  if( InOwnThread() )
    throw bciexception( "Called from own thread, throwing exception to prevent deadlock" );
  return Terminate()->Wait( inTimeout );
}

int
OSThread::CallExecute()
{
  MemberCall<int(OSThread*)> call( &OSThread::OnExecute, this );
  bool finished = ExceptionCatcher()
    .SetMessage( "Canceling thread of type " + bci::ClassName( typeid( *this ) ) )
    .Run( call );
  return finished ? call.Result() : -1;
}

void
OSThread::CallFinished()
{
  MemberCall<void(OSThread*)> call( &OSThread::OnFinished, this );
  ExceptionCatcher()
  .SetMessage( "Canceling thread of type " + bci::ClassName( typeid( *this ) ) )
  .Run( call );
}

#if _WIN32

unsigned int WINAPI
OSThread::StartThread( void* inInstance )
{
  OSThread* this_ = reinterpret_cast<OSThread*>( inInstance );
  int result = this_->CallExecute();
  this_->FinishThread( result );
  return result;
}

#else // _WIN32

void*
OSThread::StartThread( void* inInstance )
{
  OSThread* this_ = reinterpret_cast<OSThread*>( inInstance );
  // Set the cancel state to asynchronous, so pthread_cancel() will
  // immediately cancel thread execution.
  ::pthread_setcancelstate( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
  int result = this_->CallExecute();
  this_->FinishThread( result );
  return reinterpret_cast<void*>( result );
}

#endif // _WIN32

void
OSThread::FinishThread( int inResult )
{
  // The OnFinished() handler may delete the OSThread object, so we
  // use a temporary shared pointer to the termination event.
  SharedPointer<OSEvent> pTerminationEvent;
  {
    OSMutex::Lock lock( mMutex );
    mResult = inResult;
    mTerminated = true;
    pTerminationEvent = mpTerminationEvent;
  }
  CallFinished();
  pTerminationEvent->Set();
}

