//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A lightweight lock that saves resources in situations
//   where locks are only held for a short time.
//   When used in unsuited situations, high CPU usage will occur.
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
#ifndef SPIN_LOCK_H
#define SPIN_LOCK_H

#include "Synchronized.h"
#include "ThreadUtils.h"
#include "Uncopyable.h"
#include "BCIException.h"

namespace bci
{

class SpinLock : Uncopyable
{
 public:
  SpinLock()
  : mLocked( 0 )
  {}
  ~SpinLock()
  {
    if( mLocked )
      throw std_logic_error( "Deleting a lock while it is being held" );
  }
  bool Acquire() const
  {
    if( mLocked && ThreadUtils::ThreadID() == mHolder )
      return ++mLocked;
    while( mLocked ) {};
    mHolder = ThreadUtils::ThreadID();
    mLocked = 1;
    return true;
  }
  bool Release() const
  {
    if( mLocked && ThreadUtils::ThreadID() == mHolder )
    {
      --mLocked;
      return true;
    }
    return false;
  }

  class Lock
  {
   public:
    Lock( const SpinLock& s ) : mrLock( s ) { mrLock.Acquire(); }
    ~Lock() { mrLock.Release(); }
   private:
    const SpinLock& mrLock;
  };

 private:
  mutable Synchronized<int> mLocked;
  mutable ThreadUtils::ThreadID mHolder;
};

} // namespace

using bci::SpinLock;

#endif // SPIN_LOCK_H
