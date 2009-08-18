//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A thread class similar to the VCL's TThread.
//   To implement your own thread, create a class that inherits from
//   OSThread, and put your own functionality into its
//   Execute() function.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#ifndef OS_THREAD_H
#define OS_THREAD_H

#include "Windows.h"

class OSThread
{
 public:
  OSThread( bool createSuspended = false );
  virtual ~OSThread();

  unsigned int ID()
    { return mThreadID; }

  void Suspend();
  void Resume();

  void Terminate();
  bool IsTerminating() const
    { return mTerminating; }
  bool IsTerminated() const
    { return mHandle == NULL; }

  int  Result() const
    { return mResult; }

 protected:
  virtual int Execute();

 private:
  static DWORD WINAPI StartThread( void* inInstance );

  HANDLE mHandle;
  DWORD  mThreadID;
  int    mResult;
  volatile bool mTerminating;
};

#endif // OS_THREAD_H
