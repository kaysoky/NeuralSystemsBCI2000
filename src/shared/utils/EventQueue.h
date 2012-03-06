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
#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "Lockable.h"
#include "PrecisionTime.h"

class EventQueue : public Lockable
{
 public:
  EventQueue()
    : mpFront( NULL ),
      mpBack( NULL ),
      mEventsAllowed( false )
    {}
  ~EventQueue()
    { Clear(); }
  void AllowEvents()
    { ::Lock<EventQueue> lock( *this ); mEventsAllowed = true; }
  void DenyEvents()
    { ::Lock<EventQueue> lock( *this ); mEventsAllowed = false; }
  bool IsEmpty()
    { return mpFront == NULL; }
  void Clear()
    { ::Lock<EventQueue> lock( *this ); while( !IsEmpty() ) PopFront(); }
  void PushBack( const char* inDescriptor, PrecisionTime );
  void PopFront();
  const char* FrontDescriptor() const
    { return mpFront->mpDescriptor; }
  PrecisionTime FrontTimeStamp() const
    { return mpFront->mTimeStamp; }

 private:
  struct Entry
  {
    char*         mpDescriptor;
    PrecisionTime mTimeStamp;
    Entry*        mpNext;
  };

  Entry* mpFront,
       * mpBack;
  bool mEventsAllowed;
};

#endif // EVENT_QUEUE_H
