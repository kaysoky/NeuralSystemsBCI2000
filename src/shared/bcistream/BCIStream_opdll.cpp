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

#include "BCIStream.h"
#include "StateMachine.h"
#include "BCI_OperatorLib.h"
#include "RedirectIO.h"

#ifdef _WIN32
# include <windows.h>
#endif // _WIN32

using namespace std;

extern StateMachine* gpStateMachine;

static void
Handle( const string& inMessage, int inCallback )
{
  string message = inMessage;
  if( message.find_last_of( '\n' ) == message.length() - 1 )
    message = message.substr( 0, message.length() - 1 );
  if( gpStateMachine && gpStateMachine->CallbackFunction( inCallback ) )
    gpStateMachine->LogMessage( inCallback, message.c_str() );
  else
  {
    std::ostream* pOut = &Tiny::Cout();
    string title = "BCI2000 Operator";
    switch( inCallback )
    {
      case BCI_OnErrorMessage:
        title += " Error";
        pOut = &Tiny::Cerr();
        break;
      case BCI_OnWarningMessage:
        title += " Warning";
        pOut = &Tiny::Cerr();
        break;
    }
#ifdef _WIN32
    ::MessageBoxA( NULL, message.c_str(), title.c_str(), MB_OK );
#else
    *pOut << message << endl;
#endif
  }
}

bool
BCIStream::CompressMessages()
{
  return false;
}

void
BCIStream::PlainMessage( const string& s )
{
  Handle( s, BCI_OnLogMessage );
}

void
BCIStream::DebugMessage( const string& s )
{
  Handle( s, BCI_OnDebugMessage );
}

void
BCIStream::Warning( const string& s )
{
  Handle( s, BCI_OnWarningMessage );
}

void
BCIStream::ConfigurationError( const string& s )
{
  Handle( s, BCI_OnErrorMessage );
}

void
BCIStream::RuntimeError( const string& s )
{
  Handle( s, BCI_OnErrorMessage );
}

void
BCIStream::LogicError( const string& s )
{
  Handle( s, BCI_OnErrorMessage );
}
