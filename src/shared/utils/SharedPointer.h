////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A simple smart pointer class that deletes the owned object
//   when the last pointer's destructor is called.
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
#ifndef SHARED_POINTER_H
#define SHARED_POINTER_H

#include "Synchronized.h"

template<class T>
struct ArrayAllocator
{
  static T* New( size_t n ) { return new T[n]; }
  static void Delete( T* t ) { delete[] t; }
};

template<class T>
struct InstanceAllocator
{
  static void Delete( T* t ) { delete t; }
};

template<typename T>void DeleteInstance( T* t ) { delete t; }

template<typename T, class A = InstanceAllocator<T> >
class SharedPointer
{
  class Owner;

 public:
  explicit SharedPointer( T* p = 0 ) : mpOwner( new Owner( p ) ) { mpOwner->Inc(); }
  SharedPointer( const SharedPointer& s ) : mpOwner( s.mpOwner ) { mpOwner->Inc(); }
  SharedPointer& operator=( const SharedPointer& s ) { mpOwner->Dec(); mpOwner = s.mpOwner; mpOwner->Inc(); return *this; }
  ~SharedPointer() { mpOwner->Dec(); }

  const T* operator->() const { return mpOwner->Object(); }
  T* operator->() { return mpOwner->Object(); }
  operator bool() const { return operator->(); }

  bool IsShared() const { return mpOwner->Count() > 1; }

 private:
  Owner* mpOwner;

 private:
  class Owner
  {
   public:
    Owner( T* pObject ) : mpObject( pObject ), mCount( 0 ) {}
    ~Owner() { A::Delete( mpObject ); }
    T* Object() const { return mpObject; }
    int Count() const { return mCount; }
    void Inc() { ++mCount; }
    void Dec() { if( --mCount == 0 ) delete this; }
   private:
    Synchronized<T*> mpObject;
    Synchronized<int> mCount;
  };

};


#endif // SHARED_POINTER_H
