////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Implementation of bcierr and bciout message handlers for a
//              BCI2000 module.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIError.h"
#include "Environment.h"
#include "MessageHandler.h"
#include "Status.h"

using namespace std;

struct StatusMessage
{
 void operator()( const string& inText, int inCode )
 {
  static ostream* op = NULL;
  if( op == NULL )
    op = ::EnvironmentBase::Operator;

  string text = inText;
  if( text.find_last_of( ".!?" ) != text.length() - 1 )
    text += '.';

  // If the connection to the operator does not work, fall back to a local
  // error display.
  if( op != NULL )
  {
    MessageHandler::PutMessage( *op, Status( text, inCode ) );
    op->flush();
  }
  if( !op || !*op )
  {
    if( inCode >= 400 )
    {
#if !defined( _WIN32 ) || defined( __CONSOLE__ )
      cerr << text << endl;
#else
      ::MessageBox( NULL, text.c_str(), "BCI2000 Error",
                    MB_OK | MB_ICONHAND | MB_SYSTEMMODAL | MB_SETFOREGROUND );
#endif
    }
    else
    {
#if !defined( _WIN32 ) || defined( __CONSOLE__ )
      cout << text << endl;
#else
      ::MessageBox( NULL, text.c_str(), "BCI2000",
                    MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL | MB_SETFOREGROUND );
#endif
    }
  }
 }
} StatusMessage;


void
BCIError::DebugMessage( const string& message )
{
  StatusMessage( message.substr( 0, message.length() - 1 ), Status::debugMessage );
}

void
BCIError::PlainMessage( const string& message )
{
  StatusMessage( message.substr( 0, message.length() - 1 ), Status::plainMessage );
}

void
BCIError::Warning( const string& message )
{
  StatusMessage( string( "Warning: " ) + message.substr( 0, message.length() - 1 ), Status::warningMessage );
}

void
BCIError::ConfigurationError( const string& message )
{
  StatusMessage( message.substr( 0, message.length() - 1 ), Status::configurationError );
}

void
BCIError::RuntimeError( const string& message )
{
  StatusMessage( message.substr( 0, message.length() - 1 ), Status::runtimeError );
}

void
BCIError::LogicError( const string& message )
{
  StatusMessage( message.substr( 0, message.length() - 1 ), Status::logicError );
}

