////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: System-related object types for the script interpreter.
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

#include "SystemTypes.h"

#include "ScriptInterpreter.h"
#include "StateMachine.h"
#include "BCI_OperatorLib.h"
#include "BCIException.h"
#include "Version.h"
#include "VersionInfo.h"
#include "ParameterTypes.h"
#include "StateTypes.h"
#include "EventTypes.h"

using namespace std;
using namespace Interpreter;

//// SystemType
SystemType SystemType::sInstance;
const ObjectType::MethodEntry SystemType::sMethodTable[] =
{
  METHOD( Get ), METHOD( SetConfig ),
  METHOD( Start ), METHOD( Stop ), { "Suspend", &Stop },
  METHOD( Startup ), METHOD( Shutdown ), METHOD( Reset ),
  METHOD( Quit ), { "Exit", &Quit },
  END
};

bool
SystemType::Get( ScriptInterpreter& inInterpreter )
{
  string noun = inInterpreter.GetToken();
  if( !::stricmp( noun.c_str(), "State" ) )
    GetState( inInterpreter );
  else if( !::stricmp( noun.c_str(), "Version" ) )
    GetVersion( inInterpreter );
  else
    throw bciexception_( "Cannot get anything from System except State or Version" );
  return true;
}

bool
SystemType::GetState( ScriptInterpreter& inInterpreter )
{
  const struct { int value; const char* name; }
  states[] =
  {
    #define ENTRY(x) { BCI_State##x, #x }
    ENTRY( Unavailable ),
    ENTRY( Idle ),
    ENTRY( Startup ),
    ENTRY( Initialization ),
    ENTRY( Resting ),
    ENTRY( Suspended ),
    ENTRY( ParamsModified ),
    ENTRY( Running ),
    ENTRY( Termination ),
    ENTRY( Busy ),
    #undef ENTRY
  };
  int state = BCI_GetStateOfOperation();
  size_t i = 0;
  string result;
  while( result.empty() && i < sizeof( states ) / sizeof( *states ) )
    if( states[i].value == state )
      result = states[i].name;
    else
      ++i;
  if( result.empty() )
    throw bciexception_( "Unknown system state: " << state );
  inInterpreter.Out() << result;
  return true;
}

bool
SystemType::GetVersion( ScriptInterpreter& inInterpreter )
{
  istringstream iss( BCI2000_VERSION );
  VersionInfo info;
  iss >> info;
  info.WriteToStream( inInterpreter.Out(), true );
  return true;
}

bool
SystemType::SetConfig( ScriptInterpreter& inInterpreter )
{
  if( !inInterpreter.StateMachine().SetConfig() )
    throw bciexception_( "Could not set configuration" );
  return true;
}

bool
SystemType::Start( ScriptInterpreter& inInterpreter )
{
  if( !inInterpreter.StateMachine().StartRun() )
    throw bciexception_( "Could not start operation" );
  return true;
}

bool
SystemType::Stop( ScriptInterpreter& inInterpreter )
{
  if( !inInterpreter.StateMachine().StopRun() )
    throw bciexception_( "Could not stop operation" );
  return true;
}

bool
SystemType::Startup( ScriptInterpreter& inInterpreter )
{
  string args = inInterpreter.GetRemainder();
  if( !inInterpreter.StateMachine().Startup( args.c_str() ) )
    throw bciexception_( "Could not start up system" );
  return true;
}

bool
SystemType::Shutdown( ScriptInterpreter& inInterpreter )
{
  if( !inInterpreter.StateMachine().Shutdown() )
    throw bciexception_( "Could not shut down system" );
  return true;
}

bool
SystemType::Reset( ScriptInterpreter& inInterpreter )
{
  inInterpreter.StateMachine().StopRun();
  inInterpreter.StateMachine().Shutdown();
  ParametersType::Clear( inInterpreter );
  StatesType::Clear( inInterpreter );
  EventsType::Clear( inInterpreter );
  ScriptInterpreter::Initialize( inInterpreter.StateMachine() );
  return true;
}

bool
SystemType::Quit( ScriptInterpreter& inInterpreter )
{
  if( !inInterpreter.StateMachine().CallbackFunction( BCI_OnQuitRequest ) )
    throw bciexception_( "Quit request not handled by application" );
  const char* pMessage = NULL;
  inInterpreter.StateMachine().ExecuteCallback( BCI_OnQuitRequest, &pMessage );
  if( pMessage && *pMessage )
    inInterpreter.Out() << pMessage;
  return true;
}

//// ConfigType
ConfigType ConfigType::sInstance;
const ObjectType::MethodEntry ConfigType::sMethodTable[] =
{
  METHOD( Set ),
  END
};

bool
ConfigType::Set( ScriptInterpreter& inInterpreter )
{
  return SystemType::SetConfig( inInterpreter );
}
