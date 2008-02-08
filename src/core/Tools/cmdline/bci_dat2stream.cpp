////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////
#include "bci_tool.h"
#include "Param.h"
#include "ParamList.h"
#include "State.h"
#include "StateList.h"
#include "StateVector.h"
#include "GenericSignal.h"
#include "MessageHandler.h"
#include "Version.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cassert>

using namespace std;

string ToolInfo[] =
{
  "bci_dat2stream",
  BCI2000_VERSION,
  "Convert a BCI2000 data file into a BCI2000 stream.",
  "Reads a BCI2000 data file (*.dat) compliant stream from "
    "standard input and writes it to the standard output as a BCI2000 "
    "compliant binary stream.",
  "-t,       --transmit-{spd}\tSelect States, Parameters,",
  "                          \tand Data for transmission",
  "-r,       --raw           \tTransmit uncalibrated data",
  ""
};

ToolResult ToolInit()
{
  return noError;
}

ToolResult ToolMain( const OptionSet& options, istream& in, ostream& out )
{
  ToolResult result = noError;
  string transmissionList = options.getopt( "-t|-T|--transmit", "-spd" );
  bool transmitStates = ( transmissionList.find_first_of( "sS" ) != string::npos ),
       transmitParameters = ( transmissionList.find_first_of( "pP" ) != string::npos ),
       transmitData = ( transmissionList.find_first_of( "dD" ) != string::npos ),
       calibrateData = ( options.find( "-r" ) == options.end()
                         && options.find( "-R" ) == options.end()
                         && options.find( "--raw" ) == options.end() );

  // Read the BCI2000 header.
  string token;
  int headerLength,
      sourceCh,
      stateVectorLength;
  SignalType dataFormat;
  StateList states;
  enum { v10, v11 } fileFormatVersion = v10;

  bool legalInput = in >> token;
  if( legalInput && token == "BCI2000V=" )
  {
    legalInput = legalInput &&
      in >> token;
    if( token == "1.1" )
      fileFormatVersion = v11;
    else
      legalInput = false;
    legalInput = legalInput &&
      in >> token;
  }
  legalInput &=
    token == "HeaderLen=" && in >> headerLength &&
    in >> token && token == "SourceCh=" && in >> sourceCh &&
    in >> token && token == "StatevectorLen=" && in >> stateVectorLength;
  switch( fileFormatVersion )
  {
    case v10:
      dataFormat = SignalType::int16;
      break;
    case v11:
      legalInput &=
        in >> token && token == "DataFormat=" && in >> dataFormat;
      break;
    default:
      assert( false );
  }
  legalInput &=
    getline( in >> ws, token, ']' ) >> ws && token == "[ State Vector Definition ";
  while( legalInput && in.peek() != '[' && getline( in, token ) )
  {
    istringstream is( token );
    State state;
    if( is >> state )
    {
      states.Delete( state.Name() );
      states.Add( state );
    }
    legalInput = legalInput && is;
  }

  if( transmitStates )
  { // Transmit states ordered by name, i.e. independently of their order in the file.
    vector<string> stateNames;
    for( int i = 0; i < states.Size(); ++i )
      stateNames.push_back( states[ i ].Name() );
    sort( stateNames.begin(), stateNames.end(), State::NameCmp() );
    for( size_t i = 0; i < stateNames.size(); ++i )
      MessageHandler::PutMessage( out, states[ stateNames[ i ] ] );
  }

  legalInput &=
    getline( in >> ws, token, ']' ) >> ws && token == "[ Parameter Definition ";

  ParamList parameters;
  while( legalInput && getline( in, token ) &&  token.length() > 1 )
  {
    istringstream is( token );
    Param param;
    if( is >> param )
      parameters[ param.Name() ] = param;
    legalInput = legalInput && is;
    if( transmitParameters )
      MessageHandler::PutMessage( out, param );
  }
  int sampleBlockSize = atoi( parameters[ "SampleBlockSize" ].Value().c_str() );
  legalInput &= ( sampleBlockSize > 0 );
  if( !legalInput )
  {
    cerr << "Illegal header format or content" << endl;
    return illegalInput;
  }

  SignalProperties outputProperties( sourceCh, sampleBlockSize, dataFormat );
  if( transmitData || transmitStates )
  {
    StateVector statevector( states, true );
    if( statevector.Length() != stateVectorLength )
    {
      cerr << "Statevector's length differs from StateVectorLen field" << endl;
      return illegalInput;
    }
    if( calibrateData && parameters.Exists( "SamplingRate" ) )
    {
      outputProperties
        .ElementUnit().SetOffset( 0 )
        .SetGain( 1.0 / atof( parameters[ "SamplingRate" ].Value().c_str() ) )
        .SetSymbol( "s" );
    }
    vector<float> offsets( sourceCh, 0 ),
                  gains( sourceCh, 1 );
    if( calibrateData && parameters.Exists( "SourceChOffset" ) )
    {
      const Param& sourceChOffset = parameters[ "SourceChOffset" ];
      for( int ch = 0; ch < min( sourceCh, sourceChOffset.NumValues() ); ++ch )
        offsets[ ch ] = atof( sourceChOffset.Value( ch ).c_str() );
    }
    if( calibrateData && parameters.Exists( "SourceChGain" ) )
    {
      const Param& sourceChGain = parameters[ "SourceChGain" ];
      for( int ch = 0; ch < min( sourceCh, sourceChGain.NumValues() ); ++ch )
        gains[ ch ] = atof( sourceChGain.Value( ch ).c_str() );
      outputProperties
        .ValueUnit().SetOffset( 0 )
        .SetGain( 1e-6 )
        .SetSymbol( "V" );
    }
    if( parameters.Exists( "ChannelNames" ) && parameters[ "ChannelNames" ].NumValues() > 0 )
    {
      LabelIndex& outputLabels = outputProperties.ChannelLabels();
      for( int i = 0; i < min( outputProperties.Channels(), parameters[ "ChannelNames" ].NumValues() ); ++i )
        outputLabels[ i ] = parameters[ "ChannelNames" ].Value( i ).c_str();
    }
    if( transmitData )
      MessageHandler::PutMessage( out, outputProperties );

    int curSample = 0;
    GenericSignal outputSignal( outputProperties );
    while( in && in.peek() != EOF )
    {
      for( int i = 0; i < sourceCh; ++i )
        outputSignal.ReadValueBinary( in, i, curSample );
      statevector.ReadBinary( in );

      if( ++curSample == sampleBlockSize )
      {
        curSample = 0;
        if( transmitStates )
        {
          MessageHandler::PutMessage( out, statevector );
        }
        if( transmitData )
        {
          if( calibrateData )
            for( int i = 0; i < sourceCh; ++i )
              for( int j = 0; j < sampleBlockSize; ++j )
                outputSignal( i, j )
                  = ( outputSignal( i, j ) - offsets[ i ] ) * gains[ i ];
          // Send the data.
          MessageHandler::PutMessage( out, outputSignal );
        }
      }
    }
    if( curSample != 0 )
    {
      cerr << "Non-integer number of data blocks in input" << endl;
      result = illegalInput;
    }
  }
  return result;
}
