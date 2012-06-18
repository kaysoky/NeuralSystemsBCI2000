////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Sending and dispatching of BCI2000 messages.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "MessageHandler.h"

#include "ProtocolVersion.h"
#include "Status.h"
#include "Param.h"
#include "ParamList.h"
#include "State.h"
#include "StateList.h"
#include "StateVector.h"
#include "GenericSignal.h"
#include "SysCommand.h"
#include "GenericVisualization.h"
#include "LengthField.h"
#include "BCIError.h"

#include <sstream>
#include <iomanip>
#include <cstdio>

using namespace std;

// A macro that contains the structure of a single case statement in the
// main message handling function.
#define CONSIDER(x)                                                      \
  case Header<x>::descSupp:                                              \
    Handle##x( iss );                                                    \
    CheckForError( iss, #x );                                            \
    break;

// Main message handling functions.
void
MessageHandler::HandleMessage( MessageQueue& ioQueue )
{
  if( ioQueue.Empty() )
    return;

  MessageQueueEntry entry = ioQueue.Next();

  istringstream iss( string( entry.message, entry.length ) );
  delete[] entry.message;
  switch( entry.descSupp )
  {
    CONSIDER( ProtocolVersion );
    CONSIDER( StateVector );
    CONSIDER( VisSignal );
    CONSIDER( Status );
    CONSIDER( Param );
    CONSIDER( State );
    CONSIDER( SysCommand );
    CONSIDER( VisMemo );
    CONSIDER( VisSignalProperties );
    CONSIDER( VisBitmap );
    CONSIDER( VisCfg );
    default:
      bcierr_ << ": Unknown message descriptor/supplement 0x"
             << hex << entry.descSupp << endl;
  }
}

void
MessageHandler::CheckForError( istream& inStream, const char* inMessageType )
{
  if( inStream.fail() || inStream.peek() != EOF )
    bcierr_ << ": Error reading " << inMessageType << " message" << endl;
}

void
MessageHandler::HandleMessage( istream& is )
{
  MessageQueue queue;
  queue.QueueMessage( is );
  HandleMessage( queue );
}

// Functions that construct messages.

// Generic implementation.
template<typename content_type>
ostream&
MessageHandler::PutMessage( ostream& os, const content_type& content )
{
  os.put( char( Header<content_type>::descSupp >> 8 ) ); // the old-style cast will silence MSVC's irrelevant C4309 warning
  os.put( char( Header<content_type>::descSupp & 0xff ) );
  ostringstream buffer;
  content.WriteBinary( buffer );
  string str = buffer.str();
  LengthField<2> length = str.length();
  length.WriteBinary( os );
  os.write( str.data(), length );
  return os;
}

// Specializations.
template<>
ostream&
MessageHandler::PutMessage( ostream& os, const GenericSignal& signal )
{
  return PutMessage( os, VisSignal( signal ) );
}

template<>
ostream&
MessageHandler::PutMessage( ostream& os, const SignalProperties& signalProperties )
{
  return PutMessage( os, VisSignalProperties( signalProperties ) );
}

template<>
ostream&
MessageHandler::PutMessage( ostream& os, const BitmapImage& bitmap )
{
  return PutMessage( os, VisBitmap( bitmap ) );
}

template<>
ostream&
MessageHandler::PutMessage( ostream& os, const ParamList& parameters )
{
  for( int i = 0; i < parameters.Size(); ++i )
    PutMessage( os, parameters[ i ] );
  return os;
}

template<>
ostream&
MessageHandler::PutMessage( ostream& os, const StateList& states )
{
  for( int i = 0; i < states.Size(); ++i )
    PutMessage( os, states[ i ] );
  return os;
}

// Enforce instantiation of all message construction functions here,
// i.e. in this compilation unit.
template ostream& MessageHandler::PutMessage( std::ostream&, const ProtocolVersion& );
template ostream& MessageHandler::PutMessage( std::ostream&, const StateVector& );
template ostream& MessageHandler::PutMessage( std::ostream&, const Status& );
template ostream& MessageHandler::PutMessage( std::ostream&, const SysCommand& );
template ostream& MessageHandler::PutMessage( std::ostream&, const VisMemo& );
template ostream& MessageHandler::PutMessage( std::ostream&, const VisCfg& );

