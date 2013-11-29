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

#include "MessageChannel.h"

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
#include "Lockable.h"

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
  iostream sDummy( 0 );
}

namespace bci
{

MessageChannel::MessageChannel( iostream& ios )
: mrInput( ios ),
  mrOutput( ios )
{
  Init();
}

MessageChannel::MessageChannel( istream& is )
: mrInput( is ),
  mrOutput( sDummy )
{
  Init();
}

MessageChannel::MessageChannel( ostream& os )
: mrInput( sDummy ),
  mrOutput( os )
{
  Init();
}

MessageChannel::MessageChannel( istream& is, ostream& os )
: mrInput( is ),
  mrOutput( os )
{
  Init();
}

void
MessageChannel::Init()
{
  mpInputLock = 0;
  mpOutputLock = 0;
  ResetStatistics();
}

MessageChannel::~MessageChannel()
{
}

#define CONSIDER(x)                         \
  case Header<x>::descSupp:                 \
    pType = #x;                             \
    On##x( is ) || is.ignore( length ); \
    break;

// Main message handling functions.
void
MessageChannel::HandleMessage()
{
  Lock _(mpInputLock);
  istream& is = Input();

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
  if( length == 0 )
    diff = -1;
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

bool
MessageChannel::OnProtocolVersion( istream& is )
{
  mProtocol.ReadBinary( is );
  if( mProtocol.Provides( ProtocolVersion::Negotiation ) )
    Send( ProtocolVersion::Current() );
  return is;
}

// Functions that construct messages.

// Generic implementation.
template<class T>
bool
MessageChannel::Send( const T& t )
{
  if( !OnSend( t ) )
    return false;

  bool omitLength = mProtocol.Provides( ProtocolVersion::ZeroMessageLengthFields );

  Lock _(mpOutputLock);
  ostream& os = Output();
  streamoff start = os.tellp();
  os.put( static_cast<unsigned char>( Header<T>::descSupp >> 8 ) );
  os.put( static_cast<unsigned char>( Header<T>::descSupp & 0xff ) );
  if( omitLength )
  {
    LengthField<2> length = 0;
    length.WriteBinary( os );
    t.WriteBinary( os );
  }
  else
  {
    ostringstream buffer;
    t.WriteBinary( buffer );
    buffer.flush();
    string str = buffer.str();
    LengthField<2> length = str.length();
    length.WriteBinary( os );
    os.write( str.data(), length );
  }
  if( os.flush() )
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
MessageChannel::ResetStatistics()
{
  mMessagesSent = 0;
  mMessagesReceived = 0;
  mBytesSent = 0;
  mBytesReceived = 0;
}

// Specializations.
template<>
bool
MessageChannel::Send( const GenericSignal& signal )
{
  return Send( VisSignalConst( signal ) );
}

template<>
bool
MessageChannel::Send( const SignalProperties& signalProperties )
{
  return Send( VisSignalProperties( signalProperties ) );
}

template<>
bool
MessageChannel::Send( const BitmapImage& bitmap )
{
  return Send( VisBitmap( bitmap ) );
}

template<>
bool
MessageChannel::Send( const ParamList& parameters )
{
  for( int i = 0; i < parameters.Size(); ++i )
    Send( parameters[ i ] );
  return Output();
}

template<>
bool
MessageChannel::Send( const StateList& states )
{
  for( int i = 0; i < states.Size(); ++i )
    Send( states[ i ] );
  return Output();
}

// Enforce instantiation of all message construction functions here,
// i.e. in this compilation unit.
template bool MessageChannel::Send( const ProtocolVersion& );
template bool MessageChannel::Send( const Status& );
template bool MessageChannel::Send( const SysCommand& );
template bool MessageChannel::Send( const State& );
template bool MessageChannel::Send( const StateVector& );
template bool MessageChannel::Send( const VisSignal& );
template bool MessageChannel::Send( const VisMemo& );
template bool MessageChannel::Send( const VisCfg& );
template bool MessageChannel::Send( const VisSignalProperties& );

} // namespace bci

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

} // namespace


