////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Implementation of bcierr and bciout message handlers for a
//              BCI2000 module.
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

#include "BCIError.h"
#include "Environment.h"
#include "MessageHandler.h"
#include "OSMutex.h"
#include "Status.h"

using namespace std;

static ostream* spOutputStream = NULL;
static const OSMutex* spOutputLock = NULL;

void
StatusMessage( const string& inText, int inCode )
 {
  string text = inText;
  if( text.find_last_of( ".!?" ) != text.length() - 1 )
    text += '.';

  // If the connection to the operator does not work, fall back to a local
  // error display.
  if( spOutputStream != NULL )
  {
    OSMutex::Lock lock( spOutputLock );
    MessageHandler::PutMessage( *spOutputStream, Status( text, inCode ) );
    spOutputStream->flush();
  }
  if( !spOutputStream || !*spOutputStream )
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

void
BCIError::SetOperatorStream( ostream* pStream, const OSMutex* pLock )
{
  spOutputStream = pStream;
  spOutputLock = pLock;
}

