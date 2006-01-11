////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: MessageHandler.cpp
//
// Author: Juergen Mellinger
//
// Date:   Jul 24, 2003
//
// $Log$
// Revision 1.11  2006/01/11 19:08:44  mellinger
// Adaptation to latest revision of parameter and state related class interfaces.
//
// Revision 1.10  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
//
// Description: Facilities for centralized management of BCI2000 messages.
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
      if( !is )
        return;
    }
    istringstream iss( string( messageBuffer, length ) );
    delete[] messageBuffer;
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
    is.setstate( iss.rdstate() & ~ios::eofbit );
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
  static ostringstream buffer;
  buffer.str( "" );
  buffer.clear();
  content.WriteBinary( buffer );
  LengthField<2> length = buffer.str().length();
  length.WriteBinary( os );
  os.write( buffer.str().data(), length );
  return os;
}

// Specializations. The protocol could be changed to remove them.
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
  for( size_t i = 0; i < parameters.Size(); ++i )
    PutMessage( os, parameters[ i ] );
  return os;
}

template<>
ostream&
MessageHandler::PutMessage( ostream& os, const STATELIST& states )
{
  for( size_t i = 0; i < states.Size(); ++i )
    PutMessage( os, states[ i ] );
  return os;
}

// Enforce instantiation of all message construction functions here,
// i.e. in this compilation unit.
// Only those not used in one of the functions above need explicit instantiation.
template ostream& MessageHandler::PutMessage( std::ostream&, const PARAMLIST& );
template ostream& MessageHandler::PutMessage( std::ostream&, const STATELIST& );
template ostream& MessageHandler::PutMessage( std::ostream&, const STATEVECTOR& );
template ostream& MessageHandler::PutMessage( std::ostream&, const STATUS& );
template ostream& MessageHandler::PutMessage( std::ostream&, const SYSCMD& );
template ostream& MessageHandler::PutMessage( std::ostream&, const GenericSignal& );
template ostream& MessageHandler::PutMessage( std::ostream&, const VisCfg& );
template ostream& MessageHandler::PutMessage( std::ostream&, const VisMemo& );


