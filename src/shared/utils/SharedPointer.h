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

#include "OSMutex.h"

template<typename T>
class SharedPointer
{
  class Owner;

 public:
  explicit SharedPointer( T* p = NULL ) : mpOwner( new Owner( p ) ) { mpOwner->Inc(); }
  SharedPointer( const SharedPointer& s ) : mpOwner( s.mpOwner ) { mpOwner->Inc(); }
  SharedPointer& operator=( const SharedPointer& s ) { mpOwner->Dec(); mpOwner = s.mpOwner; mpOwner->Inc(); return *this; }
  ~SharedPointer() { mpOwner->Dec(); }

  const T* operator->() const { return mpOwner->Object(); }
  T* operator->() { return mpOwner->Object(); }

 private:
  Owner* mpOwner;

 private:
  class Owner
  {
   public:
    Owner( T* pObject )
    : mpObject( pObject ), mCount( 0 ) {}
    ~Owner() { delete mpObject; }
    T* Object() { return mpObject; }
    void Inc()
    {
      OSMutex::Lock lock( mMutex );
      ++mCount;
    }
    void Dec()
    {
      bool doDelete = false;
      {
        OSMutex::Lock lock( mMutex );
        doDelete = ( --mCount == 0 );
      }
      if( doDelete )
        delete this;
     }
   private:
    T* mpObject;
    int mCount;
    OSMutex mMutex;
  };

};


#endif // SHARED_POINTER_H
