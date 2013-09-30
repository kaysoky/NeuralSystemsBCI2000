//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A template that prevents concurrent construction of
//   multiple instances of a class, and thus allows automatic parent
//   identification by member objects.
//   This is a large advantage because it removes the need of
//   explicit child constructors, allowing for full child definition
//   at a single location in the parent class header.
//
//   Usage:
//     class MyClass : public CtorLock<MyClass>
//     {
//       class MyChild
//       {
//         MyChild() : mpParent( MyClass::InstanceInCtor() ) {}
//         MyClass* mpParent;
//       };
//       MyChild mChild1, mChild2, mChild3;
//       CtorUnlock mUnlock; // must be last the class declaration
//     };
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
#ifndef CTOR_LOCK_H
#define CTOR_LOCK_H

#include "OSMutex.h"

template<class T> class CtorLock
{
public:
  CtorLock()
  { Lock( static_cast<T*>( this ) ); }
  ~CtorLock()
  { if( static_cast<T*>( this ) == sInstanceInCtor ) Unlock(); }
  struct CtorUnlock
  { CtorUnlock()
    { CtorLock::Unlock(); }
  };
  static T* InstanceInCtor()
  { bciassert( sInstanceInCtor != 0 ); return sInstanceInCtor; }

private:
  static void Lock( T* t )
  { 
    sMutex.Acquire(); 
    bciassert( sInstanceInCtor == 0 );
    sInstanceInCtor = t;
  }
  static void Unlock()
  { 
    bciassert( sInstanceInCtor != 0 );
    sInstanceInCtor = 0;
    sMutex.Release();
  }

private:
  static OSMutex sMutex;
  static T* sInstanceInCtor;
};

template<class T> OSMutex CtorLock<T>::sMutex;
template<class T> T* CtorLock<T>::sInstanceInCtor;

#endif // CTOR_LOCK_H
