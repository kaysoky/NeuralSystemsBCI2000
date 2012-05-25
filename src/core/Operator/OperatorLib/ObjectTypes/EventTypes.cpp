////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Event-related object types for the script interpreter.
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

#include "EventTypes.h"
#include "StateTypes.h"
#include "CommandInterpreter.h"
#include "StateMachine.h"
#include "Lockable.h"
#include "WildcardMatch.h"
#include "BCIException.h"
#include <cstdlib>

using namespace std;
using namespace bci;
using namespace Interpreter;

//// EventType
EventType EventType::sInstance;
const ObjectType::MethodEntry EventType::sMethodTable[] =
{
  METHOD( Set ), METHOD( Get ),
  METHOD( Insert ), { "Add", &Insert },
  METHOD( List ),
  METHOD( Exists ), { "Is", &Exists },
  END
};

bool
EventType::Set( CommandInterpreter& inInterpreter )
{
  string name;
  State::ValueType value = 0;
  {
    Lock<StateMachine> lock( inInterpreter.StateMachine() );
    State& event = GetEvent( inInterpreter );
    name = event.Name();
    value = ::atoi( inInterpreter.GetToken().c_str() );
    if( ( value & ~( ( 1LL << event.Length() ) - 1LL ) ) != 0 )
      throw bciexception_( "Event value " << value << " out of range" );
    if( !inInterpreter.StateMachine().SetEvent( name.c_str(), value ) )
      throw bciexception_( "Could not set event " << name << " to " << value );
  }
  return true;
}

bool
EventType::Get( CommandInterpreter& inInterpreter )
{
  // All events appear as states in the state vector, and have their values stored there.
  return StateType::Get( inInterpreter );
}

bool
EventType::Insert( CommandInterpreter& inInterpreter )
{
  string name = inInterpreter.GetToken();
  string line = inInterpreter.GetRemainder(),
         eventline = name + " " + line + " 0 0";
  State event;
  istringstream iss( eventline );
  if( !( iss >> event ) )
    throw bciexception_( "Invalid event definition: " << eventline );
  {
    Lock<StateMachine> lock( inInterpreter.StateMachine() );
    if( inInterpreter.StateMachine().SystemState() != StateMachine::Idle )
      throw bciexception_( "Could not add event " << name << " to list outside idle state" );
    inInterpreter.StateMachine().Events().Add( event );
  }
  return true;
}

bool
EventType::List( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  inInterpreter.Out() << GetEvent( inInterpreter );
  return true;
}

bool
EventType::Exists( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  bool exists = inInterpreter.StateMachine().Events().Exists( inInterpreter.GetToken() );
  inInterpreter.Out() << exists ? "true" : "false";
  return true;
}


State&
EventType::GetEvent( CommandInterpreter& inInterpreter )
{
  string name = inInterpreter.GetToken();
  if( name.empty() )
    throw bciexception_( "Expected an event name" );
  if( !inInterpreter.StateMachine().Events().Exists( name ) )
    throw bciexception_( "Event " << name << " does not exist" );
  return inInterpreter.StateMachine().Events()[name];
}

//// EventsType
EventsType EventsType::sInstance;
const ObjectType::MethodEntry EventsType::sMethodTable[] =
{
  METHOD( List ), METHOD( Clear ),
  END
};

bool
EventsType::List( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  string pattern = inInterpreter.GetOptionalRemainder();
  if( pattern.empty() )
    pattern = "*";
  const StateList& events = inInterpreter.StateMachine().Events();
  for( int i = 0; i < events.Size(); ++i )
    if( WildcardMatch( pattern, events[i].Name(), false ) )
      inInterpreter.Out() << events[i] << '\n';
  return true;
}

bool
EventsType::Clear( CommandInterpreter& inInterpreter )
{
  Lock<StateMachine> lock( inInterpreter.StateMachine() );
  if( inInterpreter.StateMachine().SystemState() != StateMachine::Idle )
    throw bciexception_( "Must be in idle state to clear events" );
  inInterpreter.StateMachine().Events().Clear();
  return true;
}

