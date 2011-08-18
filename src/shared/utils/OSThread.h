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
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#include "OSEvent.h"

class OSThread : private Uncopyable
{
 public:
  OSThread();
  virtual ~OSThread();

  void Start();
  void Terminate( OSEvent* = NULL );
  bool TerminateWait( int timeout_ms = OSEvent::cInfiniteTimeout ); // returns false on timeout
  bool IsTerminating() const
    { return mTerminating; }
  bool IsTerminated() const;

  int  Result() const
    { return mResult; }

  static void Sleep( int ); // milliseconds
  static bool IsMainThread();

 private:
  virtual int Execute() = 0;

  int CallExecute();

 private:
#ifdef _WIN32
  static DWORD WINAPI StartThread( void* inInstance );

  volatile HANDLE mHandle;
  DWORD  mThreadID;
  static DWORD sMainThreadID;
#else // _WIN32
  static void* StartThread( void* inInstance );

  pthread_t mThread;
  volatile bool mTerminated;
  static pthread_t sMainThread;
#endif // _WIN32
  int mResult;
  OSEvent* volatile mpTerminationEvent;
  volatile bool mTerminating;
};

#endif // OS_THREAD_H
