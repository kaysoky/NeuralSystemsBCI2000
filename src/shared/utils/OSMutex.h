//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for mutex objects.
//
// (C) 2000-2008, BCI2000 Project
///////////////////////////////////////////////////////////////////////
#ifndef OS_MUTEX_H
#define OS_MUTEX_H

#include <Uncopyable.h>

#ifdef _WIN32
# include <windows.h>
#else
# include <pthread.h>
#endif // _WIN32

class OSMutex : private Uncopyable
{
 public:
  OSMutex();
  virtual ~OSMutex();

  bool Acquire() const;
  bool Release() const;

  class Lock : private Uncopyable
  { // A RAAI object that locks a mutex.
   public:
    Lock( const OSMutex& );
    Lock( const OSMutex* );
    ~Lock();

   private:
    void DoAcquire();
    const OSMutex* mpMutex;
  };

  class Unlock : private Uncopyable
  { // A RAAI object that unlocks a mutex.
   public:
    Unlock( const OSMutex& );
    Unlock( const OSMutex* );
    ~Unlock();

   private:
    void DoRelease();
    const OSMutex* mpMutex;
  };

 private:
#ifdef _WIN32
  HANDLE mHandle;
#else // _WIN32
	mutable pthread_mutex_t mMutex;
#endif // _WIN32
};

#endif // OS_THREAD_H
