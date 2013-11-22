//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A thread class similar to the VCL's TThread.
//   To implement your own thread, create a class that inherits from
//   Thread, and put your own functionality into its
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
#ifndef TINY_THREAD_H
#define TINY_THREAD_H

#include "Uncopyable.h"
#include "Waitable.h"
#include "Synchronized.h"
#include "SpinLock.h"
#include "SharedPointer.h"

namespace Tiny
{

struct ThreadStarter_;

class Thread : private Uncopyable
{
 public:
  Thread();
  virtual ~Thread();

  void Start();
  SharedPointer<Waitable> Terminate();
  bool TerminateWait( int timeout_ms = InfiniteTimeout ); // returns false on timeout
  bool IsTerminating() const
    { return mTerminating; }
  bool IsTerminated() const
    { return mTerminated; }

  bool InOwnThread() const;

  int Result() const
    { return mResult; }

 private:
  virtual int Execute() { return 0; }
  virtual int OnExecute() { return Execute(); } // Now named as an event handler.
  virtual void OnFinished() {}

 private:
  int RunThread();
  int CallExecute();
  void CallFinished();
  friend struct ThreadStarter_;

  ThreadUtils::ThreadID mThreadID;
  int mResult;
  SpinLock mLock;
  SharedPointer<Waitable> mpTerminationEvent;
  Synchronized<bool> mTerminating, mTerminated;
};

} // namespace

using Tiny::Thread;

#endif // TINY_THREAD_H
