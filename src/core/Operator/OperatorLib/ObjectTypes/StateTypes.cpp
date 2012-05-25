////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: State-related object types for the script interpreter.
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

#include "StateTypes.h"
#include "State.h"
#include "CommandInterpreter.h"
#include "StateMachine.h"
#include "Lockable.h"
#include "WildcardMatch.h"
#include "BCIException.h"
#include "BCI_OperatorLib.h"
#include <cstdlib>

using namespace std;
using namespace bci;
using namespace Interpreter;

//// StateType
StateType StateType::sInstance;
const ObjectType::MethodEntry StateType::sMethodTable[] =
{
  METHOD( Set ), METHOD( Get ),
  METHOD( Insert ), { "Add", &Insert },
  METHOD( List ),
  METHOD( Exists ), { "Is", &Exists },
  END
};

bool
StateType::Set( CommandInterpreter& inInterpreter )
{
  string name;
  State::ValueType value = 0;
  ostringstream oss;
  {
    Lock<StateMachine> lock( inInterpreter.StateMachine() );
    State& state = GetState( inInterpreter );
    name = state.Name();
    value = ::atoi( inInterpreter.GetToken().c_str() );
    if( !inInterpreter.StateMachine().SetStateValue( name.c_str(), value ) )
      throw bciexception_( "Could not set state " << name << " to " << value );
    // StateMachine::SetStateValue() does not set the value of the state object in its state list.
    state.SetValue( value );
    oss << state;
  }
  inInterpreter.StateMachine().ExecuteCallback( BCI_OnState, oss.str().c_str() );
  inInterpreter.Log() << "Set state " << name << " to " << value;
  return true;
}

bool
StateType::Get( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  const State& state = GetState( inInterpreter ); // GetState() tests for existence of the state.
  inInterpreter.Out() <<  inInterpreter.StateMachine().GetStateValue( state.Name().c_str() );
  return true;
}

bool
StateType::Insert( CommandInterpreter& inInterpreter )
{
  string name = inInterpreter.GetToken();
  string line = inInterpreter.GetRemainder(),
         stateline = name + " " + line + " 0 0";
  State state;
  istringstream iss( stateline );
  if( !( iss >> state ) )
    throw bciexception_( "Invalid state definition" );
  {
    Lock<StateMachine> lock( inInterpreter.StateMachine() );
    switch( inInterpreter.StateMachine().SystemState() )
    {
      case StateMachine::Idle:
      case StateMachine::WaitingForConnection:
      case StateMachine::Publishing:
      case StateMachine::Information:
        break;
      default:
        throw bciexception_( "Could not add state " << name << " to list after information phase" );
    }
    inInterpreter.StateMachine().States().Add( state );
  }
  inInterpreter.StateMachine().ExecuteCallback( BCI_OnState, stateline.c_str() );
  inInterpreter.Log() << "Added state " << name << " to list";
  return true;
}

bool
StateType::List( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  inInterpreter.Out() << GetState( inInterpreter );
  return true;
}

bool
StateType::Exists( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  bool exists = inInterpreter.StateMachine().States().Exists( inInterpreter.GetToken() );
  inInterpreter.Out() << exists ? "true" : "false";
  return true;
}


State&
StateType::GetState( CommandInterpreter& inInterpreter )
{
  string name = inInterpreter.GetToken();
  if( name.empty() )
    throw bciexception_( "Expected a state name" );
  if( !inInterpreter.StateMachine().States().Exists( name ) )
    throw bciexception_( "State " << name << " does not exist" );
  return inInterpreter.StateMachine().States()[name];
}

//// StatesType
StatesType StatesType::sInstance;
const ObjectType::MethodEntry StatesType::sMethodTable[] =
{
  METHOD( List ), METHOD( Clear ),
  END
};

bool
StatesType::List( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  string pattern = inInterpreter.GetOptionalRemainder();
  if( pattern.empty() )
    pattern = "*";
  const StateList& states = inInterpreter.StateMachine().States();
  for( int i = 0; i < states.Size(); ++i )
    if( WildcardMatch( pattern, states[i].Name(), false ) )
      inInterpreter.Out() << states[i] << '\n';
  return true;
}

bool
StatesType::Clear( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  if( inInterpreter.StateMachine().SystemState() != StateMachine::Idle )
    throw bciexception_( "Must be in idle state to clear states" );
  inInterpreter.StateMachine().States().Clear();
  return true;
}

