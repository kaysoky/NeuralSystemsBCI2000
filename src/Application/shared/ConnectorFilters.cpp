////////////////////////////////////////////////////////////////////////////////
//
// File:   ConnectorFilters.cpp
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date:   Jun 14, 2005
//
// Description: A pair of filters that send/receive states and signals over a
//         UDP connection.
//
//         Data transmission is done via UDP socket connections.
//         Messages consist in a name and a value, separated by white space
//         and terminated with a single newline '\n' character.
//
//         Names may identify
//         -- BCI2000 states by name, and are then followed
//            by an integer value in decimal ASCII representation;
//         -- Signal elements in the form Signal(<channel>,<element>), and are
//            then followed by a float value in decimal ASCII representation.
//
//         Examples:
//           Running 0
//           ResultCode 2
//           Signal(1,2) 1e-8
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ConnectorFilters.h"

#define SECTION "Connector"

using namespace std;

RegisterFilter( ConnectorInput,  2.9999 ); // Place the input filter
                                           // immediately before the task
                                           // filter.
RegisterFilter( ConnectorOutput, 3.0001 ); // Place the output filter
                                           // immediately after the task
                                           // filter.

ConnectorInput::ConnectorInput()
: mInputFilter( "" ),
  mAllowAny( false )
{
  BEGIN_PARAMETER_DEFINITIONS
    SECTION " list ConnectorInputFilter= 0 "
      "% % % // list of state names or signal elements to allow, "
      "\"*\" for any, signal elements as in \"Signal(1,0)\"",
    SECTION " string ConnectorInputAddress= % "
      "localhost:20320 % % // IP address/port to read from, e.g. localhost:20320",
  END_PARAMETER_DEFINITIONS
}

ConnectorInput::~ConnectorInput()
{
}

void
ConnectorInput::Preflight( const SignalProperties& inSignalProperties,
                                 SignalProperties& outSignalProperties ) const
{
  string connectorInputAddress( Parameter( "ConnectorInputAddress" ) );
  if( connectorInputAddress != "" )
  {
    receiving_udpsocket preflightSocket( connectorInputAddress.c_str() );
    if( !preflightSocket.is_open() )
      bcierr << "Could not connect to " << connectorInputAddress << endl;
  }
  outSignalProperties = inSignalProperties;
}

void
ConnectorInput::Initialize()
{
  mConnection.close();
  mConnection.clear();
  mSocket.close();
  string connectorInputAddress( Parameter( "ConnectorInputAddress" ) );
  if( connectorInputAddress != "" )
  {
    mSocket.open( connectorInputAddress.c_str() );
    mConnection.open( mSocket );
    if( !mConnection.is_open() )
      bcierr << "Could not connect to " << connectorInputAddress << endl;

    mInputFilter = "";
    mAllowAny = ( string( Parameter( "ConnectorInputFilter", 0 ) ) == "*" );
    if( !mAllowAny )
      for( size_t i = 0; i < Parameter( "ConnectorInputFilter" )->GetNumValues(); ++i )
      {
        mInputFilter += string( Parameter( "ConnectorInputFilter", i ) );
        mInputFilter += ' ';
      }
  }
}

void
ConnectorInput::Process( const GenericSignal* input, GenericSignal* output )
{
  *output = *input;
  while( mConnection.rdbuf()->in_avail() )
  {
    string name;
    float  value;
    mConnection >> name >> value;
    mConnection.ignore();
    if( !mConnection )
    {
      bciout << "Unexpected input" << endl;
      mConnection.clear();
    }
    else if( mAllowAny || mInputFilter.find( name + ' ' ) != string::npos )
    {
      if( name.find( "Signal" ) == 0 )
      {
        istringstream iss( name.substr( name.find( '(' ) ) );
        char ignore;
        size_t channel = 0,
               element = 0;
        if( !( iss >> ignore >> channel >> ignore >> element >> ignore ) )
          bciout << "Incorrect Signal index syntax: " << name << endl;
        else if( channel >= input->Channels() || element >= input->Elements() )
          bciout << "Received signal index out-of-bounds: " << name << endl;
        else
          ( *output )( channel, element ) = value;
      }
      else
      {
        if( !States->GetStatePtr( name.c_str() ) )
          bciout << "Ignoring value for non-existent " << name << " state" << endl;
        else
          State( name.c_str() ) = value;
      }
    }
  }
}

ConnectorOutput::ConnectorOutput()
{
  BEGIN_PARAMETER_DEFINITIONS
    SECTION " string ConnectorOutputAddress= % "
      "localhost:20320 % % // IP address/port to write to, e.g. localhost:20320",
  END_PARAMETER_DEFINITIONS
}

ConnectorOutput::~ConnectorOutput()
{
}

void
ConnectorOutput::Preflight( const SignalProperties& inSignalProperties,
                                 SignalProperties& outSignalProperties ) const
{
  string connectorOutputAddress( Parameter( "ConnectorOutputAddress" ) );
  if( connectorOutputAddress != "" )
  {
    sending_udpsocket preflightSocket( connectorOutputAddress.c_str() );
    if( !preflightSocket.is_open() )
      bcierr << "Could not connect to " << connectorOutputAddress << endl;
  }
  outSignalProperties = inSignalProperties;
}

void
ConnectorOutput::Initialize()
{
  mConnection.close();
  mConnection.clear();
  mSocket.close();
  string connectorOutputAddress( Parameter( "ConnectorOutputAddress" ) );
  if( connectorOutputAddress != "" )
  {
    mSocket.open( connectorOutputAddress.c_str() );
    mConnection.open( mSocket );
    if( !mConnection.is_open() )
      bcierr << "Could not connect to " << connectorOutputAddress << endl;
  }
}

void
ConnectorOutput::Process( const GenericSignal* input, GenericSignal* output )
{
  *output = *input;
  for( int state = 0; state < States->GetNumStates(); ++state )
  {
    string stateName = States->GetStatePtr( state )->GetName();
    mConnection << stateName << ' '
                << State( stateName.c_str() ) << endl;
  }
  for( size_t channel = 0; channel < input->Channels(); ++channel )
    for( size_t element = 0; element < input->Elements(); ++element )
      mConnection << "Signal(" << channel << "," << element << ") "
                  << ( *input )( channel, element ) << endl;
}

