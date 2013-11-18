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
#include "SockStream.h"
#include "BCIException.h"

#include <sstream>
#include <iomanip>
#include <cstdio>

#ifndef STREAM_DEBUG
# if BCIDEBUG
#  define STREAM_DEBUG 1
# endif
#endif

using namespace std;

namespace
{
  void SaveDebugInfo( istream& );
  string GetDebugHistory( istream& );
}

// A macro that contains the structure of a single case statement in the
// main message handling function.
#define CONSIDER(x)                         \
  case Header<x>::descSupp:                 \
    pType = #x;                             \
    Handle##x( is ) || is.ignore( length ); \
    break;

// Main message handling functions.
void
MessageHandler::HandleMessage( istream& is )
{
  SaveDebugInfo( is );

  streamoff start = is.tellg();
  int descSupp = is.get() << 8;
  descSupp |= is.get();
  LengthField<2> length;
  length.ReadBinary( is );
  streamoff msgStart = is.tellg();
  const char* pType = 0;
  switch( descSupp )
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
      ;
  }

  SaveDebugInfo( is );
  streamoff end = is.tellg();
  if( is.fail() )
  {
    is.clear();
    end = is.tellg();
    is.setstate( ios::failbit );
  }
  streamoff diff = ( msgStart < 0 || end < 0 ) ? -1 : end - msgStart;
  if( is.fail() || !pType || diff >= 0 && diff != length )
    throw std_runtime_error(
      "Error reading message of type 0x" << hex << descSupp
       << " (" << ( pType ? pType : "unknown" ) << "), "
       << "nominal length: " << dec << length
       << ", read: " << diff
       << GetDebugHistory( is )
    );

  ++mMessagesReceived;
  if( end >= 0 && start >= 0 && mBytesReceived >= 0 )
    mBytesReceived += ( end - start );
  else
    mBytesReceived = -1;
}

// Functions that construct messages.

// Generic implementation.
template<typename content_type>
ostream&
MessageHandler::PutMessage( ostream& os, const content_type& content )
{
  os.put( static_cast<unsigned char>( Header<content_type>::descSupp >> 8 ) );
  os.put( static_cast<unsigned char>( Header<content_type>::descSupp & 0xff ) );
  ostringstream buffer;
  content.WriteBinary( buffer );
  buffer.flush();
  string str = buffer.str();
  LengthField<2> length = str.length();
  length.WriteBinary( os );
  os.write( str.data(), length );
  return os.flush();
}

#undef SendMessage
template<typename content_type>
ostream&
MessageHandler::SendMessage( ostream& os, const content_type& content )
{
  streamoff start = os.tellp();
  if( PutMessage( os, content ) )
  {
    ++mMessagesSent;
    streamoff end = os.tellp();
    if( start >= 0 && end >= 0 && mBytesSent >= 0 )
      mBytesSent += ( end - start );
    else
      mBytesSent = -1;
  }
  return os;
}

void
MessageHandler::ResetStatistics()
{
  mMessagesSent = 0;
  mMessagesReceived = 0;
  mBytesSent = 0;
  mBytesReceived = 0;
}

// Specializations.
template<>
ostream&
MessageHandler::PutMessage( ostream& os, const GenericSignal& signal )
{
  return PutMessage( os, VisSignalConst( signal ) );
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
template ostream& MessageHandler::SendMessage( std::ostream&, const ProtocolVersion& );
template ostream& MessageHandler::SendMessage( std::ostream&, const Status& );
template ostream& MessageHandler::SendMessage( std::ostream&, const SysCommand& );
template ostream& MessageHandler::SendMessage( std::ostream&, const ParamList& );
template ostream& MessageHandler::SendMessage( std::ostream&, const State& );
template ostream& MessageHandler::SendMessage( std::ostream&, const StateList& );
template ostream& MessageHandler::SendMessage( std::ostream&, const StateVector& );
template ostream& MessageHandler::SendMessage( std::ostream&, const VisSignal& );
template ostream& MessageHandler::SendMessage( std::ostream&, const VisMemo& );
template ostream& MessageHandler::SendMessage( std::ostream&, const VisCfg& );
template ostream& MessageHandler::SendMessage( std::ostream&, const SignalProperties& );
template ostream& MessageHandler::SendMessage( std::ostream&, const VisSignalProperties& );


namespace
{
const size_t BackHistory = 4;

#if STREAM_DEBUG
typedef map< streambuf*, list<string> > DebugInfoContainer;
DebugInfoContainer& DebugInfoInstance()
{
  static DebugInfoContainer instance;
  return instance;
}
#endif

void SaveDebugInfo( istream& is )
{
#if STREAM_DEBUG
  ostringstream oss;
  oss << "stream state: ";
  if( is.good() )
    oss << "good";
  else
  {
    if( is.fail() )
      oss << "fail ";
    if( is.bad() )
      oss << "bad ";
    if( is.eof() )
      oss << "eof ";
  }
  oss << endl;

  streambuf* pS = is.rdbuf();
# if STREAM_DEBUG > 1
  sockbuf* pSock = dynamic_cast<sockbuf*>( pS );
  if( pSock )
    pSock->debug_print( oss );
# endif
  list<string>& state = DebugInfoInstance()[pS];
  state.push_front( oss.str() );
  if( state.size() > BackHistory )
    state.pop_back();
#endif
}

string GetDebugHistory( istream& is )
{
#if STREAM_DEBUG
  ostringstream oss;
  const list<string>& info = DebugInfoInstance()[is.rdbuf()];
  if( !info.empty() )
  {
    oss << "\nBuffer history:\n\n";
    for( list<string>::const_iterator i = info.begin(); i != info.end(); ++i )
      oss << *i << endl;
    oss.flush();
  }
  return oss.str();
#else
  return "";
#endif
}

}


