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
#ifndef SYNCHRONIZED_QUEUE_H
#define SYNCHRONIZED_QUEUE_H

#include "Synchronized.h"
#include "Lockable.h"
#include "ThreadUtils.h"
#include "OSEvent.h"

namespace bci
{

template<class T> class SynchronizedQueue : Uncopyable
{ // Multiple producers, single consumer
 public:
  SynchronizedQueue();
  ~SynchronizedQueue();

  void Produce( const T& );
  void Consume();
  T& Consumable()
    { return Front(); }
  bool AwaitConsumption( int timeout = -1 ) const;
  void WakeConsumer();
  bool Empty() const;
  void Clear();

  void push( const T& t )
    { Produce( t ); }
  void pop()
    { Consume(); }
  T& front()
    { return Front(); }
  const T& front() const
    { return Front(); }
  bool empty() const
    { return Empty(); }
  void clear()
    { Clear(); }

 private:
  T& Front() const;
  struct Element
  { Element(const T& t) : data(t), pNext(0) {}
    Element* pNext;
    T data;
  } *mpHead, *mpTail;
  Lockable<> mHeadAccess, mTailAccess;
  OSEvent mEvent;
#if BCIDEBUG
  ThreadUtils::ThreadID mConsumer;
#endif
};

// SynchronizedQueue Implementation
template<class T>
SynchronizedQueue<T>::SynchronizedQueue()
: mpHead( 0 ), mpTail( 0 )
#if BCIDEBUG
, mConsumer( false )
#endif
{
  mEvent.Reset();
}

template<class T>
SynchronizedQueue<T>::~SynchronizedQueue()
{
  Clear();
}

template<class T> void
SynchronizedQueue<T>::Clear()
{
  while( !Empty() )
    Consume();
}

template<class T> void
SynchronizedQueue<T>::Produce( const T& t )
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

template<class T> void
SynchronizedQueue<T>::Consume()
{
#if BCIDEBUG
  if( ThreadUtils::ThreadID() != mConsumer )
    bcidebug( "Multiple consumer threads detected" );
  mConsumer = ThreadUtils::ThreadID();
#endif
  Element* p = 0;
  {
    Lock _(mHeadAccess);
    bciassert( mpHead );
    {
      Lock _(mTailAccess);
      if( mpHead == mpTail )
      {
        mpTail = 0;
        mEvent.Reset();
      }
    }
    p = mpHead;
    mpHead = mpHead->pNext;
  }
  delete p;
}

template<class T> T&
SynchronizedQueue<T>::Front() const
{
  Lock _(mHeadAccess);
  bciassert( mpHead );
  return const_cast<Element*>( mpHead )->data;
}

template<class T> bool
SynchronizedQueue<T>::Empty() const
{
  MemoryBarrier();
  return mpHead == 0;
}

template<class T> bool
SynchronizedQueue<T>::AwaitConsumption( int timeout ) const
{
  mEvent.Wait( timeout );
  return !Empty();
}

template<class T> void
SynchronizedQueue<T>::WakeConsumer()
{
  mEvent.Set();
}

} // namespace bci

using bci::SynchronizedQueue;

#endif // SYNCHRONIZED_QUEUE_H
