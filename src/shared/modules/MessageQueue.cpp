////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A queue for BCI2000 messages.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "MessageQueue.h"
#include "LengthField.h"
#include "Lockable.h"

using namespace std;

void
MessageQueue::QueueMessage( std::istream& is )
{
  MessageQueueEntry entry = { 0, 0, NULL };
  entry.descSupp = is.get() << 8;
  entry.descSupp |= is.get();
  LengthField<2> length;
  length.ReadBinary( is );
  entry.length = length;
  if( is )
  {
    entry.message = new char[ length ];
    if( length > 0 )
    {
      is.read( entry.message, length - 1 );
      entry.message[ length - 1 ] = is.get();
    }
  }
  if( is )
  {
    ::Lock<MessageQueue> lock( *this );
    this->push( entry );
  }
}

bool
MessageQueue::Empty() const
{
  ::Lock<const MessageQueue> lock( *this );
  return this->empty();
}

MessageQueueEntry
MessageQueue::Next()
{
  ::Lock<MessageQueue> lock( *this );
  MessageQueueEntry result = this->front();
  this->pop();
  return result;
}

