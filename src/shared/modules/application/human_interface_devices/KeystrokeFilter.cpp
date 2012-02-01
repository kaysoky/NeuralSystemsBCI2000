////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Date:   Jul 28, 2004
// Description: A filter that watches a given state for changes, and simulates
//         a key press for the respective number key.
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

#include "KeystrokeFilter.h"

#include <string>
#include <windows.h>

using namespace std;

RegisterFilter( KeystrokeFilter, 3.1 ); // Place it after the task filter.

KeystrokeFilter::KeystrokeFilter()
: mPreviousStateValue( 0 ),
  mKeystrokeStateName( "" )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Application:Human%20Interface%20Devices string KeystrokeStateName= % "
      "// State to be translated into keystrokes, empty for off",
  END_PARAMETER_DEFINITIONS
}

KeystrokeFilter::~KeystrokeFilter()
{
}

void
KeystrokeFilter::Preflight( const SignalProperties& Input,
                                  SignalProperties& Output ) const
{
  string keystrokeStateName = Parameter( "KeystrokeStateName" );
  if( keystrokeStateName != "" )
    State( keystrokeStateName.c_str() );
  Output = Input;
}

void
KeystrokeFilter::Initialize( const SignalProperties&, const SignalProperties& )
{
  mKeystrokeStateName = string( Parameter( "KeystrokeStateName" ) );
  if( mKeystrokeStateName != "" )
  {
    mPreviousStateValue = State( mKeystrokeStateName.c_str() );
    SendKeystroke( mPreviousStateValue );
  }
}

void
KeystrokeFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  if( mKeystrokeStateName != "" )
  {
    State::ValueType currentStateValue = State( mKeystrokeStateName.c_str() );
    if( currentStateValue != mPreviousStateValue )
      SendKeystroke( currentStateValue );
    mPreviousStateValue = currentStateValue;
  }
  Output = Input;
}

void KeystrokeFilter::SendKeystroke( State::ValueType s )
{
  const short unicodeChars[] =
  {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F',
  };
  const int numChars = sizeof( unicodeChars ) / sizeof( *unicodeChars );
  if( s < 0 || s >= numChars )
  {
    bciout << "Only state values between 0 and "
           << numChars - 1
           << " may be sent as keystrokes"
           << endl;
    return;
  }

  KEYBDINPUT keyEvents[] =
  {
    { 0, unicodeChars[ s ], KEYEVENTF_UNICODE, 0, 0 },
    { 0, unicodeChars[ s ], KEYEVENTF_UNICODE | KEYEVENTF_KEYUP, 0, 0 },
  };
  const int numInputs = sizeof( keyEvents ) / sizeof( *keyEvents );
  INPUT inputs[ numInputs ];
  for( int i = 0; i < numInputs; ++i )
  {
    inputs[ i ].type = INPUT_KEYBOARD;
    inputs[ i ].ki = keyEvents[ i ];
  }
  if( ::SendInput( numInputs, inputs, sizeof( *inputs ) ) != numInputs )
    bciout << "Could not send keystroke for state value '"
           << s << "'" << endl;
}
