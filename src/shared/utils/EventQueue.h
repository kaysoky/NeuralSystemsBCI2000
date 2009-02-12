//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A thread-safe event queue, implemented as a linked
//   list.
//   An event is defined by a string description, and a time stamp.
//   Any thread may insert/remove events at any time.
//   We assume new/delete to be thread-safe for structs without
//   constructors.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "PrecisionTime.h"
#include <cstddef>

class EventQueue
{
 public:
  EventQueue()
    : mpFront( NULL ),
      mpBack( NULL ),
      mLocked( false )
    {}
  ~EventQueue()
    { Clear(); }
  bool IsEmpty()
    { return mpFront == NULL; }
  void Clear()
    { while( !IsEmpty() ) PopFront(); }
  void PushBack( const char* inDescriptor, PrecisionTime );
  void PopFront();
  const char* FrontDescriptor() const
    { return mpFront->mpDescriptor; }
  PrecisionTime FrontTimeStamp() const
    { return mpFront->mTimeStamp; }

 private:
  void Lock()
    { while( mLocked ) {}; mLocked = true; }
  void Unlock()
    { mLocked = false; }

  bool mLocked;

  struct Entry
  {
    char*         mpDescriptor;
    PrecisionTime mTimeStamp;
    Entry*        mpNext;
  };

  Entry* mpFront,
       * mpBack;
};

#endif // EVENT_QUEUE_H
