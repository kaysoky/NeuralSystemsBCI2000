///////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A command line utility to read RDAClient source related parameters
//   from the BrainVisionRecorder, and write them out in BCI2000 parameter format.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include <set>
#include "Param.h"
#include "ParamList.h"
#include "RDAQueue.h"

#define PROGRAM "getparams"

using namespace std;

const char* usage = PROGRAM " is a command line utility that allows\n"
                    "to obtain appropriate source module parameters\n"
                    "from a host running BrainAmp's VisionRecorder.\n"
                    "On the target host, start the VisionRecorder\n"
                    "program, check that RDA is enabled under\n"
                    "Configuration->Preferences, and click the monitor (eye)\n"
                    "button before running " PROGRAM " with the host's\n"
                    "IP address as the only parameter (when omitted, this\n"
                    "defaults to \"localhost\").\n"
                    "To direct the output into a file that can later be loaded\n"
                    "into the operator module's configuration dialog, append\n"
                    "e.g. \">rda.prm\" to the command.";

template<typename T> string str( T t )
{
  ostringstream oss;
  oss << t;
  return oss.str();
}

enum
{
  noError = 0,
  generalError = -1,
};

int main( int argc, char** argv )
{
  const char* hostname = "localhost";
  if( argc > 1 )
    hostname = argv[ 1 ];

  if( hostname[ 0 ] == '-' )
  {
    cout << usage << endl;
    return noError;
  }

  RDAQueue q;
  q.open( hostname );
  if( !q )
  {
    cerr << "Could not open connection to host \"" << hostname << "\".\n\n"
         << "Hint: " << usage << endl;
    return generalError;
  }

  ParamList paramlist;
  const char* params[] =
  {
    "RDA string HostName= ",
    "RDA int SourceCh= ",
    "RDA floatlist SourceChOffset= 0 ",
    "RDA floatlist SourceChGain= 0 ",
    "RDA float SamplingRate= 1 ",
    "RDA intlist TransmitChList= 0 ",
    "RDA matrix SpatialFilter= 0 1 ",
    "RDA int SourceMax= 300muV ",
    "RDA int SourceMin= -300muV ",
  };
  for( size_t i = 0; i < sizeof( params ) / sizeof( *params ); ++i )
    paramlist.Add( ( string( params[ i ] ) + " // getparams " + hostname ).c_str() );

  paramlist[ "HostName" ].Value() = hostname;
  int numInputChannels = q.info().numChannels + 1;
  paramlist[ "SourceCh" ].Value() = str( numInputChannels );
  paramlist[ "SourceChOffset" ].SetNumValues( numInputChannels );
  paramlist[ "SourceChGain" ].SetNumValues( numInputChannels );
  paramlist[ "TransmitChList" ].SetNumValues( numInputChannels - 1 );
  paramlist[ "SpatialFilter" ].SetDimensions( numInputChannels - 1, numInputChannels - 1 );

  for( int i = 0; i < numInputChannels - 1; ++i )
  {
    paramlist[ "SourceChOffset" ].Value( i ) = "0";
    paramlist[ "SourceChGain" ].Value( i ) = str( q.info().channelResolutions[ i ] );
    paramlist[ "TransmitChList" ].Value( i ) = str( i + 1 );
    paramlist[ "SpatialFilter" ].Value( i, i ) = "1";
  }
  paramlist[ "SourceChOffset" ].Value( numInputChannels - 1 ) = "0";
  paramlist[ "SourceChGain" ].Value( numInputChannels - 1 ) = "1";
  paramlist[ "SamplingRate" ].Value() = str( 1e6 / q.info().samplingInterval );

  cout << paramlist;

  return noError;
}

