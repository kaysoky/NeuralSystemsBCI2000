////////////////////////////////////////////////////////////////////
// File:        bci_dat2stream.cpp
// Date:        Jul 9, 2003
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
////////////////////////////////////////////////////////////////////
#include "bci_tool.h"
#include "bci2000_types.h"
#include "shared/UParameter.h"
#include "shared/UGenericSignal.h"
#include <iostream>
#include <string>

using namespace std;

string ToolInfo[] =
{
  "bci_dat2stream",
  "0.1.0, compiled " __DATE__,
  "Convert a BCI2000 data file into a BCI2000 stream.",
  "Reads a BCI2000 data file (*.dat) compliant stream from "
    "standard input and writes it to the standard output as a BCI2000 "
    "compliant binary stream.",
  "-t, --transmit-{spd}\tSelect States, Parameters, and Data for transmission",
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
       transmitData = ( transmissionList.find_first_of( "dD" ) != string::npos );

  // Read the BCI2000 header.
  // The format given here does not match the specification but is what is actually
  // found in .dat files written by the BCI2000 source module.
  string token;
  int headerLength,
      sourceCh,
      stateVectorLength;

  bool legalInput =
  in >> token && token == "HeaderLen=" && in >> headerLength &&
  in >> token && token == "SourceCh=" && in >> sourceCh &&
  in >> token && token == "StatevectorLen=" && in >> stateVectorLength &&
  getline( in >> ws, token, ']' ) >> ws && token == "[ State Vector Definition ";
  while( legalInput && in.peek() != '[' && getline( in, token ) )
  {
    token += '\n';
    int length = token.length();
    legalInput &= ( length < ( 1 << 16 ) );
    if( transmitStates )
    {
      char stateHeader[] =
      {
        state,
        none,
        length & 0xff,
        ( length >> 8 ) & 0xff
      };
      out.write( stateHeader, sizeof( stateHeader ) );
      out.write( token.data(), length );
    }
  }
  legalInput &=
    getline( in >> ws, token, ']' ) >> ws && token == "[ Parameter Definition ";
  PARAMLIST parameters;
  while( legalInput && getline( in, token ) &&  token.length() > 1 )
  {
    token += '\n';
    parameters.AddParameter2List( token.c_str() );
    int length = token.length();
    legalInput &= ( length < ( 1 << 16 ) );
    if( transmitParameters )
    {
      char parameterHeader[] =
      {
        parameter,
        none,
        length & 0xff,
        ( length >> 8 ) & 0xff
      };
      out.write( parameterHeader, sizeof( parameterHeader ) );
      out.write( token.data(), length );
    }
  }
  int sampleBlockSize = atoi( parameters[ "SampleBlockSize" ].GetValue() );
  legalInput &= ( sampleBlockSize > 0 );
  if( !legalInput )
  {
    cerr << "Illegal header format or content" << endl;
    return illegalInput;
  }

  int bufferSize = sourceCh * 2 + stateVectorLength;
  char* dataBuffer = new char[ bufferSize ];
  int curSample = 0;
  GenericSignal outputSignal( sourceCh, sampleBlockSize );
  while( legalInput && in.peek() != EOF && in.read( dataBuffer, bufferSize ) )
  {
	for( int i = 0; i < sourceCh; ++i )
	{
	  sint16 value = dataBuffer[ 2 * i + 1 ] << 8 | dataBuffer[ 2 * i ];
	  outputSignal( i, curSample ) = value;
	}
	if( ++curSample == sampleBlockSize )
	{
	  curSample = 0;
	  if( transmitData )
	  {
        // Send the data.
        ostringstream oss;
        outputSignal.WriteBinary( oss );
        int length = oss.str().length() + 1;
        char signalHeader[] =
        {
          data,
          signal,
          length & 0xff,
          ( length >> 8 ) & 0xff,
          none
        };
        out.write( signalHeader, sizeof( signalHeader ) );
        out.write( oss.str().data(), oss.str().length() );
      }
      if( transmitStates )
      {
        char statevectorHeader[] =
        {
          state_vector,
          none,
          stateVectorLength & 0xff,
          ( stateVectorLength >> 8 ) & 0xff
        };
        out.write( statevectorHeader, sizeof( statevectorHeader ) );
        out.write( dataBuffer + sourceCh * 2, stateVectorLength );
      }
    }
  }
  delete[] dataBuffer;
  if( curSample != 0 )
  {
    cerr << "Non-integer number of data blocks in input" << endl;
    result = illegalInput;
  }

  if( !out )
  {
    cerr << "Error writing to standard output" << endl;
    result = genericError;
  }
  if( !in )
  {
    cerr << "Illegal data format" << endl;
    result = illegalInput;
  }
  return result;
}
