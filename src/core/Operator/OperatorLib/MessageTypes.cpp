////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Message-related object types for the script interpreter.
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

#include "MessageTypes.h"

#include "ScriptInterpreter.h"
#include "StateMachine.h"
#include "BCI_OperatorLib.h"
#include "BCIException.h"
#include "Version.h"
#include "VersionInfo.h"

using namespace std;
using namespace Interpreter;

//// MessageType
MessageType MessageType::sInstance;
const ObjectType::MethodEntry MessageType::sMethodTable[] =
{
  METHOD( Log ),
  END
};

bool
MessageType::Log( ScriptInterpreter& inInterpreter )
{
  string message = inInterpreter.GetRemainder();
  if( message.empty() )
    message = "<empty log entry>";
  inInterpreter.StateMachine().ExecuteCallback( BCI_OnLogMessage, message.c_str() );
  return true;
}

//// WarningType
WarningType WarningType::sInstance;
const ObjectType::MethodEntry WarningType::sMethodTable[] =
{
  METHOD( Issue ), { "Log", &Issue }, { "Show", &Issue },
  END
};

bool
WarningType::Issue( ScriptInterpreter& inInterpreter )
{
  string message = inInterpreter.GetRemainder();
  if( message.empty() )
    message = "unspecified warning";
  inInterpreter.StateMachine().ExecuteCallback( BCI_OnWarningMessage, message.c_str() );
  return true;
}

//// ErrorType
ErrorType ErrorType::sInstance;
const ObjectType::MethodEntry ErrorType::sMethodTable[] =
{
  METHOD( Report ), { "Log", &Report }, { "Show", &Report },
  END
};

bool
ErrorType::Report( ScriptInterpreter& inInterpreter )
{
  string message = inInterpreter.GetRemainder();
  if( message.empty() )
    message = "unspecified error";
  inInterpreter.StateMachine().ExecuteCallback( BCI_OnErrorMessage, message.c_str() );
 return true;
}
