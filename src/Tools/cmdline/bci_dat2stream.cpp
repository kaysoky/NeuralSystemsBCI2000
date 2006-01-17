////////////////////////////////////////////////////////////////////
// $Id$
// File:        bci_dat2stream.cpp
// Date:        Jul 9, 2003
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: See the ToolInfo definition below.
// $Log$
// Revision 1.9  2006/01/17 17:39:44  mellinger
// Fixed list of project files.
//
// Revision 1.8  2006/01/12 20:37:14  mellinger
// Adaptation to latest revision of parameter and state related class interfaces.
//
////////////////////////////////////////////////////////////////////
#include "bci_tool.h"
#include "shared/defines.h"
#include "shared/UParameter.h"
#include "shared/UState.h"
#include "shared/UGenericSignal.h"
#include "shared/MessageHandler.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>

using namespace std;

string ToolInfo[] =
{
  "bci_dat2stream",
  "$Revision$, compiled " __DATE__,
  "Convert a BCI2000 data file into a BCI2000 stream.",
  "Reads a BCI2000 data file (*.dat) compliant stream from "
    "standard input and writes it to the standard output as a BCI2000 "
    "compliant binary stream.",
  "-t,       --transmit-{spd}\tSelect States, Parameters,",
  "                          \tand Data for transmission",
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
  string token;
  int headerLength,
      sourceCh,
      stateVectorLength;
  SignalType dataFormat;
  STATELIST states;
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
    STATE state;
    if( is >> state )
    {
      states.Delete( state.GetName() );
      states.Add( state );
    }
    legalInput = legalInput && is;
    if( transmitStates )
      MessageHandler::PutMessage( out, state );
  }
  legalInput &=
    getline( in >> ws, token, ']' ) >> ws && token == "[ Parameter Definition ";

  PARAMLIST parameters;
  while( legalInput && getline( in, token ) &&  token.length() > 1 )
  {
    istringstream is( token );
    PARAM param;
    if( is >> param )
      parameters[ param.GetName() ] = param;
    legalInput = legalInput && is;
    if( transmitParameters )
      MessageHandler::PutMessage( out, param );
  }
  int sampleBlockSize = atoi( parameters[ "SampleBlockSize" ].GetValue() );
  legalInput &= ( sampleBlockSize > 0 );
  if( !legalInput )
  {
    cerr << "Illegal header format or content" << endl;
    return illegalInput;
  }

  if( transmitData || transmitStates )
  {
    STATEVECTOR statevector( states, true );
    if( statevector.Length() != stateVectorLength )
    {
      cerr << "Statevector's length differs from StateVectorLen field" << endl;
      return illegalInput;
    }
    int curSample = 0;
    GenericSignal outputSignal( sourceCh, sampleBlockSize, dataFormat );
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
