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
#ifndef LOCKABLE_H
#define LOCKABLE_H

#include "OSMutex.h"
#include "BCIAssert.h"

class Lockable
{
 public:
  Lockable() {}
  Lockable( const Lockable& ) {}
  virtual ~Lockable() {}
  Lockable& operator=( const Lockable& ) { return *this; }

  bool Lock() const   { return mMutex.Acquire(); }
  bool Unlock() const { return mMutex.Release(); }

 private:
  OSMutex mMutex;
};

template<typename T> class Lock;

template<typename T>
class Lock
{
 private:
  Lock();
  Lock& operator=( const Lock& );

 public:
  Lock( T& t )
    : mpT( &t ), mcpT( 0 )
    { t.Lock(); }
  Lock( const T& t )
    : mpT( 0 ), mcpT( &t )
    { t.Lock(); }
  Lock( T* pT )
    : mpT( pT ), mcpT( 0 )
    { pT->Lock(); }
  Lock( const T* cpT )
    : mpT( 0 ), mcpT( cpT )
    { cpT->Lock(); }
  // A copy constructor is required by some compilers that need it to implement
  //   return Lock( x );
  // An object of type T must allow recursive locking, otherwise the copy
  // constructor will block.
  Lock( const Lock& other )
    : mpT( other.mpT ), mcpT( other.mcpT )
    { if( mpT ) mpT->Lock(); else if( mcpT ) mcpT->Lock(); }
  ~Lock()
    { if( mpT ) mpT->Unlock(); else if( mcpT ) mcpT->Unlock(); }
    
  const T& ConstRef() const
    { return mpT ? *mpT : *mcpT; }
  T& MutableRef() const
    { bciassert( mpT ); return *mpT; }
  T& operator()() const
    { return MutableRef(); }

 private:
  T* mpT;
  const T* mcpT;
};

template<typename T>
class Lock<const T> : public Lock<T>
{
  public:
   Lock( const T* t ) : Lock<T>( t ) {}
   Lock( const T& t ) : Lock<T>( t ) {}
   const T& operator()() const
    { return Lock<T>::ConstRef(); }
};

template<typename T>
Lock<T>
TemporaryLock( T& t )
{ // Some compilers require a copy constructor here.
  return Lock<T>( t );
}

#endif // LOCKABLE_H

