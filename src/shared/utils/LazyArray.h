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
#ifndef LAZY_ARRAY_H
#define LAZY_ARRAY_H

#include "SharedPointer.h"
#include "Lockable.h"
#include "BCIException.h"

template<class T>
class LazyArray
{
  struct Allocator
  {
    struct Data : Lockable<> {};
    static Data& Data( T* t )
      { return *reinterpret_cast<struct Data*>( reinterpret_cast<char*>( t ) - sizeof( struct Data ) ); }
    static T* New( size_t n )
    {
      char* raw = new char[n*sizeof( T ) + sizeof( struct Data )];
      new (raw) struct Data;
      return reinterpret_cast<T*>( raw + sizeof( struct Data ) );
    }
    static void Delete( T* t )
    {
      if( t )
      {
        Data( t ).~Data();
        char* raw = reinterpret_cast<char*>( &Data( t ) );
        delete[] raw;
      }
    }
  };
  struct Pointer : SharedPointer<T, Allocator>
  {
    Pointer( T* t ) : SharedPointer<T, Allocator>( t ) {}
    void Lock() { Allocator::Data( operator->() ).Lock(); }
    void Unlock() { Allocator::Data( operator->() ).Unlock(); }
  };

 public:
  LazyArray()
    : mSize( 0 ), mpData( 0 ), mPointer( 0 ), mShared( false )
    {}
  LazyArray( size_t size )
    : mSize( size ), mpData( Allocator::New( size ) ), mPointer( mpData ), mShared( false )
    {}
  LazyArray( T* data, size_t size )
    : mSize( size ), mpData( data ? data : Allocator::New( size ) ), mPointer( data ? 0 : mpData ), mShared( false )
    {}
  LazyArray( const LazyArray& a )
    : mSize( 0 ), mpData( 0 ), mPointer( 0 ), mShared( false )
    { AssignFrom( a ); }
  LazyArray& operator=( const LazyArray& a )
    { AssignFrom( a ); return *this; }
  size_t Size() const
    { return mSize; }
  const T& operator[]( size_t idx ) const
    { return At( idx ); }
  T& operator[]( size_t idx )
    { OnWrite(); return At( idx ); }

 private:
  T& At( size_t idx ) const
    { return const_cast<T&>( mpData[idx] ); }
  bool LazyAssignmentAllowed() const
    { return mpData == mPointer.operator->(); }
  void AssignFrom( const LazyArray& a )
    {
      if( &a == this )
        return;

      if( this->LazyAssignmentAllowed() )
      {
        mSize = a.mSize;
        mpData = a.mpData;
        mPointer = a.mPointer;
        mShared = true;
        a.mShared = true;
      }
      else
      {
        if( mSize != a.mSize )
          throw std_runtime_error( "Cannot assign from array with different size" );
        ::memcpy( mpData, a.mpData, mSize * sizeof( T ) );
        mShared = false;
      }
    }
  void OnWrite()
    {
      if( mShared )
      {
        mPointer.Lock();
        mShared = false;
        if( mPointer.IsShared() )
        {
          Pointer lockedPointer = mPointer;
          T* newData = Allocator::New( mSize );
          ::memcpy( newData, mpData, mSize * sizeof( T ) );
          mpData = newData;
          mPointer = Pointer( newData );
          lockedPointer.Unlock();
        }
        else
          mPointer.Unlock();
      }
    }

 private:
  size_t mSize;
  T* mpData;
  Pointer mPointer;
  mutable bool mShared;
};

#endif // LAZY_ARRAY_H
