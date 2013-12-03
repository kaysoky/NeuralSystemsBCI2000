//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A template for "safe" static objects.
//   * Access is done through an accessor function, ensuring proper order
//     of initialization.
//   * Object creation is thread-safe.
//   * The object will exist at the beginning of main().
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
#ifndef TINY_STATIC_OBJECT_H
#define TINY_STATIC_OBJECT_H

#include "Uncopyable.h"
#include "Synchronized.h"
#include "Debugging.h"

namespace Tiny
{

template<class T, class U = T> class StaticObject : Uncopyable
{
 public:
  StaticObject() { Get(); }
  ~StaticObject() { Get()->~T(); mpInstance = 0; mpThis = Invalid(); }
  bool Destructed() const { return mpThis == Invalid(); }
  U& operator()() { return *Get(); }
  const U& operator()() const { return *Get(); }
  U* operator->() { return Get(); }
  const U* operator->() const { return Get(); }
  operator U*() { return Get(); }
  operator const U*() const { return Get(); }

 private:
  T* Get() const
  { return const_cast<StaticObject*>( this )->Get(); }
  T* Get()
  {
    if( Destructed() )
      throw std_logic_error( "StaticObject<> instance used after being destructed" );
#if TINY_DEBUG
    if( ( mpThis || mpInstance ) && mpThis != this )
      throw std_logic_error( "StaticObject<> instance appears to be on stack or in heap -- declare as \"static\" to fix" );
#endif
    if( !Atomic_::Pointer<StaticObject>( mpThis ).Exchange( this ) )
      mpInstance = new (mMemory) T;
    while( !mpInstance )
      MemoryFence();
    return mpInstance;
  }
  static StaticObject* Invalid() { return reinterpret_cast<StaticObject*>( -1 ); }
  StaticObject* mpThis;
  T* mpInstance;
  char mMemory[sizeof(T)];
};

template<typename T, T (*F)()> class StaticBuffer
{
 private:
  struct Buffer : T
  { Buffer() : T( F() ) {} };
 public:
  operator T() const
  { static StaticObject<Buffer, T> buffer; return buffer.Destructed() ? F() : buffer(); }
  T operator()() const
  { return *this; }
};

} // namespace

using Tiny::StaticObject;
using Tiny::StaticBuffer;

#endif // TINY_STATIC_OBJECT_H