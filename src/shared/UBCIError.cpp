////////////////////////////////////////////////////////////////////////////////
//
// File: UBCIError.cpp
//
// Description: Declarations for stream symbols related to error handling.
//              To report an error, write e.g.
//               bcierr << "My error message" << endl;
//              For an informational message, write
//               bciout << "My info message" << endl;
//
// Author: Juergen Mellinger
//
// Date:   Mar 27, 2003
//
// Changes: Apr 16, 2003: Replaced dummy implementations by objects that
//          actually hold messages.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UBCIError.h"

#include "UCoreComm.h"
#include "UEnvironment.h"

using namespace std;
using namespace BCIError;

// Definitions of the actual global objects.
bci_ostream __bcierr;
bci_ostream __bciout;

std::ostream&
bci_ostream::operator()( const char* inInfoHeader )
{
#if 0
  static string lastInfoHeader;
  if( inInfoHeader != lastInfoHeader )
  {
    *this << inInfoHeader << ": ";
    lastInfoHeader = inInfoHeader;
  }
#else
  *this << inInfoHeader << ": ";
#endif
  return ( *this )();
}

void
bci_ostream::bci_stringbuf::SetFlushHandler( bci_ostream::flush_handler f )
{
  if( on_flush )
    on_flush( str() );
  else
    f( str() );
  on_flush = f;
  num_flushes = 0;
}

int
bci_ostream::bci_stringbuf::sync()
{
  int r = stringbuf::sync();
  ++num_flushes;
  if( on_flush )
  {
    on_flush( str() );
    str( "" );
  }
  return r;
}

// A preliminary implementation of the error display/error handling mechanism
// on the core module side.
struct StatusMessage
{
 void operator()( int code, const std::string& text )
 {
  static CORECOMM* corecomm = NULL;
  if( corecomm == NULL )
    corecomm = ::Environment::Corecomm;

  ostringstream status;
  status << code << " " << text;
  // If the connection to the operator does not work, fall back to a local
  // error display.
  if( !( corecomm && corecomm->SendStatus( status.str().c_str() ) ) && code >= 400 )
    ::MessageBox( NULL, text.c_str(), "BCI2000 Error",
                    MB_OK | MB_ICONHAND | MB_SYSTEMMODAL | MB_SETFOREGROUND );
 }
} StatusMessage;

void
BCIError::Warning( const std::string& message )
{
  if( message.length() > 1 )
    StatusMessage( 301, string( "Warning: " ) + message.substr( 0, message.length() - 1 ) );
}

void
BCIError::ConfigurationError( const std::string& message )
{
  if( message.length() > 1 )
    StatusMessage( 408, message.substr( 0, message.length() - 1 ) );
}

void
BCIError::RuntimeError( const std::string& message )
{
  if( message.length() > 1 )
    StatusMessage( 409, message.substr( 0, message.length() - 1 ) );
}

void
BCIError::LogicError( const std::string& message )
{
  if( message.length() > 1 )
    StatusMessage( 499, message.substr( 0, message.length() - 1 ) );
}

