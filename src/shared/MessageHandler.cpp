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
#pragma warn -ccc
#pragma warn -rch

#include "MessageHandler.h"

#include <UStatus.h>
#include <UParameter.h>
#include <UState.h>
#include <UGenericSignal.h>
#include <USysCommand.h>

using namespace std;

// A macro that contains the structure of a case statement in the
// main message handling function.
#define CONSIDER(x)                                                      \
  case Header<x>::desc:                                                  \
    if( Header<x>::supp == ignore || Header<x>::supp == supplement )     \
    {                                                                    \
      if( Header<x>::src != ignore )                                     \
      {                                                                  \
        if( Header<x>::src == is.peek() )                                \
        {                                                                \
          is.get();                                                      \
          Handle##x( is ) || is.ignore( length );                        \
        }                                                                \
        else                                                             \
          is.ignore( length );                                           \
      }                                                                  \
      else                                                               \
        Handle##x( is ) || is.ignore( length );                          \
    }                                                                    \
    else                                                                 \
      is.ignore( length );                                               \
    break;

// The main message handling function.
void
MessageHandler::HandleMessage( istream& is )
{
  char descriptor = is.get(),
       supplement = is.get();
  int  length = is.get();
  length |= is.get() << 8;
  if( is )
  {
    switch( descriptor )
    {
      CONSIDER( STATEVECTOR );
      CONSIDER( GenericSignal );
      CONSIDER( STATUS );
      CONSIDER( PARAM );
      CONSIDER( STATE );
      CONSIDER( SYSCMD );
      default:
        bcierr << "Unknown message descriptor" << endl;
    }
  }
}

// Functions that construct messages.

// The generic implementation.
template<typename content_type>
void
MessageHandler::PutMessage( std::ostream& os, const content_type& content )
{
  os.put( Header<content_type>::desc );
  os.put( Header<content_type>::supp );
#ifdef BCI_TOOL // The version without buffering.
  os.put( 0 ).put( 0 );
  content.WriteBinary( os );
  if( Header<content_type>::src != ignore )
    os.put( Header<content_type>::src );
#else // We need the buffer to correctly fill in the length fields.
  static ostringstream buffer;
  buffer.str( "" );
  buffer.clear();
  if( Header<content_type>::src != ignore )
    buffer.put( Header<content_type>::src );
  content.WriteBinary( buffer );
  size_t length = buffer.str().length();
  if( length > ( 1 << 16 ) )
    bcierr << "Message length field overflow" << endl;
  os.put( length & 0xff ).put( ( length >> 8 ) & 0xff );
  os.write( buffer.str().data(), length );
#endif
}

// Enforce instantiation of the message construction functions.
template void MessageHandler::PutMessage( std::ostream&, const STATUS& );
template void MessageHandler::PutMessage( std::ostream&, const PARAM& );
template void MessageHandler::PutMessage( std::ostream&, const STATE& );
template void MessageHandler::PutMessage( std::ostream&, const GenericSignal& );
template void MessageHandler::PutMessage( std::ostream&, const STATEVECTOR& );
template void MessageHandler::PutMessage( std::ostream&, const SYSCMD& );
