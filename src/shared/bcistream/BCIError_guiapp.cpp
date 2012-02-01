////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Implementation of bcierr and bciout message handlers for a
//              GUI application.
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
#include "Status.h"

#ifdef __BORLANDC__
# include <vcl.h>
#else
# include <QMessageBox>
#endif

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
# ifdef __BORLANDC__
 ::MessageBoxA( NULL, text.c_str(), "BCI2000 Error",
                   MB_OK | MB_ICONHAND | MB_SYSTEMMODAL | MB_SETFOREGROUND );
# else // __BORLANDC__
 QMessageBox::critical( NULL, "BCI2000 Error", text.c_str() );
# endif // __BORLANDC__
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

