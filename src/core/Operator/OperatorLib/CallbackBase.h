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
#ifndef CALLBACK_BASE_H
#define CALLBACK_BASE_H

#include "OSMutex.h"
#include "OSEvent.h"
#include <map>

#if _WIN32
# include <windows.h>
# define STDCALL __stdcall
#else
# include <pthread.h>
# define STDCALL
#endif

class CallbackBase
{
 public:
  typedef int (STDCALL *Function)();

  enum Result
  {
    Ignored = 0, // no callback function registered for event
    OK = 1,      // event handled, ok
    Error = 2,   // event handled, error
  };

  // Callback thread context
  //  CallingThread: The callback function is executed in the thread from
  //                 which the call is issued.
  //  MainThread:    The call is buffered, and executed when the main thread calls
  //                 CallbackBase::CheckExternalCallback(). The calling thread is
  //                 blocked until the buffered call exits.
  enum Context
  {
    CallingThread = 0,
    DefaultContext = CallingThread, // default should be 0 for consistency with std::map
    MainThread = 1,
  };

 protected:
  CallbackBase();

 public:
  ~CallbackBase() {}

  CallbackBase& SetCallback( int event, Function function, void* data, Context = DefaultContext );

  CallbackBase& SetCallbackFunction( int event, Function function );
  Function CallbackFunction( int event ) const;
  CallbackBase& SetCallbackData( int event, void* data );
  void* CallbackData( int event ) const;
  CallbackBase& SetCallbackContext( int event, Context );
  Context CallbackContext( int event ) const;

  bool CheckPendingCallback();

  Result ExecuteCallback( int event );
  template<class T>
   Result ExecuteCallback( int event, T );
  template<class T , class U>
   Result ExecuteCallback( int event, T, U );
  template<class T , class U, class V>
   Result ExecuteCallback( int event, T, U, V );
  template<class T , class U, class V, class W>
   Result ExecuteCallback( int event, T, U, V, W );

 private:
  // A callback object stores all information required to execute a callback function.
  class Callback
  {
   public:
    Callback( Function function, void* data, CallbackBase::Context );
    virtual ~Callback();

    CallbackBase::Context Context() const
      { return mContext; }
    CallbackBase::Result Result() const
      { return mResult; }
    void SuspendThread();
    void ResumeThread();
    virtual void Execute() = 0;

   protected:
    Function mFunction;
    void*    mData;
    OSEvent* mpWaitEvent;
    CallbackBase::Context mContext;
    CallbackBase::Result  mResult;
  };

  // Callback descendants provide a way to accommodate a variable number of
  // arguments to callback functions.
  class Callback0 : public Callback
  {
   public:
    Callback0( Function function, void* data, CallbackBase::Context c )
      : Callback( function, data, c )
      {}
    virtual void Execute();
  };

  template<class T>
  class Callback1 : public Callback
  {
   public:
    Callback1( Function function, void* data, CallbackBase::Context c, T t )
      : Callback( function, data, c ),
        mT( t )
      {}
    virtual void Execute();

   private:
    T mT;
  };

  template<class T, class U>
  class Callback2 : public Callback
  {
   public:
    Callback2( Function function, void* data, CallbackBase::Context c, T t, U u )
      : Callback( function, data, c ),
        mT( t ),
        mU( u )
      {}
    virtual void Execute();

   private:
    T mT;
    U mU;
  };

  template<class T, class U, class V>
  class Callback3 : public Callback
  {
   public:
    Callback3( Function function, void* data, CallbackBase::Context c, T t, U u, V v )
      : Callback( function, data, c ),
        mT( t ),
        mU( u ),
        mV( v )
      {}
    virtual void Execute();

   private:
    T mT;
    U mU;
    V mV;
  };

  template<class T, class U, class V, class W>
  class Callback4 : public Callback
  {
   public:
    Callback4( Function function, void* data, CallbackBase::Context c, T t, U u, V v, W w )
      : Callback( function, data, c ),
        mT( t ),
        mU( u ),
        mV( v ),
        mW( w )
      {}
    virtual void Execute();

   private:
    T mT;
    U mU;
    V mV;
    W mW;
  };

  Result DoExecute( Callback& );
  bool   CurrentlyInMainThread() const;

  typedef std::map<int, Function> FunctionMap;
  typedef std::map<int, void*>    DataMap;
  typedef std::map<int, Context>  ContextMap;
  FunctionMap    mFunction;
  DataMap        mData;
  ContextMap     mContext;

#if _WIN32
  size_t         mMainThreadID; // ID of the thread in which the CallbackBase constructor was executed.
#else
  pthread_t      mMainThreadID;
#endif
  OSMutex        mMainThreadAccess, // A mutex that blocks multiple threads while waiting for the main thread to become available.
                 mPendingCallbackAccess; // A mutex that protects access to the mpPendingCallback variable.
  Callback*      mpPendingCallback;
};


template<class T>
CallbackBase::Result
CallbackBase::ExecuteCallback( int inEvent, T t )
{
  if( mFunction[inEvent] != NULL )
  {
    Callback1<T> c( mFunction[inEvent], mData[inEvent], mContext[inEvent], t );
    return DoExecute( c );
  }
  return Ignored;
}

template<class T, class U>
CallbackBase::Result
CallbackBase::ExecuteCallback( int inEvent, T t, U u )
{
  if( mFunction[inEvent] != NULL )
  {
    Callback2<T, U> c( mFunction[inEvent], mData[inEvent], mContext[inEvent], t, u );
    return DoExecute( c );
  }
  return Ignored;
}

template<class T, class U, class V>
CallbackBase::Result
CallbackBase::ExecuteCallback( int inEvent, T t, U u, V v )
{
  if( mFunction[inEvent] != NULL )
  {
    Callback3<T, U, V> c( mFunction[inEvent], mData[inEvent], mContext[inEvent], t, u, v );
    return DoExecute( c );
  }
  return Ignored;
}

template<class T, class U, class V, class W>
CallbackBase::Result
CallbackBase::ExecuteCallback( int inEvent, T t, U u, V v, W w )
{
  if( mFunction[inEvent] != NULL )
  {
    Callback4<T, U, V, W> c( mFunction[inEvent], mData[inEvent], mContext[inEvent], t, u, v, w );
    return DoExecute( c );
  }
  return Ignored;
}

template<class T>
void
CallbackBase::Callback1<T>::Execute()
{
  typedef CallbackBase::Result (STDCALL *Fn)( void*, T );
  mResult = reinterpret_cast<Fn>( mFunction )( mData, mT );
}

template<class T, class U>
void
CallbackBase::Callback2<T, U>::Execute()
{
  typedef CallbackBase::Result (STDCALL *Fn)( void*, T, U );
  mResult = reinterpret_cast<Fn>( mFunction )( mData, mT, mU );
}

template<class T, class U, class V>
void
CallbackBase::Callback3<T, U, V>::Execute()
{
  typedef CallbackBase::Result (STDCALL *Fn)( void*, T, U, V );
  mResult = reinterpret_cast<Fn>( mFunction )( mData, mT, mU, mV );
}

template<class T, class U, class V, class W>
void
CallbackBase::Callback4<T, U, V, W>::Execute()
{
  typedef CallbackBase::Result (STDCALL *Fn)( void*, T, U, V, W );
  mResult = reinterpret_cast<Fn>( mFunction )( mData, mT, mU, mV, mW );
}

#endif // CALLBACK_BASE_H

