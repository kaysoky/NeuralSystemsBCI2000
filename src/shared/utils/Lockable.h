//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Classes/templates related to locking.
//   *Lockable is a mixin-class to add locking to a class,
//   *Lock<T> defines RAAI objects that lock a lockable
//    object during the lifetime of the RAAI object.
//    For a RAAI class that operates on an OSMutex rather than
//    a Lockable, see OSMutex::Lock.
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
class Lock
{
 private:
  Lock();
  Lock& operator=( const Lock& );

 public:
  Lock( T& t )
    : mrT( t )
    { mrT.Lock(); }
  // A copy constructor is required by some compilers that need it to implement
  //   return Lock( x );
  // An object of type T must allow recursive locking, otherwise the copy
  // constructor will block.
  Lock( const Lock& lock )
    : mrT( lock.mrT )
    { mrT.Lock(); }
  ~Lock()
    { mrT.Unlock(); }
  T& operator()() const
    { return mrT; }

 private:
  T& mrT;
};

template<typename T>
Lock<T>
TemporaryLock( T& t )
{ // Some compilers require a copy constructor here.
  return Lock<T>( t );
}

#endif // LOCKABLE_H
