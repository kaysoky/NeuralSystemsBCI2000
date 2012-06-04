//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A template for thread-local storage.
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
#ifndef OS_THREAD_LOCAL_H
#define OS_THREAD_LOCAL_H

#include "Uncopyable.h"

#if _WIN32
# include <Windows.h>
#else
# include <pthread.h>
#endif // _WIN32

template<typename T>
class OSThreadLocal : public Uncopyable
{
 public:
  OSThreadLocal();
  ~OSThreadLocal();
  T& operator=( const T& t ) { return *Data() = t; }
  operator T&() { return *Data(); }
  operator const T&() const { return *Data(); }

 private:
  T* Data() const;
#if _WIN32
  DWORD mIndex;
#else
  pthread_key_t mKey;
#endif // _WIN32
};

#if _WIN32

template<typename T>
OSThreadLocal<T>::OSThreadLocal()
: mIndex( ::TlsAlloc() )
{
  ::TlsSetValue( mIndex, new T );
}

template<typename T>
OSThreadLocal<T>::~OSThreadLocal()
{
  delete Data();
  ::TlsFree( mIndex );
}

template<typename T>
T*
OSThreadLocal<T>::Data() const
{
  return reinterpret_cast<T*>( ::TlsGetValue( mIndex ) );
}

#else // _WIN32

template<typename T>
OSThreadLocal<T>::OSThreadLocal()
{
  pthread_key_create( &mKey, NULL );
  pthread_setspecific( mKey, new T );
}

template<typename T>
OSThreadLocal<T>::~OSThreadLocal()
{
  delete Data();
  pthread_key_delete( mKey );
}

template<typename T>
T*
OSThreadLocal<T>::Data() const
{
  return reinterpret_cast<T*>( pthread_getspecific( mKey ) );
}


#endif // _WIN32

#endif // OS_THREAD_LOCAL_H
