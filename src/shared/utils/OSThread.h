#ifndef OS_THREAD_H
#define OS_THREAD_H

#include "Thread.h"

struct OSThread : Tiny::Thread
{
 protected: // These utility functions are now obsolete.
            // Use their counterparts from the ThreadUtils.h header instead.
  static void SleepFor( int inMs )
    { ThreadUtils::SleepFor( inMs ); }
  static void Sleep( int inMs )
    { SleepFor( inMs ); }
  static void SleepUntil( PrecisionTime inWakeup )
    { ThreadUtils::SleepUntil( inWakeup ); }
  static bool IsMainThread()
    { return InMainThread(); }
  static bool InMainThread()
    { return ThreadUtils::InMainThread(); }
  static int NumberOfProcessors()
    { return ThreadUtils::NumberOfProcessors(); }
};

#endif // OS_THREAD_H
