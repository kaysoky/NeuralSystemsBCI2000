//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A template for variables to be synchronized between
//   threads. Only supports int32_t, bool, and pointer types.
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
#ifndef SYNCHRONIZED_H
#define SYNCHRONIZED_H

#include "Atomic.h"

namespace bci
{

template<class T = int32_t> class Synchronized
{
 public:
  Synchronized( T t = 0 ) : mValue( 0 ) { (*this) = t; }
  operator T() const { MemoryFence(); return mValue; }
  int32_t operator=( T t ) { mValue = t; MemoryFence(); return t; }
  int32_t operator++() { return Atomic().operator++(); }
  int32_t operator++(int) { return Atomic().operator++(0); }
  int32_t operator--() { return Atomic().operator--(); }
  int32_t operator--(int) { return Atomic().operator--(0); }
  int32_t operator+=( T t ) { return Atomic() += t; }
  int32_t operator-=( T t ) { return Atomic() -= t; }
  int32_t operator&=( T t ) { return Atomic() &= t; }
  int32_t operator|=( T t ) { return Atomic() |= t; }
  Atomic_::Value Atomic() { MemoryFence(); return Atomic_::Value( mValue ); }
 private:
  volatile T mValue;
};

template<> class Synchronized<bool>
{
 public:
  Synchronized( bool b = false ) : mValue( b ) {}
  operator bool() const { return mValue != 0; }
  bool operator=( bool b ) { return mValue = ( b ? 1 : 0 ); }
  bool operator&=( bool b ) { return mValue &= ( b ? 1 : 0 ); }
  bool operator|=( bool b ) { return mValue |= ( b ? 1 : 0 ); }
 private:
  Synchronized<> mValue;
};

template<class T> class Synchronized<T*>
{
 public:
  Synchronized( T* t = 0 ) : mPointer( 0 ) { (*this) = t; }
  T* operator=( T* t ) { return Set( t ); }
  operator T*() const { return Get(); }
  T* operator->() { return Get(); }
  const T* operator->() const { return Get(); }
 private:
  T* Get() const { MemoryFence(); return const_cast<T*>( mPointer ); }
  T* Set( T* t ) { mPointer = t; MemoryFence(); return t; }
  T* volatile mPointer;
};

} // namespace bci

using bci::Synchronized;

#endif // SYNCHRONIZED_H
