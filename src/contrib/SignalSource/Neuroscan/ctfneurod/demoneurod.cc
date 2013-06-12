//////////////////////////////////////////////////////////////////////////////////////////////
//
//  File:        demoneurod.cpp
//
//  Author:      juergen.mellinger@uni-tuebingen.de
//
//  Date:        Sept 21, 2012
//
//  Description: A test program for the Neuroscan demon framework.
//
///////////////////////////////////////////////////////////////////////////////////////////////
#include "./NeuroSrv.h"
#include <stdexcept>
#include <sstream>
#include <cmath>

using namespace std;

class DemoNeuroSrv : public NeuroSrv
{
 public:
  DemoNeuroSrv( int, char** );
  int Run();
  
 protected:
  virtual int SendData( std::ostream& );
  virtual void SendASTSetupFile( std::ostream& );

 private:
  int mRemArgc;
  char** mRemArgv;
};

DemoNeuroSrv::DemoNeuroSrv( int argc, char** argv )
: mRemArgc( argc ),
  mRemArgv( argv )
{
  int eegChannels = 13,
      eventChannels = 2,
      samplesInBlock = 17,
      samplingRate = 129,
      dataDepth = 4;
  float resolution = ::pow( 2.f, -dataDepth * 8 + 7 );
  
  mBasicInfo = NscBasicInfo( eegChannels, eventChannels, samplesInBlock, samplingRate, dataDepth, resolution );
  mChannelInfo.resize( eegChannels );
  for( size_t i = 0; i < mChannelInfo.size(); ++i )
  {
    ostringstream oss;
    oss << "EEG Ch" << i + 1;
    mChannelInfo[i].name = oss.str();
    mChannelInfo[i].unit = "uV";
  }
}

int
DemoNeuroSrv::Run()
{
  return NeuroSrv::Run( mRemArgc, const_cast<const char**>( mRemArgv ) );
}

void
DemoNeuroSrv::SendASTSetupFile( std::ostream& os )
{
  char data[2049];
  for( size_t i = 0; i < sizeof( data ) / sizeof( *data ); ++i )
    data[i] = static_cast<char>( i );
  NscPacketHeader( 'FILE', SetupFile, CtfDSFormat, sizeof( data ) ).WriteBinary( os );
  os.write( data, sizeof( data ) );
  os.flush();
}

int
DemoNeuroSrv::SendData( std::ostream& os )
{
  int numBytes = ( mBasicInfo.EEGChannels() + mBasicInfo.EventChannels() ) * mBasicInfo.SamplesInBlock() * mBasicInfo.DataDepth(),
      dataType;
  switch( mBasicInfo.DataDepth() )
  {
    case 2:
      dataType = DataTypeRaw16bit;
      break;
    case 4:
      dataType = DataTypeRaw32bit;
      break;
    default:
      throw runtime_error( "unsupported value in mBasicInfo.DataDepth()" );
  }
  NscPacketHeader( HeaderIdData, DataType_EegData, dataType, numBytes ).WriteBinary( os );
  for( int i = 0; i < numBytes; ++i )
    os.put( i );
  os.flush();
  return ( 1000 * mBasicInfo.SamplesInBlock() ) / mBasicInfo.SamplingRate();
}


int
main( int argc, char* argv[] )
{
  int result = 0;
  try
  {
    result = DemoNeuroSrv( argc, argv ).Run();
  }
  catch( const exception& e )
  {
    cerr << "Exception caught: " << e.what() << endl;
    result = -1;
  }
  catch( const char* s )
  {
    cerr << "Exception caught: " << s << endl;
    result = -1;
  }
  catch( ... )
  {
    cerr << "Unknown exception caught" << endl;
    result = -1;
  }
  return result;
}
