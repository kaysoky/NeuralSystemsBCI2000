//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for mutex objects.
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
#ifndef TINY_MUTEX_H
#define TINY_MUTEX_H

#ifdef _WIN32
# include <windows.h>
#else
# include <pthread.h>
#endif // _WIN32

#include "Uncopyable.h"

namespace Tiny
{

class Mutex : private Uncopyable
{
 public:
  Mutex();
  virtual ~Mutex();

  bool Acquire() const;
  bool Release() const;

  class Lock : private Uncopyable
  { // A RAAI object that locks a mutex.
   public:
    Lock( const Mutex& );
    Lock( const Mutex* );
    ~Lock();

   private:
    void DoAcquire();
    const Mutex* mpMutex;
  };

  class Unlock : private Uncopyable
  { // A RAAI object that unlocks a mutex.
   public:
    Unlock( const Mutex& );
    Unlock( const Mutex* );
    ~Unlock();

   private:
    void DoRelease();
    const Mutex* mpMutex;
  };

 private:
#ifdef _WIN32
  HANDLE mHandle;
#else // _WIN32
  mutable pthread_mutex_t mMutex;
#endif // _WIN32
};

} // namespace

using Tiny::Mutex;

#endif // OS_THREAD_H
