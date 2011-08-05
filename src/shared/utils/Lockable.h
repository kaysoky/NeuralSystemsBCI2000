//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Classes/templates related to locking.
//   *Lockable is a mixin-class to add locking to a class,
//   *Lock<T> and Unlock<T> define RAAI objects that lock a lockable 
//    object while the RAAI object exists.
//   *TemporaryLock<T>() is a function template that returns a
//    temporary Lock<T> object. This allows, e.g., to lock a stream
//    object during evaluation of an expression that writes into the
//    stream object.
//
// (C) 2000-2008, BCI2000 Project
///////////////////////////////////////////////////////////////////////
#ifndef LOCKABLE_H
#define LOCKABLE_H

#include "Uncopyable.h"
#include "OSMutex.h"

class Lockable : private Uncopyable
{
 public:
  Lockable()          {}
  virtual ~Lockable() {}
  bool Lock() const   { return mMutex.Acquire(); }
  bool Unlock() const { return mMutex.Release(); }

 private:
  OSMutex mMutex;
};

template<typename T>
class Lock : private Uncopyable
{
 private:
  Lock();

 public:
  Lock( T& t )
    : mrT( t ),
      mLocked( t.Lock() )
    {}
  ~Lock()
    { if( mLocked ) mrT.Unlock(); }
  T& operator()() const
    { return mrT; }

 private:
  T& mrT;
  bool mLocked;
};

template<typename T>
class Unlock : private Uncopyable
{
 private:
  Unlock();

 public:
  Unlock( T& t )
    : mrT( t ),
      mUnlocked( t.Unlock() )
    {}
  ~Unlock()
    { if( mUnlocked ) mrT.Lock(); }
  T& operator()() const
    { return mrT; }

 private:
  T& mrT;
  bool mUnlocked;
};

template<typename T>
Lock<T>
TemporaryLock( T& t )
{
  return Lock<T>( t );
}

#endif // LOCKABLE_H
