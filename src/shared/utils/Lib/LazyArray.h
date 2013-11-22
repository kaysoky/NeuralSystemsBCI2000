////////////////////////////////////////////////////////////////////////////////
// $Id: SharedPointer.h 3906 2012-04-04 14:22:44Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An array that implements a copy-on-write strategy.
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
////////////////////////////////////////////////////////////////////////////////
#ifndef TINY_LAZY_ARRAY_H
#define TINY_LAZY_ARRAY_H

#include "SharedPointer.h"
#include "Lockable.h"
#include "BCIException.h"

namespace Tiny
{

template<class T>
class LazyArray
{
 public:
  class Memory
  {
   public:
    typedef T ValueType;
    Memory( T* t, size_t c ) : ptr( t ), count( c ) {}
    virtual ~Memory() {}
    T* Ptr() { return ptr; }
    const T* Ptr() const { return ptr; }
    size_t Count() const { return count; }
    void CopyFrom( const T* t ) { if( ptr != t ) ::memcpy( ptr, t, count * sizeof( T ) ); }
   private:
    T* ptr;
    size_t count;
  };
  typedef SharedPointer<Memory> MemoryObj;

 private:
  struct DefaultMemory : Memory
  {
    DefaultMemory( size_t count )
      : Memory( new T[count], count ) {}
    DefaultMemory( const T* t, size_t count )
      : Memory( new T[count], count ) { CopyFrom( t ); }
    ~DefaultMemory() { delete Ptr(); }
  };

 public:
  LazyArray()
    : mMemory( 0 ), mShared( false )
    {}
  LazyArray( size_t count )
    : mMemory( new DefaultMemory( count ) ), mShared( false )
    {}
  LazyArray( MemoryObj memory )
    : mMemory( memory ), mShared( false )
    {}
  LazyArray( const LazyArray& a )
    : mMemory( 0 ), mShared( false )
    { ShallowAssignFrom( a ); }
  LazyArray& operator=( const LazyArray& a )
    { return ShallowAssignFrom( a ); }
  size_t Count() const
    { return mMemory ? mMemory->Count() : 0; }
  const T& operator[]( size_t idx ) const
    { return mMemory->Ptr()[idx]; }
  T& operator[]( size_t idx )
    { OnWrite(); return mMemory->Ptr()[idx]; }

  LazyArray& ShallowAssignFrom( const LazyArray& a )
  {
    if( &a == this )
      return *this;
    mShared = true;
    a.mShared = true;
    mMemory = a.mMemory;
    return *this;
  }
  LazyArray& DeepAssignFrom( const LazyArray& a )
  {
    if( a.mMemory )
    {
      if( !mMemory || mShared && mMemory.IsShared() )
        mMemory = MemoryObj( new DefaultMemory( a.Count() ) );
      mMemory->CopyFrom( a.mMemory->Ptr() );
    }
    else
      mMemory = MemoryObj( 0 );
    mShared = false;
    return *this;
  }

 private:
  void OnWrite()
  {
    if( mShared && mMemory.IsShared() && mMemory )
      mMemory = MemoryObj( new DefaultMemory( mMemory->Ptr(), Count() ) );
    mShared = false;
  }

 private:
  MemoryObj mMemory;
  mutable bool mShared;
};

} // namespace

using Tiny::LazyArray;

#endif // TINY_LAZY_ARRAY_H
