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
#include "BCIException.h"
#include "ClassName.h"
#include "ExceptionCatcher.h"

#ifdef _WIN32
# include <Windows.h>
# include <Process.h>
#else
# include <pthread.h>
# include <unistd.h>
#endif // _WIN32

#include <cstdlib>
#include <cstring>
#include <cerrno>

using namespace std;

struct ThreadStarter_
{
#ifdef _WIN32
  static unsigned int WINAPI ThreadProc( void* inData )
  { return reinterpret_cast<OSThread*>( inData )->RunThread(); }
  static bool StartThread( void* data )
  {
    uintptr_t handle = ::_beginthreadex( 0, 0, &ThreadStarter_::ThreadProc, data, 0, 0 );
    if( handle )
      ::CloseHandle( HANDLE( handle ) );
    return handle != 0;
  }
#else // _WIN32
  static void* ThreadProc( void* inData )
  {
    // Set the cancel state to asynchronous, so pthread_cancel() will
    // immediately cancel thread execution.
    ::pthread_setcancelstate( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
    return reinterpret_cast<void*>( reinterpret_cast<OSThread*>( inData )->RunThread() );
  }
  static bool StartThread( void* data )
  {
    ::pthread_attr_t attributes;
    errno = ::pthread_attr_init( &attributes );
    if( !errno )
    {
      // Create a thread in detached mode, i.e. thread resources are
      // auto-released when the thread terminates.
      ::pthread_attr_setdetachstate( &attributes, PTHREAD_CREATE_DETACHED );
      pthread_t thread;
      errno = ::pthread_create( &thread, &attributes, &ThreadStarter_::ThreadProc, data );
      ::pthread_attr_destroy( &attributes );
    }
    return errno == 0;
  }
#endif // !_WIN32
};

OSThread::OSThread()
: mThreadID( false ),
  mResult( 0 ),
  mTerminating( false ),
  mTerminated( true ),
  mpTerminationEvent( new OSEvent )
{
  mpTerminationEvent->Set();
}

OSThread::~OSThread()
{
  if( !IsTerminated() )
    throw std_logic_error(
      "Thread still running when being destructed -- "
      "call OSThread::TerminateWait() from your derived class' destructor for a fix"
    );
}

SharedPointer<OSEvent>
OSThread::Terminate()
{
  SpinLock::Lock _( mLock );
  mTerminating = true;
  return mpTerminationEvent;
}

bool
OSThread::TerminateWait( int inTimeout )
{
  if( InOwnThread() )
    throw std_logic_error( "Called from own thread, throwing exception to prevent deadlock" );
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

void
OSThread::Start()
{
  TerminateWait();
  SpinLock::Lock _( mLock );
  mTerminating = false;
  if( ThreadStarter_::StartThread( this ) )
  {
    mTerminated = false;
    mpTerminationEvent->Reset();
  }
  else
    throw std_runtime_error( ::strerror( errno ) );
}

int
OSThread::RunThread()
{
  mThreadID = ThreadUtils::ThreadID( true );
  int result = CallExecute();
  // The OnFinished() handler may delete the OSThread object, so we
  // use a temporary shared pointer to the termination event.
  SharedPointer<OSEvent> pTerminationEvent;
  {
    SpinLock::Lock _( mLock );
    mResult = result;
    mTerminated = true;
    pTerminationEvent = mpTerminationEvent;
  }
  mThreadID = ThreadUtils::ThreadID( false );
  CallFinished();
  pTerminationEvent->Set();
  return result;
}

bool
OSThread::InOwnThread() const
{
  return ThreadUtils::ThreadID() == mThreadID;
}

