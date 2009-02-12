////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Date:   Jul 28, 2004
// Description: A filter that watches a given state for changes, and simulates
//         a key press for the respective number key.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
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
KeystrokeFilter::Preflight( const SignalProperties& inSignalProperties,
                                  SignalProperties& outSignalProperties ) const
{
  string keystrokeStateName = Parameter( "KeystrokeStateName" );
  if( keystrokeStateName != "" )
    State( keystrokeStateName.c_str() );
  outSignalProperties = inSignalProperties;
}

void
KeystrokeFilter::Initialize( const SignalProperties&, const SignalProperties& )
{
  mKeystrokeStateName = Parameter( "KeystrokeStateName" );
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
    short currentStateValue = State( mKeystrokeStateName.c_str() );
    if( currentStateValue != mPreviousStateValue )
      SendKeystroke( currentStateValue );
    mPreviousStateValue = currentStateValue;
  }
  Output = Input;
}

void KeystrokeFilter::SendKeystroke( short s )
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
