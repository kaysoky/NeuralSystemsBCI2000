//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A thread class similar to the VCL's TThread.
//   To implement your own thread, create a class that inherits from
//   OSThread, and put your own functionality into its
//   OnExecute() function.
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
#ifndef OS_THREAD_H
#define OS_THREAD_H

#ifdef _WIN32
# include <Windows.h>
#else
# include <pthread.h>
#endif // _WIN32

#include "Uncopyable.h"
#include "ThreadUtils.h"
#include "OSEvent.h"
#include "OSMutex.h"
#include "PrecisionTime.h"
#include "SharedPointer.h"
#include <vector>

class OSThread : private Uncopyable
{
 public:
  OSThread();
  virtual ~OSThread();

  void Start();
  SharedPointer<OSEvent> Terminate();
  bool TerminateWait( int timeout_ms = OSEvent::cInfiniteTimeout ); // returns false on timeout
  bool IsTerminating() const
    { return mTerminating; }
  bool IsTerminated() const
    { OSMutex::Lock lock( mMutex ); return mTerminated; }

  bool InOwnThread() const;

  int Result() const
    { return mResult; }

 private:
  virtual int Execute() { return 0; }
  virtual int OnExecute() { return Execute(); } // Now named as an event handler.
  virtual void OnFinished() {}

 public: // These utility functions are now obsolete.
         // Use their counterparts from the ThreadUtils.h header instead.
  static void SleepFor( int inMs )
    { ThreadUtils::SleepFor( inMs ); }// sleep for milliseconds
  static void Sleep( int inMs )
    { SleepFor( inMs ); }
  static void PrecisionSleepFor( double inMs ) // sleep for milliseconds
    { ThreadUtils::PrecisionSleepFor( inMs ); }
  static void PrecisionSleepUntil( PrecisionTime inWakeup ) // sleep until absolute wakeup time
    { ThreadUtils::PrecisionSleepUntil( inWakeup ); }
  static bool IsMainThread()
    { return InMainThread(); }
  static bool InMainThread()
    { return ThreadUtils::InMainThread(); }
  static int NumberOfProcessors()
    { return ThreadUtils::NumberOfProcessors(); }

 private:
  int CallExecute();
  void CallFinished();

#ifdef _WIN32
  static unsigned int WINAPI StartThread( void* inInstance );
  void FinishThread( int );

  unsigned int mThreadID;
  static unsigned int sMainThreadID;
#else // _WIN32
  static void* StartThread( void* inInstance );
  void FinishThread( int );

  pthread_t mThread;
  static pthread_t sMainThread;
#endif // _WIN32
  int mResult;
  OSMutex mMutex;
  SharedPointer<OSEvent> mpTerminationEvent;
  bool mTerminating,
       mTerminated;
};

#endif // OS_THREAD_H
