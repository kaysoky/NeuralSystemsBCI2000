////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Implementation of bcierr and bciout message handlers for a
//              GUI application.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIError.h"
#include "Status.h"

using namespace std;

struct StatusMessage
{
 void operator()( const string& inText, int inCode )
 {
  string text = inText;
  if( text.find_last_of( ".!?" ) != text.length() - 1 )
    text += '.';

#if !defined( _WIN32 ) || defined( __CONSOLE__ )
  cerr << text << endl;
#else
  ::MessageBox( NULL, text.c_str(), "BCI2000 Error",
                  MB_OK | MB_ICONHAND | MB_SYSTEMMODAL | MB_SETFOREGROUND );
#endif
 }
} StatusMessage;

void
BCIError::DebugMessage( const string& message )
{
  StatusMessage( message.substr( 0, message.length() - 1 ), Status::debugMessage );
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

