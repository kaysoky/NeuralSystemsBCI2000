//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A thread-safe event queue.
//   An event is defined by a string description, and a time stamp.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "EventQueue.h"
#include "BCIException.h"
#include <cstring>

using namespace std;

void
EventQueue::PushBack( const char* inDescriptor, PrecisionTime inTimeStamp )
{
  size_t descLen = ::strlen( inDescriptor ) + 1;
  char* pDesc = new char[descLen];
  ::memcpy( pDesc, inDescriptor, descLen );
  push( make_pair( pDesc, inTimeStamp ) );

  if( !mEventsAllowed ) // Defer test to avoid loss of memory references.
    throw std_runtime_error(
      "No events allowed when receiving \"" << inDescriptor << "\" event "
      "-- trying to record events outside the \"running\" state?"
    );
}

void
EventQueue::PopFront()
{
  delete[] front().first;
  pop();
}

