////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A script interpreter type that handles global commands.
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

#include "ImpliedType.h"

#include "SystemTypes.h"
#include "StateTypes.h"
#include "ParameterTypes.h"
#include "EventTypes.h"
#include "SignalTypes.h"
#include "VariableType.h"
#include "MessageTypes.h"
#include "ExpressionType.h"
#include "ConditionType.h"
#include "FileSystemTypes.h"
#include "CommandInterpreter.h"
#include "StateMachine.h"
#include "BCI_OperatorLib.h"
#include "BCIException.h"
#include "ParserToken.h"
#include "ProcessUtils.h"
#include "EnvVariable.h"

using namespace std;
using namespace Interpreter;

ImpliedType ImpliedType::sInstance;
const ObjectType::MethodEntry ImpliedType::sMethodTable[] =
{
  METHOD( Get ), METHOD( Set ),
  METHOD( Wait ), METHOD( Sleep ),
  METHOD( SetConfig ),
  METHOD( Start ), { "Resume", &Start }, METHOD( Stop ), { "Suspend", &Stop },
  METHOD( Startup ), METHOD( Shutdown ), METHOD( Reset ),
  METHOD( Quit ), { "Exit", &Quit },
  METHOD( Version ),
  METHOD( Log ), METHOD( Warn ), METHOD( Error ),
  METHOD( System ), METHOD( Echo ), METHOD( Ls ), { "Dir", &Ls }, METHOD( Cd ), METHOD( Pwd ),
  { "[", &Square },
  END
};

void
ImpliedType::OnHelp( CommandInterpreter& inInterpreter ) const
{
  inInterpreter.Out() << "Control statements: if, elseif, else, end, while, do, until\n";
  inInterpreter.Out() << "Global commands: ";
  this->ListMethods( inInterpreter.Out() );

  const ObjectType* pType = ObjectType::Next( this );
  if( pType && pType != this )
    inInterpreter.Out() << "\nCommands for objects of type";
  while( pType && pType != this )
  {
    ostringstream oss;
    pType->ListMethods( oss );
    if( !oss.str().empty() )
      inInterpreter.Out() << "\n " << pType->Name() << ": " << oss.str();
    pType = ObjectType::Next( pType );
  }
  const char* pHelpString = NULL;
  inInterpreter.StateMachine().ExecuteCallback( BCI_OnScriptHelp, &pHelpString );
  if( pHelpString )
    inInterpreter.Out() << "\nApplication-defined commands: " << pHelpString;
}

bool
ImpliedType::Get( CommandInterpreter& inInterpreter )
{
  string object = inInterpreter.GetToken();
  inInterpreter.Unget();
  CommandInterpreter::ArgumentList args;
  inInterpreter.ParseArguments( object, args );
  if( !::stricmp( object.c_str(), "Signal" ) )
    return SignalType::Get( inInterpreter );
  if( inInterpreter.StateMachine().States().Exists( object ) )
    return StateType::Get( inInterpreter );
  if( inInterpreter.StateMachine().Parameters().Exists( object ) )
    return ParameterType::Get( inInterpreter );
  if( inInterpreter.LocalVariables().Exists( object ) || EnvVariable::Get( object, string() ) )
    return VariableType::Get( inInterpreter );
  try
  {
    ExpressionType::Evaluate( inInterpreter );
    return true;
  }
  catch( const BCIException& )
  {
    inInterpreter.Unget();
  }
  if( ConditionType::EvaluateExpression( inInterpreter ) )
    return true;
  inInterpreter.Unget();

  return false;
}

bool
ImpliedType::Set( CommandInterpreter& inInterpreter )
{
  string object = inInterpreter.GetToken();
  inInterpreter.Unget();
  CommandInterpreter::ArgumentList args;
  inInterpreter.ParseArguments( object, args );
  if( inInterpreter.StateMachine().Events().Exists( object ) )
    return EventType::Set( inInterpreter );
  if( inInterpreter.StateMachine().States().Exists( object ) )
    return StateType::Set( inInterpreter );
  if( inInterpreter.StateMachine().Parameters().Exists( object ) )
    return ParameterType::Set( inInterpreter );
  return VariableType::Set( inInterpreter );
}

bool
ImpliedType::Wait( CommandInterpreter& inInterpreter )
{
  string token = inInterpreter.GetToken();
  if( !::stricmp( token.c_str(), "for" ) )
    return Interpreter::SystemType::WaitFor( inInterpreter );
  inInterpreter.Unget();
  return Interpreter::SystemType::Sleep( inInterpreter );
}

bool
ImpliedType::Sleep( CommandInterpreter& inInterpreter )
{
  return Interpreter::SystemType::Sleep( inInterpreter );
}

bool
ImpliedType::SetConfig( CommandInterpreter& inInterpreter )
{
  return SystemType::SetConfig( inInterpreter );
}

bool
ImpliedType::Start( CommandInterpreter& inInterpreter )
{
  return SystemType::Start( inInterpreter );
}

bool
ImpliedType::Stop( CommandInterpreter& inInterpreter )
{
  return SystemType::Stop( inInterpreter );
}

bool
ImpliedType::Startup( CommandInterpreter& inInterpreter )
{
  return SystemType::Startup( inInterpreter );
}

bool
ImpliedType::Shutdown( CommandInterpreter& inInterpreter )
{
  return SystemType::Shutdown( inInterpreter );
}

bool
ImpliedType::Reset( CommandInterpreter& inInterpreter )
{
  return SystemType::Reset( inInterpreter );
}

bool
ImpliedType::Quit( CommandInterpreter& inInterpreter )
{
  return SystemType::Quit( inInterpreter );
}

bool
ImpliedType::Version( CommandInterpreter& inInterpreter )
{
  return SystemType::GetVersion( inInterpreter );
}

bool
ImpliedType::Log( CommandInterpreter& inInterpreter )
{
  return MessageType::Log( inInterpreter );
}

bool
ImpliedType::Warn( CommandInterpreter& inInterpreter )
{
  return WarningType::Issue( inInterpreter );
}

bool
ImpliedType::Error( CommandInterpreter& inInterpreter )
{
  return ErrorType::Report( inInterpreter );
}

bool
ImpliedType::System( CommandInterpreter& inInterpreter )
{
  string command = inInterpreter.GetRemainder();
#if _WIN32
  string shell = "cmd /c";
#else
  string shell = "/bin/sh -c";
#endif
  int exitCode = 0;
  if( !ProcessUtils::ExecuteSynchronously( shell, command, inInterpreter.Out(), exitCode ) )
    throw bciexception_( "Could not run \"" << command.c_str() << "\"" );
  if( exitCode != 0 )
    inInterpreter.Out() << inInterpreter.ExitCodeTag() << exitCode;
  inInterpreter.Log() << "Executed \"" << command.c_str() << "\"";
  return true;
}

bool
ImpliedType::Echo( CommandInterpreter& inInterpreter )
{
  string token = inInterpreter.GetToken();
  if( !token.empty() )
    inInterpreter.Out() << token;
  token = inInterpreter.GetToken();
  while( !token.empty() )
  {
    inInterpreter.Out() << " " << token;
    token = inInterpreter.GetToken();
  }
  inInterpreter.Unget();
  return true;
}

bool
ImpliedType::Ls( CommandInterpreter& inInterpreter )
{
  return DirectoryType::List( inInterpreter );
}

bool
ImpliedType::Cd( CommandInterpreter& inInterpreter )
{
  string newWd = inInterpreter.GetToken();
  inInterpreter.Unget();
  if( newWd.empty() )
    return Pwd( inInterpreter );
  return DirectoryType::Change( inInterpreter );
}

bool
ImpliedType::Pwd( CommandInterpreter& inInterpreter )
{
  return DirectoryType::Current( inInterpreter );
}

bool
ImpliedType::Square( CommandInterpreter& inInterpreter )
{
  return ConditionType::Evaluate( inInterpreter );
}

//// ExecutableType
ExecutableType ExecutableType::sInstance;
const ObjectType::MethodEntry ExecutableType::sMethodTable[] =
{
  METHOD( Start ),
  END
};

bool
ExecutableType::Start( CommandInterpreter& inInterpreter )
{
  string executable = inInterpreter.GetToken(),
         arguments = inInterpreter.GetRemainder();
  int exitCode = 0;
  if( !ProcessUtils::ExecuteAsynchronously( executable, arguments, exitCode ) )
    throw bciexception_( "Could not run \"" << executable.c_str() << "\"" );
  if( exitCode != 0 )
    inInterpreter.Out() << inInterpreter.ExitCodeTag() << exitCode;
  inInterpreter.Log() << "Executed \"" << executable.c_str() << "\"";
  return true;
}
