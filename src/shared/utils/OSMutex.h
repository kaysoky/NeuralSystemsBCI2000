//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for mutex objects.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#ifndef OS_MUTEX_H
#define OS_MUTEX_H

class OSMutex
{
 public:
  OSMutex();
  virtual ~OSMutex();

  bool Acquire() const;
  bool Release() const;

  class Lock
  { // An object that locks a mutex during its lifetime.
   public:
    Lock( const OSMutex& );
    Lock( const OSMutex* );
    ~Lock();

   private:
    void DoAcquire();
    const OSMutex* mpMutex;
  };

  class Unlock
  { // An object that unlocks a mutex during its lifetime.
   public:
    Unlock( const OSMutex& );
    Unlock( const OSMutex* );
    ~Unlock();

   private:
    void DoRelease();
    const OSMutex* mpMutex;
  };

 private:
  HANDLE mHandle;
};

#endif // OS_THREAD_H
