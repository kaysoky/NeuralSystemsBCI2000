////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A mix-in class that maintains pointers to callback functions,
//   and allows to call them from within a descendant using
//   CallbackBase::ExecuteCallback().
//   The signature of the callback function is determined from the arguments
//   to the ExecuteCallback() function, with the first argument always being
//   a void* specified as callback data, and following arguments being taken
//   from the ExecuteCallback() function call.
//   E.g., when calling
//     ExecuteCallback( MyEvent, "some string", long( 3 ) );
//   the callback function signature will be
//     void OnMyEvent( const char*, long );
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
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "CallbackBase.h"
#include "BCIError.h"
#include "BCIException.h"

using namespace std;

CallbackBase::CallbackBase()
: mMainThreadID( 0 ),
  mpPendingCallback( NULL )
{
#if _WIN32
  mMainThreadID = ::GetCurrentThreadId();
#else
  mMainThreadID = ::pthread_self();
#endif
}

CallbackBase&
CallbackBase::SetCallback( int inEvent, Function inCallbackFn, void* inData, Context inContext )
{
  return this->SetCallbackFunction( inEvent, inCallbackFn )
              .SetCallbackData( inEvent, inData )
              .SetCallbackContext( inEvent, inContext );
}

CallbackBase&
CallbackBase::SetCallbackFunction( int inEvent, Function inCallbackFn )
{
  mFunction[inEvent] = inCallbackFn;
  return *this;
}

CallbackBase::Function
CallbackBase::CallbackFunction( int inEvent ) const
{
  FunctionMap::const_iterator i = mFunction.find( inEvent );
  return i == mFunction.end() ? NULL : i->second;
}

CallbackBase&
CallbackBase::SetCallbackData( int inEvent, void* inData )
{
  mData[inEvent] = inData;
  return *this;
}

void*
CallbackBase::CallbackData( int inEvent ) const
{
  DataMap::const_iterator i = mData.find( inEvent );
  return i == mData.end() ? NULL : i->second;
}

CallbackBase&
CallbackBase::SetCallbackContext( int inEvent, Context inContext )
{
  mContext[inEvent] = inContext;
  return *this;
}

CallbackBase::Context
CallbackBase::CallbackContext( int inEvent ) const
{
  ContextMap::const_iterator i = mContext.find( inEvent );
  return i == mContext.end() ? DefaultContext : i->second;
}

CallbackBase::Result
CallbackBase::ExecuteCallback( int inEvent )
{
  if( mFunction[inEvent] != NULL )
  {
    Callback0 c( mFunction[inEvent], mData[inEvent], mContext[inEvent] );
    DoExecute( c );
  }
  return Ignored;
}

CallbackBase::Result
CallbackBase::DoExecute( Callback& inCallback )
{
  Context context = inCallback.Context();
  // Suspending the main thread would not be such a good idea.
  if( context == MainThread && CurrentlyInMainThread() )
    context = CallingThread;

  // During callback execution, any existing lock on the callback descendant must
  // be released; otherwise, deadlock will occur if a callback function calls
  // a member function that again acquires such a lock.
  switch( context )
  {
    case MainThread:
      {
        OSMutex::Lock lock( mMainThreadAccess );
        {
          OSMutex::Lock lock( mPendingCallbackAccess );
          mpPendingCallback = &inCallback;
        }
        // Suspend execution of the calling thread while the callback function
        // is being executed in the main thread.
        inCallback.SuspendThread();
      }
      break;

    case CallingThread:
      inCallback.Execute();
      break;

    default:
      throw bciexception( "Unknown callback execution context:" << context );
  }
  return inCallback.Result();
}

bool
CallbackBase::CurrentlyInMainThread() const
{
#if _WIN32
  return mMainThreadID == ::GetCurrentThreadId();
#else
  return ::pthread_equal( mMainThreadID, ::pthread_self() );
#endif
}

bool
CallbackBase::CheckPendingCallback()
{
#if BCIDEBUG
  if( !CurrentlyInMainThread() )
  {
    bcierr__ << "CallbackBase::CheckPendingCallback() called from a thread that is not the main thread"
             << endl;
    return false;
  }
#endif // BCIDEBUG
  OSMutex::Lock lock( mPendingCallbackAccess );
  if( mpPendingCallback != NULL )
  {
    mpPendingCallback->Execute();
    mpPendingCallback->ResumeThread();
    mpPendingCallback = NULL;
    return true;
  }
  return false;
}


// CallbackBase::Callback
CallbackBase::Callback::Callback( CallbackBase::Function inFunction, void* inData, CallbackBase::Context inContext )
: mFunction( inFunction ),
  mData( inData ),
  mpWaitEvent( NULL ),
  mContext( inContext ),
  mResult( CallbackBase::Ignored )
{
  if( inContext == CallbackBase::MainThread )
    mpWaitEvent = new OSEvent;
}

CallbackBase::Callback::~Callback()
{
  delete mpWaitEvent;
}

void
CallbackBase::Callback::SuspendThread()
{
  if( !mpWaitEvent )
    throw bciexception( "trying to wait for NULL event" );
  mpWaitEvent->Wait();
}

void
CallbackBase::Callback::ResumeThread()
{
  if( mpWaitEvent )
    mpWaitEvent->Set();
}


void
CallbackBase::Callback0::Execute()
{
  typedef CallbackBase::Result (STDCALL *Fn)( void* );
  mResult = reinterpret_cast<Fn>( mFunction )( mData );
}


