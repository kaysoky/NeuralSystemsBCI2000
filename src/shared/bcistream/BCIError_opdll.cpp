////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Implementation of bcierr and bciout message handlers for the
//   BCI2000 operator DLL.
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
#include "StateMachine.h"
#include "BCI_OperatorLib.h"
#include <iostream>

#ifdef _WIN32
# include <windows.h>
#endif // _WIN32

using namespace std;

extern StateMachine* gpStateMachine;

void
BCIError::DebugMessage( const string& message )
{
  Warning( message );
}

void
BCIError::Warning( const string& inMessage )
{
  if( inMessage.length() > 1 )
  {
    string message = inMessage;
    if( message.find_last_of( '\n' ) == message.length() - 1 )
      message = message.substr( 0, message.length() - 1 );
    if( gpStateMachine && gpStateMachine->CallbackFunction( BCI_OnWarningMessage ) )
      gpStateMachine->LogMessage( BCI_OnWarningMessage, message.c_str() );
    else
    {
#ifdef _WIN32
      ::MessageBoxA( NULL, message.c_str(), "BCI2000 Operator", MB_OK );
#else
      cout << message << endl;
#endif
    }
  }
}

void
BCIError::ConfigurationError( const string& inMessage )
{
  if( inMessage.length() > 1 )
  {
    string message = inMessage;
    if( message.find_last_of( '\n' ) == message.length() - 1 )
      message = message.substr( 0, message.length() - 1 );
    if( gpStateMachine && gpStateMachine->CallbackFunction( BCI_OnErrorMessage ) )
      gpStateMachine->LogMessage( BCI_OnErrorMessage, message.c_str() );
    else
    {
#ifdef _WIN32
      ::MessageBoxA( NULL, message.c_str(), "BCI2000 Operator Error", MB_OK );
#else
      cerr << message << endl;
#endif
    }
  }
}

void
BCIError::RuntimeError( const string& message )
{
  ConfigurationError( message );
}

void
BCIError::LogicError( const string& message )
{
  ConfigurationError( message );
}
