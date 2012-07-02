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

#include "CommandInterpreter.h"
#include "StateMachine.h"
#include "BCI_OperatorLib.h"
#include "BCIException.h"
#include "Version.h"
#include "VersionInfo.h"
#include "ParameterTypes.h"
#include "StateTypes.h"
#include "EventTypes.h"
#include <limits>

using namespace std;
using namespace Interpreter;

static const int cTimeResolution = 50; // for Sleep and Wait commands, in ms
static const double cDefaultTimeout = numeric_limits<double>::infinity(); // for Wait commands, in s

//// SystemType
SystemType SystemType::sInstance;
const ObjectType::MethodEntry SystemType::sMethodTable[] =
{
  METHOD( Get ), METHOD( WaitFor ), METHOD( Sleep ), METHOD( SetConfig ),
  METHOD( Start ), METHOD( Stop ), { "Suspend", &Stop },
  METHOD( Startup ), METHOD( Shutdown ), METHOD( Reset ),
  METHOD( Quit ), { "Exit", &Quit },
  END
};

static const struct { int value; const char* name; }
sSystemStates[] =
{
  #define ENTRY(x) { BCI_State##x, #x }
  ENTRY( Unavailable ),
  ENTRY( Idle ),
  ENTRY( Startup ),
  ENTRY( Initialization ), ENTRY( Connected ),
  ENTRY( Resting ),
  ENTRY( Suspended ),
  ENTRY( ParamsModified ),
  ENTRY( Running ),
  ENTRY( Busy ),
  #undef ENTRY
};

bool
SystemType::Get( CommandInterpreter& inInterpreter )
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
SystemType::GetState( CommandInterpreter& inInterpreter )
{
  int state = BCI_GetStateOfOperation();
  size_t i = 0;
  string result;
  while( result.empty() && i < sizeof( sSystemStates ) / sizeof( *sSystemStates ) )
    if( sSystemStates[i].value == state )
      result = sSystemStates[i].name;
    else
      ++i;
  if( result.empty() )
    throw bciexception_( "Unknown system state: " << state );
  inInterpreter.Out() << result;
  return true;
}

bool
SystemType::GetVersion( CommandInterpreter& inInterpreter )
{
  VersionInfo::Current.WriteToStream( inInterpreter.Out(), true );
  return true;
}


bool
SystemType::WaitFor( CommandInterpreter& inInterpreter )
{
  string states = inInterpreter.GetToken();
  set<int> desiredStates;
  istringstream iss( states );
  string stateName;
  while( std::getline( iss, stateName, '|' ) )
  {
    size_t i = 0;
    int stateCode;
    bool found = false;
    while( !found && i < sizeof( sSystemStates ) / sizeof( *sSystemStates ) )
      if( !::stricmp( sSystemStates[i].name, stateName.c_str() ) )
      {
        stateCode = sSystemStates[i].value;
        found = true;
      }
      else
        ++i;
    if( !found )
      throw bciexception_( "Unknown system state: " << stateName );
    desiredStates.insert( stateCode );
  }
  double timeout = 0;
  if( !( istringstream( inInterpreter.GetOptionalToken() ) >> timeout ) )
    timeout = cDefaultTimeout;
  if( timeout < 0 )
    throw bciexception_( "Timeout must be >= 0" );

  return DoWaitFor( desiredStates, timeout, inInterpreter );
}

bool
SystemType::DoWaitFor( const set<int>& inStates, double inTimeout, CommandInterpreter& inInterpreter )
{
  int timeElapsed = 0,
      state = BCI_GetStateOfOperation();
  while( inStates.find( state ) == inStates.end()
        && state != BCI_StateUnavailable
        && timeElapsed < 1e3 * inTimeout )
  {
    timeElapsed += inInterpreter.Background( cTimeResolution );
    state = BCI_GetStateOfOperation();
  }
  if( inStates.find( state ) == inStates.end() )
    throw bciexception_( "Wait aborted" );
  else if( timeElapsed >= 1e3 * inTimeout )
    inInterpreter.Out() << "Timeout occurred after " << inTimeout << " seconds";
  return true;
}

bool
SystemType::Sleep( CommandInterpreter& inInterpreter )
{
  double duration = 0;
  if( !( istringstream( inInterpreter.GetToken() ) >> duration ) )
    throw bciexception_( "Invalid sleep duration" );
  if( duration < 0 )
    throw bciexception_( "Sleep duration must be >= 0" );
  int timeElapsed = 0;
  while( timeElapsed < 1e3 * duration )
    timeElapsed += inInterpreter.Background( cTimeResolution );
  return true;
}

bool
SystemType::SetConfig( CommandInterpreter& inInterpreter )
{
  if( !inInterpreter.StateMachine().SetConfig() )
    throw bciexception_( "Must be in Connected state to set configuration" );
  set<int> states;
  states.insert( BCI_StateConnected );
  states.insert( BCI_StateResting );
  DoWaitFor( states, cDefaultTimeout, inInterpreter );
  if( BCI_GetStateOfOperation() != BCI_StateResting )
    throw bciexception_( "Could not set configuration" );
  return true;
}

bool
SystemType::Start( CommandInterpreter& inInterpreter )
{
  if( BCI_GetStateOfOperation() == BCI_StateRunning )
  {
    inInterpreter.Out() << "System already in Running state";
    return true;
  }
  if( !inInterpreter.StateMachine().StartRun() )
    throw bciexception_( "Must be in Resting or Suspended state to start operation" );
  set<int> states;
  states.insert( BCI_StateRunning );
  DoWaitFor( states, cDefaultTimeout, inInterpreter );
  if( BCI_GetStateOfOperation() != BCI_StateRunning )
    throw bciexception_( "Could not start operation" );
  return true;
}

bool
SystemType::Stop( CommandInterpreter& inInterpreter )
{
  if( BCI_GetStateOfOperation() == BCI_StateResting
      || BCI_GetStateOfOperation() == BCI_StateSuspended )
  {
    inInterpreter.Out() << "System not in Running state";
    return true;
  }
  bool success = inInterpreter.StateMachine().StopRun();
  if( success )
  {
    set<int> states;
    states.insert( BCI_StateSuspended );
    DoWaitFor( states, cDefaultTimeout, inInterpreter );
    success = ( BCI_GetStateOfOperation() == BCI_StateSuspended );
  }
  if( !success )
    throw bciexception_( "Could not stop operation" );
  return true;
}

bool
SystemType::Startup( CommandInterpreter& inInterpreter )
{
  string args = inInterpreter.GetOptionalRemainder();
  if( BCI_GetStateOfOperation() != BCI_StateIdle )
  {
    inInterpreter.Out() << "System already started up";
    return true;
  }
  bool success = inInterpreter.StateMachine().Startup( args.c_str() );
  if( success )
  {
    set<int> states;
    states.insert( BCI_StateStartup );
    states.insert( BCI_StateConnected );
    DoWaitFor( states, cDefaultTimeout, inInterpreter );
    success = ( BCI_GetStateOfOperation() == BCI_StateStartup
                || BCI_GetStateOfOperation() == BCI_StateConnected );
  }
  if( !success )
    throw bciexception_( "Could not start up system" );
  return true;
}

bool
SystemType::Shutdown( CommandInterpreter& inInterpreter )
{
  if( BCI_GetStateOfOperation() == BCI_StateIdle )
  {
    inInterpreter.Out() << "System already in Idle state";
    return true;
  }
  if( BCI_GetStateOfOperation() == BCI_StateRunning )
    Stop( inInterpreter );
  bool success = inInterpreter.StateMachine().Shutdown();
  if( success )
  {
    set<int> states;
    states.insert( BCI_StateIdle );
    DoWaitFor( states, cDefaultTimeout, inInterpreter );
    success = ( BCI_GetStateOfOperation() == BCI_StateIdle );
  }
  if( !success )
    throw bciexception_( "Could not shut down system" );
  return true;
}

bool
SystemType::Reset( CommandInterpreter& inInterpreter )
{
  if( BCI_GetStateOfOperation() != BCI_StateIdle )
    Shutdown( inInterpreter );
  inInterpreter.StateMachine().Reset();
  ObjectType::Initialize( inInterpreter.StateMachine() );
  return true;
}

bool
SystemType::Quit( CommandInterpreter& inInterpreter )
{
  string result = inInterpreter.GetOptionalToken();
  if( !inInterpreter.StateMachine().CallbackFunction( BCI_OnQuitRequest ) )
    throw bciexception_( "Quit request not handled by application" );
  const char* pMessage = NULL;
  inInterpreter.StateMachine().ExecuteCallback( BCI_OnQuitRequest, &pMessage );
  if( pMessage && *pMessage )
    inInterpreter.Out() << pMessage;
  else
  {
    if( !result.empty() )
      inInterpreter.Out() << result << '\n';
    inInterpreter.Out() << inInterpreter.TerminationTag();
  }
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
ConfigType::Set( CommandInterpreter& inInterpreter )
{
  return SystemType::SetConfig( inInterpreter );
}
