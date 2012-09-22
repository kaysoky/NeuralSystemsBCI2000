////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A source module connecting to a Neuroscan Acquire server.
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

#include "NeuroscanADC.h"
#include "NeuroscanProtocol.h"
#include "ThreadUtils.h"

using namespace std;

static const int cTimeout = 2000; // ms

RegisterFilter( NeuroscanADC, 1 );

NeuroscanADC::NeuroscanADC()
: mServer( mSocket ),
  mEventChannels( 0 )
{
 BEGIN_PARAMETER_DEFINITIONS
   "Source int SourceCh=      16 16 1 % "
       "// number of digitized channels (has to match Neuroscan)",
   "Source int SampleBlockSize= 32 5 1 % "
       "// number of samples per block (has to match Neuroscan)",
   "Source int SamplingRate=    256 128 1 % "
       "// the signal sampling rate (has to match Neuroscan)",
   "Source string ServerAddress= localhost:3998 "
       "// address and port of the Neuroscan Acquire server",
 END_PARAMETER_DEFINITIONS

 BEGIN_STATE_DEFINITIONS
   "NeuroscanEvent1 8 0 0 0",
 END_STATE_DEFINITIONS
}

NeuroscanADC::~NeuroscanADC()
{
  Halt();
}

void
NeuroscanADC::Preflight( const SignalProperties&, SignalProperties& Output ) const
{
  // Connect to the server and gather basic info to compare against the BCI2000 parameter settings
  client_tcpsocket socket( Parameter( "ServerAddress" ).c_str() );
  sockstream server( socket );
  if( !server.is_open() )
  {
    bcierr << "Could not connect to ServerAddress=" << Parameter( "ServerAddress" ) << ". "
           << "Make sure Acquire is running and the server is enabled at the correct port."
           << endl;
    return;
  }
  NscInfoRequest().WriteBinary( server ).flush();
  NscPacketHeader response;
  if( socket.wait_for_read( cTimeout ) )
    response.ReadBinary( server );
  else
  {
    bcierr << "Server connection timed out" << endl;
    return;
  }
  if( response.Id() != HeaderIdData || response.Code() != DataType_InfoBlock || response.Value() != InfoType_BasicInfo )
  {
    bcierr << "Unexpected packet from server: " << response << endl;
    return;
  }
  NscBasicInfo AcqSettings;
  AcqSettings.ReadBinary( server );
  if( !server )
  {
    bcierr << "Could not read data packet" << endl;
    return;
  }
  NscCloseRequest().WriteBinary( server ).flush();
  socket.close();
  
  PreflightCondition( Parameter( "SourceCh" ) == AcqSettings.EEGChannels() );
  PreflightCondition( Parameter( "SampleBlockSize" ) == AcqSettings.SamplesInBlock() );
  PreflightCondition( Parameter( "SamplingRate" ).InHertz() == AcqSettings.SamplingRate() );
  for( int ch = 0; ch < Parameter( "SourceCh" )->NumValues(); ++ch )
  {
    PreflightCondition( Parameter( "SourceChOffset" )( ch ) == 0 );
    PreflightCondition( fabs( Parameter( "SourceChGain" )( ch ) - AcqSettings.Resolution() ) < 1e-3 );
  }

  SignalType outSignalType;
  switch( AcqSettings.DataDepth() )
  {
    case 2:
      outSignalType = SignalType::int16;
      break;
    case 4:
      outSignalType = SignalType::int32;
      break;
    default:
      bcierr << "Server reports unsupported data size "
             << "(" << AcqSettings.DataDepth() * 8 << " bits per sample)"
             << endl;
  }
  Output = SignalProperties( Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), outSignalType );
}

void
NeuroscanADC::Initialize( const SignalProperties&, const SignalProperties& )
{
  mServer.clear();
  int timeout = 2000,
      resolution = 100;
  while( !mSocket.is_open() && timeout > 0 )
  {
    mSocket.open( Parameter( "ServerAddress" ).c_str() );
    if( !mSocket.is_open() )
    {
      ThreadUtils::SleepFor( resolution );
      timeout -= resolution;
    }
  }
  NscInfoRequest().WriteBinary( mServer ).flush();
  NscPacketHeader header;
  while( mServer && mServer.is_open() &&
    ( header.Id() != HeaderIdData
      || header.Code() != DataType_InfoBlock 
      || header.Value() != InfoType_BasicInfo )
  )
  {
    mServer.ignore( header.DataSize() );
    header.ReadBinary( mServer );
  }
  NscBasicInfo info;
  info.ReadBinary( mServer );
  mEventChannels = info.EventChannels();
  NscStartAcquisition().WriteBinary( mServer );
  NscStartDataRequest().WriteBinary( mServer );
  mServer.flush();
}

void
NeuroscanADC::Process( const GenericSignal&, GenericSignal& Output )
{
  NscPacketHeader header;
  while( mServer && ( header.Id() != HeaderIdData || header.Code() != DataType_EegData ) )
  {
    mServer.ignore( header.DataSize() );
    header.ReadBinary( mServer );
  }
  if( mServer )
    switch( header.Value() )
    {
      case DataTypeRaw16bit:
        ReadData<int16_t>( Output );
        break;
      case DataTypeRaw32bit:
        ReadData<int32_t>( Output );
        break;
      default:
        bcierr << "Unknown data format" << endl;
    }
  if( !mServer )
    bcierr << "Lost connection to Acquire server" << endl;
}

void
NeuroscanADC::Halt()
{
  NscStopDataRequest().WriteBinary( mServer );
  NscCloseRequest().WriteBinary( mServer );
  mServer.flush();
  mSocket.close();
}

template<typename T>
void
NeuroscanADC::ReadData( GenericSignal& Output )
{
  for( int sample = 0; sample < Output.Elements(); ++sample )
  {
    T value;
    for( int channel = 0; channel < Output.Channels(); ++channel )
    {
      LittleEndianData::get( mServer, value );
      Output( channel, sample ) = value;
    }
    if( mEventChannels > 0 )
    {
      LittleEndianData::get( mServer, value );
      State( "NeuroscanEvent1" )( sample ) = value & 0xff;
      mServer.ignore( sizeof( T ) * ( mEventChannels - 1 ) );
    }
  }
}

