///////////////////////////////////////////////////////////////////////////////////
//
// File: RDAClient/getparams.cpp
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A command line utility to get RDAClient source related parameters
//              in BCI2000 parameter format.
//
///////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include <set>
#include "../../../shared/UParameter.h"
#include "../RDAQueue.h"

#define PROGRAM "getparams"

using namespace std;

const char* usage = PROGRAM " is a command line utility that allows\n"
                    "one to obtain appropriate source module parameters\n"
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

  PARAMLIST paramlist;
  const char* params[] =
  {
    "RDA string HostName= ",
    "RDA int SoftwareCh= ",
    "RDA floatlist SourceChOffset= 0 ",
    "RDA floatlist SourceChGain= 0 ",
    "RDA float SamplingRate= 1 ",
    "RDA int TransmitCh= ",
    "RDA intlist TransmitChList= 0 ",
    "RDA int SpatialFilteredChannels= ",
    "RDA matrix SpatialFilterKernal= 0 1 ",
    "RDA int SourceMax= 300 ",
    "RDA int SourceMin= -300 ",
  };
  for( size_t i = 0; i < sizeof( params ) / sizeof( *params ); ++i )
    paramlist.AddParameter2List( ( string( params[ i ] ) + " // getparams " + hostname ).c_str() );

  paramlist[ "HostName" ].SetValue( hostname );
  int numInputChannels = q.info().numChannels + 1;
  paramlist[ "SoftwareCh" ].SetValue( str( numInputChannels ) );
  paramlist[ "SourceChOffset" ].SetNumValues( numInputChannels );
  paramlist[ "SourceChGain" ].SetNumValues( numInputChannels );
  paramlist[ "TransmitCh" ].SetValue( str( numInputChannels - 1 ) );
  paramlist[ "TransmitChList" ].SetNumValues( numInputChannels - 1 );
  paramlist[ "SpatialFilteredChannels" ].SetValue( str( numInputChannels - 1 ) );
  paramlist[ "SpatialFilterKernal" ].SetDimensions( numInputChannels - 1, numInputChannels - 1 );

  for( int i = 0; i < numInputChannels - 1; ++i )
  {
    paramlist[ "SourceChOffset" ].SetValue( "0", i );
    paramlist[ "SourceChGain" ].SetValue( str( q.info().channelResolutions[ i ] ), i );
    paramlist[ "TransmitChList" ].SetValue( str( i + 1 ), i );
    paramlist[ "SpatialFilterKernal" ].SetValue( "1", i, i );
  }
  paramlist[ "SourceChOffset" ].SetValue( "0", numInputChannels - 1 );
  paramlist[ "SourceChGain" ].SetValue( "1", numInputChannels - 1 );
  paramlist[ "SamplingRate" ].SetValue( str( 1e6 / q.info().samplingInterval ) );

  cout << paramlist;

  return noError;
}