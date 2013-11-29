//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A queue that implements synchronization between
//   a single consumer thread, and one or more producer threads.
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
#ifndef TINY_SYNCHRONIZED_QUEUE_H
#define TINY_SYNCHRONIZED_QUEUE_H

#include "Synchronized.h"
#include "Lockable.h"
#include "ThreadUtils.h"
#include "Waitable.h"

namespace Tiny
{

template<class T> class SynchronizedQueue : Uncopyable
{ // Multiple producers, single consumer
 public:
  SynchronizedQueue();
  ~SynchronizedQueue();

  bool Empty() const;
  void Clear();

  void Produce( const T& t )
    { Push( t ); }
 private:
  struct Ref_;
 public:
  Ref_ Consume();
  Ref_ AwaitConsumption( int timeout = -1 );
  void WakeConsumer();

 private:
  struct Element
  { Element(const T& t) : data(t), pNext(0) {}
    Element* pNext;
    T data;
  };
  void Push( const T& );
  Element* Pop();

  Element* mpHead, *mpTail;
  Lockable<> mHeadAccess, mTailAccess;
  Waitable mEvent;
#if BCIDEBUG
  ThreadUtils::ThreadID mConsumer;
#endif

  struct Ref_
  {
    Ref_( SynchronizedQueue& q ) : q( q ) {}
    SynchronizedQueue& q;
  };

 public:
  friend class Consumable;
  class Consumable : Uncopyable
  {
   public:
    Consumable()
      : mp( 0 ) {}
    Consumable( const Ref_& r )
      : mp( r.q.Pop() ) {}
    Consumable& operator=( const Ref_& r )
      { delete mp; mp = r.q.Pop(); return *this; }
    ~Consumable()
      { delete mp; }
    T* operator->()
      { return Data(); }
    const T* operator->() const
      { return Data(); }
    operator T*()
      { return Data(); }
    operator const T*() const
      { return Data(); }
   private:
    T* Data() const
      { return mp ? &mp->data : 0; }
    typename SynchronizedQueue<T>::Element* mp;
  };
};

// SynchronizedQueue Implementation
template<class T>
SynchronizedQueue<T>::SynchronizedQueue()
: mpHead( 0 ), mpTail( 0 )
#if BCIDEBUG
, mConsumer( false )
#endif
{
  Clear();
}

template<class T>
SynchronizedQueue<T>::~SynchronizedQueue()
{
  Clear();
}

template<class T> void
SynchronizedQueue<T>::Clear()
{
#if BCIDEBUG
  mConsumer = ThreadUtils::ThreadID( false );
#endif
  Lock _(mHeadAccess);
  Lock __(mTailAccess);
  mEvent.Reset();
  Element* p = 0;
  while( p = mpHead )
  {
    mpHead = mpHead->pNext;
    delete p;
  }
  mpTail = 0;
}

template<class T> void
SynchronizedQueue<T>::Push( const T& t )
{
  Lock _(mTailAccess);
  Element* p = mpTail;
  mpTail = new Element( t );
  if( p )
    p->pNext = mpTail;
  else
  {
    Lock _(mHeadAccess);
    mpHead = mpTail;
  }
  WakeConsumer();
}

template<class T> typename SynchronizedQueue<T>::Element*
SynchronizedQueue<T>::Pop()
{
#if BCIDEBUG
  if( ThreadUtils::ThreadID() != mConsumer )
    throw std_logic_error( "Multiple consumer threads detected" );
  mConsumer = ThreadUtils::ThreadID();
#endif
  Lock _(mHeadAccess);
  Element* p = mpHead;
  if( p )
  {
    mpHead = p->pNext;
    if( mpHead == 0 )
    {
      Lock _(mTailAccess);
      mEvent.Reset();
      mpTail = 0;
    }
  }
  return p;
}

template<class T> bool
SynchronizedQueue<T>::Empty() const
{
  Tiny::MemoryFence();
  return mpHead == 0;
}

template<class T> typename SynchronizedQueue<T>::Ref_
SynchronizedQueue<T>::Consume()
{
  return *this;
}

template<class T> typename SynchronizedQueue<T>::Ref_
SynchronizedQueue<T>::AwaitConsumption( int timeout )
{
  mEvent.Wait( timeout );
  return Consume();
}

template<class T> void
SynchronizedQueue<T>::WakeConsumer()
{
  mEvent.Set();
}

} // namespace

using Tiny::SynchronizedQueue;

#endif // TINY_SYNCHRONIZED_QUEUE_H
