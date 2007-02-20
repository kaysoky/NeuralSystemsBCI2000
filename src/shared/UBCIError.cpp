////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: UBCIError.cpp
//
// Author: Juergen Mellinger
//
// Date:   Mar 27, 2003
//
// Changes: Apr 16, 2003: Replaced dummy implementations by objects that
//          actually hold messages.
//          Jul 22, 2003: Added implementations for command line tools.
// $Log$
// Revision 1.10  2006/01/17 17:39:44  mellinger
// Fixed list of project files.
//
// Revision 1.9  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
//
// Description: Declarations for stream symbols related to error handling.
//              To report an error, write e.g.
//               bcierr << "My error message" << endl;
//              For an informational message, write
//               bciout << "My info message" << endl;
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UBCIError.h"
#include <iostream>

#if( !defined( BCI_TOOL ) && !defined( BCI_DLL ) && !defined( BCI_MEX ) )
# include "UEnvironment.h"
# include "MessageHandler.h"
# include "UStatus.h"
#endif

#ifdef BCI_MEX
# include "mex.h"
#endif

using namespace std;
using namespace BCIError;

// Definitions of the actual global objects.
bci_ostream __bcierr;
bci_ostream __bciout;

#ifdef BCI_TOOL
# ifdef BCI_DLL
extern ostream sErr;
ostream& _err = sErr;
# else
ostream& _err = cerr;
# endif // BCI_DLL
#endif // BCI_TOOL

ostream&
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
  if( mp_on_flush )
    mp_on_flush( str() );
  else
    f( str() );
  str( "" );
  mp_on_flush = f;
}

int
bci_ostream::bci_stringbuf::sync()
{
  int r = stringbuf::sync();
  ++m_num_flushes;
  if( mp_on_flush )
  {
    mp_on_flush( str() );
    str( "" );
  }
  return r;
}

#ifdef BCI_TOOL // implementation for a filter wrapper
void
BCIError::Warning( const string& message )
{
  if( message.length() > 1 )
    _err << message << endl;
}

void
BCIError::ConfigurationError( const string& message )
{
  if( message.length() > 1 )
    _err << "Configuration Error: " << message << endl;
}

void
BCIError::RuntimeError( const string& message )
{
  if( message.length() > 1 )
    _err << "Runtime Error: " << message << endl;
}

void
BCIError::LogicError( const string& message )
{
  if( message.length() > 1 )
    _err << "Logic Error: " << message << endl;
}
#elif( defined( BCI_MEX ) ) // implementation for a Matlab mex file
void
BCIError::Warning( const string& message )
{
  if( message.length() > 1 )
    mexWarnMsgTxt( message.c_str() );
}

void
BCIError::ConfigurationError( const string& message )
{
  if( message.length() > 1 )
    mexErrMsgTxt( message.c_str() );
}

void
BCIError::RuntimeError( const string& message )
{
  if( message.length() > 1 )
    mexErrMsgTxt( message.c_str() );
}

void
BCIError::LogicError( const string& message )
{
  if( message.length() > 1 )
    mexErrMsgTxt( message.c_str() );
}
#else // implementation for a module
// A preliminary implementation of the error display/error handling mechanism
// on the core module side.
struct StatusMessage
{
 void operator()( const string& inText, int inCode )
 {
  static ostream* op = NULL;
  if( op == NULL )
    op = ::Environment::Operator;

  string text = inText;
  if( text.find_last_of( ".!?" ) != text.length() - 1 )
    text += '.';

  // If the connection to the operator does not work, fall back to a local
  // error display.
  if( op != NULL )
  {
    MessageHandler::PutMessage( *op, STATUS( text.c_str(), inCode ) );
    op->flush();
  }
  if( ( !op || !*op ) && inCode >= 400 )
#if !defined( _WIN32 ) || defined( __CONSOLE__ )
    cerr << text << endl;
#else
    ::MessageBox( NULL, text.c_str(), "BCI2000 Error",
                    MB_OK | MB_ICONHAND | MB_SYSTEMMODAL | MB_SETFOREGROUND );
#endif
 }
} StatusMessage;

void
BCIError::Warning( const string& message )
{
  if( message.length() > 1 )
    StatusMessage( string( "Warning: " ) + message.substr( 0, message.length() - 1 ), 301 );
}

void
BCIError::ConfigurationError( const string& message )
{
  if( message.length() > 1 )
    StatusMessage( message.substr( 0, message.length() - 1 ), 408 );
}

void
BCIError::RuntimeError( const string& message )
{
  if( message.length() > 1 )
    StatusMessage( message.substr( 0, message.length() - 1 ), 409 );
}

void
BCIError::LogicError( const string& message )
{
  if( message.length() > 1 )
    StatusMessage( message.substr( 0, message.length() - 1 ), 499 );
}
#endif // BCI_TOOL
