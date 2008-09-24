////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Sending and dispatching of BCI2000 messages.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
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

using namespace std;

// A macro that contains the structure of a single case statement in the
// main message handling function.
#define CONSIDER(x)                                                      \
  case Header<x>::descSupp:                                              \
    Handle##x( iss );                                                    \
    break;

// The main message handling function.
void
MessageHandler::HandleMessage( istream& is )
{
  int descSupp = is.get() << 8;
  descSupp |= is.get();
  LengthField<2> length;
  length.ReadBinary( is );
  if( is )
  {
    char* messageBuffer = new char[ length ];
    if( length > 0 )
    {
      is.read( messageBuffer, length - 1 );
      messageBuffer[ length - 1 ] = is.get();
    }
    if( is )
    {
      istringstream iss( string( messageBuffer, length ) );
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
          bcierr << "Unknown message descriptor/supplement" << endl;
      }
      is.setstate( iss.rdstate() & ~ios::eofbit );
    }
    delete[] messageBuffer;
  }
}

// Functions that construct messages.

// Generic implementation.
template<typename content_type>
ostream&
MessageHandler::PutMessage( ostream& os, const content_type& content )
{
  os.put( Header<content_type>::descSupp >> 8 );
  os.put( Header<content_type>::descSupp & 0xff );
  static ostringstream buffer;
  buffer.str( "" );
  buffer.clear();
  content.WriteBinary( buffer );
  LengthField<2> length = buffer.str().length();
  length.WriteBinary( os );
  os.write( buffer.str().data(), length );
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

