////////////////////////////////////////////////////////////////////////////////
//
// File: MessageHandler.cpp
//
// Description: Facilities for centralized management of BCI2000 messages.
//
// Author: Juergen Mellinger
//
// Date:   Jul 24, 2003
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "MessageHandler.h"

#include "UStatus.h"
#include "UParameter.h"
#include "UState.h"
#include "UGenericSignal.h"
#include "USysCommand.h"
#include "UGenericVisualization.h"
#include "LengthField.h"

#ifndef BCI_TOOL // Some old modules out there send improperly terminated
                 // parameter messages. Using an additional buffer for
                 // received messages will help.
# include <sstream>
#endif // BCI_TOOL

using namespace std;

// A macro that contains the structure of a single case statement in the
// main message handling function.
#define CONSIDER(x)                                                      \
  case Header<x>::descSupp:                                              \
    Handle##x( is ) || is.ignore( length );                              \
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
#ifndef BCI_TOOL
    char* messageBuffer = new char[ length ];
    if( length > 0 )
    {
      is.read( messageBuffer, length - 1 );
      messageBuffer[ length - 1 ] = is.get();
      if( !is )
        return;
    }
    istringstream is( string( messageBuffer, length ) );
    delete[] messageBuffer;
#endif // BCI_TOOL
    switch( descSupp )
    {
      CONSIDER( STATEVECTOR );
      CONSIDER( VisSignal );
      CONSIDER( STATUS );
      CONSIDER( PARAM );
      CONSIDER( STATE );
      CONSIDER( SYSCMD );
      CONSIDER( VisMemo );
      CONSIDER( VisCfg );
      default:
        bcierr << "Unknown message descriptor/supplement" << endl;
    }
  }
}

// Functions that construct messages.

// The generic implementation.
template<typename content_type>
ostream&
MessageHandler::PutMessage( ostream& os, const content_type& content )
{
  os.put( Header<content_type>::descSupp >> 8 );
  os.put( Header<content_type>::descSupp & 0xff );
#ifdef BCI_TOOL // The version without buffering.
  os.put( 0 ).put( 0 );
  content.WriteBinary( os );
#else // We need the buffer to correctly fill in the length field.
  static ostringstream buffer;
  buffer.str( "" );
  buffer.clear();
  content.WriteBinary( buffer );
  LengthField<2> length = buffer.str().length();
  length.WriteBinary( os );
  os.write( buffer.str().data(), length );
#endif
  return os;
}

// Specializations. The protocol could be re-defined to remove them.
template<>
ostream&
MessageHandler::PutMessage( ostream& os, const GenericSignal& signal )
{
  return PutMessage( os, VisSignal( signal ) );
}

template<>
ostream&
MessageHandler::PutMessage( ostream& os, const PARAMLIST& parameters )
{
  for( PARAMLIST::const_iterator i = parameters.begin(); i != parameters.end(); ++i )
    PutMessage( os, i->second );
  return PutMessage( os, SYSCMD::EndOfParameter );
}

template<>
ostream&
MessageHandler::PutMessage( ostream& os, const STATELIST& states )
{
  for( int i = 0; i < states.GetNumStates(); ++i )
    PutMessage( os, *states.GetStatePtr( i ) );
  return PutMessage( os, SYSCMD::EndOfState );
}

// Enforce instantiation of all message construction functions here,
// i.e. in this compilation unit.
// Only those not used in one of the functions above need explicit instantiation.
template ostream& MessageHandler::PutMessage( std::ostream&, const PARAMLIST& );
template ostream& MessageHandler::PutMessage( std::ostream&, const STATELIST& );
template ostream& MessageHandler::PutMessage( std::ostream&, const STATEVECTOR& );
template ostream& MessageHandler::PutMessage( std::ostream&, const STATUS& );
template ostream& MessageHandler::PutMessage( std::ostream&, const GenericSignal& );
template ostream& MessageHandler::PutMessage( std::ostream&, const VisCfg& );
template ostream& MessageHandler::PutMessage( std::ostream&, const VisMemo& );


