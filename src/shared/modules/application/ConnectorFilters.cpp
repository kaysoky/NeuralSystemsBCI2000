////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
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
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ConnectorFilters.h"

#define SECTION "Connector"

using namespace std;

RegisterFilter( ConnectorInput,  2.9999 ); // Place the input filter 
                                           // immediately before the task
                                           // filter.
RegisterFilter( ConnectorOutput, 3.9999 ); // Place the output filter
                                           // last in the application module.

ConnectorInput::ConnectorInput()
: mConnectorInputAddress( "" ),
  mInputFilter( "" ),
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
  string connectorInputAddress = Parameter( "ConnectorInputAddress" );
  if( connectorInputAddress != "" )
  {
    receiving_udpsocket preflightSocket( connectorInputAddress.c_str() );
    if( !preflightSocket.is_open() )
      bcierr << "Could not connect to " << connectorInputAddress << endl;
  }
  Parameter( "ConnectorInputFilter" );
  
  // Pre-flight access each state in the list.
  for( int state = 0; state < States->Size(); ++state )
    State( ( *States )[ state ].Name() );

  outSignalProperties = inSignalProperties;
}

void
ConnectorInput::Initialize( const SignalProperties&, const SignalProperties& )
{
  mConnectorInputAddress = Parameter( "ConnectorInputAddress" );
  ParamRef ConnectorInputFilter = Parameter( "ConnectorInputFilter" );
  if( mConnectorInputAddress != "" )
  {
    mInputFilter = "";
    mAllowAny = ( ConnectorInputFilter->NumValues() > 0 && ConnectorInputFilter( 0 ) == "*" );
    if( !mAllowAny )
      for( int i = 0; i < ConnectorInputFilter->NumValues(); ++i )
      {
        mInputFilter += ConnectorInputFilter( i );
        mInputFilter += ' ';
      }
  }
}

void
ConnectorInput::StartRun()
{
  if( mConnectorInputAddress != "" )
  {
    mSocket.open( mConnectorInputAddress.c_str() );
    mConnection.open( mSocket );
    if( !mConnection.is_open() )
      bcierr << "Could not connect to " << mConnectorInputAddress << endl;
  }
}

void
ConnectorInput::StopRun()
{
  mConnection.close();
  mConnection.clear();
  mSocket.close();
}

void
ConnectorInput::Process( const GenericSignal& Input, GenericSignal& Output )
{
  Output = Input;
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
        int channel = 0,
            element = 0;
        if( !( iss >> ignore >> channel >> ignore >> element >> ignore ) )
          bciout << "Incorrect Signal index syntax: " << name << endl;
        else if( channel >= Input.Channels() || element >= Input.Elements() )
          bciout << "Received signal index out-of-bounds: " << name << endl;
        else
          Output( channel, element ) = value;
      }
      else
      {
        if( !States->Exists( name ) )
          bciout << "Ignoring value for non-existent " << name << " state" << endl;
        else
          State( name.c_str() ) = value;
      }
    }
  }
}


// ConnectorOutput
ConnectorOutput::ConnectorOutput()
: mConnectorOutputAddress( "" )
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
  // Pre-flight access each state in the list.
  for( int state = 0; state < States->Size(); ++state )
    State( ( *States )[ state ].Name() );

  outSignalProperties = inSignalProperties;
}

void
ConnectorOutput::Initialize( const SignalProperties&, const SignalProperties& )
{
  mConnectorOutputAddress = Parameter( "ConnectorOutputAddress" );
}

void
ConnectorOutput::StartRun()
{
  if( mConnectorOutputAddress != "" )
  {
    mSocket.open( mConnectorOutputAddress.c_str() );
    mConnection.open( mSocket );
    if( !mConnection.is_open() )
      bcierr << "Could not connect to " << mConnectorOutputAddress << endl;
  }
}

void
ConnectorOutput::StopRun()
{
  mConnection.close();
  mConnection.clear();
  mSocket.close();
}

void
ConnectorOutput::Process( const GenericSignal& Input, GenericSignal& Output )
{
  Output = Input;
  for( int state = 0; state < States->Size(); ++state )
  {
    string stateName = ( *States )[ state ].Name();
    mConnection << stateName << ' '
                << State( stateName.c_str() ) << endl;
  }
  for( int channel = 0; channel < Input.Channels(); ++channel )
    for( int element = 0; element < Input.Elements(); ++element )
      mConnection << "Signal(" << channel << "," << element << ") "
                  << Input( channel, element ) << endl;
}
