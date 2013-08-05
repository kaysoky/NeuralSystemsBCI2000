////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: griffin.milsap@gmail.com
// Description: Implementation of a source module for Blackrock systems
//
// Known Issues:
//   *cbMex/Blackrock library issue: Analog input channels are listed as invalid
//     while blackrock is starting up
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

#include "BlackrockADC.h"
#include "BCIError.h"

#include "ThreadUtils.h" // for SleepFor()

#include <sstream>
#include <algorithm>

#define INST 0
int gValidRates[] = { 500, 1000, 2000, 10000, 30000 };

using namespace std;

RegisterFilter( BlackrockADC, 1 );

BlackrockADC::BlackrockADC()
{

  // Declare any parameters that the filter needs....

 BEGIN_PARAMETER_DEFINITIONS

    "Source:Signal%20Properties int SourceCh= 144 "
       "144 1 % // number of digitized and stored channels",

    "Source:Signal%20Properties intlist AnalogCh= 1 0 "
       "0 % % // Analog channels to acquire from Blackrock",

    "Source:Signal%20Properties int SignalFilter= 0 "
       "0 0 12 // Filter to apply to data: "
         "0 No Filter, "
         "1 HP 750Hz, "
         "2 HP 250Hz, "
         "3 HP 100Hz, "
         "4 LP 50Hz, "
         "5 LP 125Hz, "
         "6 LP 250Hz, "
         "7 LP 500Hz, "
         "8 LP 150Hz, "
         "9 BP 10Hz-250Hz, "
         "10 LP 2.5kHz, "
         "11 LP 2kHz, "
         "12 BP 250Hz-5kHz "
       "(enumeration)",

// TODO: Figure out how to add this into the mix
//  "Source:Signal%20Properties int LogDigital= 0 "
//     "1 0 1 // Acquire Digital Input (boolean)",

    "Source:Signal%20Properties int SampleBlockSize= 1000 "
       "1000 1 % // number of samples transmitted at a time",

    "Source:Signal%20Properties float SamplingRate= 30000Hz "
       "30000Hz 0.0 % // sample rate",

 END_PARAMETER_DEFINITIONS

}

BlackrockADC::~BlackrockADC()
{
  // You should think twice before deallocating memory here as opposed to OnHalt().
  // OnHalt() is automatically called by BufferedADC upon destruction of this object.
}

void
BlackrockADC::OnHalt()
{
  // De-allocate any memory reserved in OnInitialize, stop any threads, etc.
  // Good practice is to write the OnHalt() method such that it is safe to call it even *before*
  // OnInitialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
  // have already been deleted:  set them to NULL after deletion).

  // Note that OnStopAcquisition() will be called immediately before this, in the acquisition
  // thread. OnStopAcquisition() is the proper place to do any amplifier-API cleanup.
}

void
BlackrockADC::OnPreflight( SignalProperties& Output ) const
{
  // Connect to the Blackrock cbMex library
  if( !Connect() )
    return;

  // Validate SamplingRate
  int rate = 0;
  stringstream strRates;
  for( int i = 0; i < sizeof( gValidRates ) / sizeof( int ); i++ )
  {
    strRates << gValidRates[i] << ", ";
    if( ( int )Parameter( "SamplingRate" ) == gValidRates[i] )
      rate = i + 1;
  }
  if( !rate )
      bcierr << "SamplingRate must be equal to a valid sampling rate.  Valid rates are: " << strRates.str() << endl;

  // Validate AnalogCh and DigitalCh
  vector< int > analogCh;
  if( !CreateChannelList( "AnalogCh", cbNUM_ANALOG_CHANS, analogCh ) )
  {
    Disconnect();
    return;
  }

  int numChannels = analogCh.size();
  //if( ( int )Parameter( "LogDigital" ) ) numChannels += cbNUM_DIGIN_CHANS;
  if( numChannels != Parameter( "SourceCh" ) )
  {
    bcierr << "According to AnalogCh/LogDigital, SourceCh should be " << numChannels << endl;
    Disconnect();
    return;
  }

  // Verify Channel Configuration
  // TODO: Take Digital into account here
  stringstream chanlabels;
  stringstream changains;
  bool checklabels = Parameter( "ChannelNames" )->NumValues() == Parameter( "SourceCh" );
  bool checkgain = Parameter( "SourceChGain" )->NumValues() == Parameter( "SourceCh" );
  bool checkoffset = Parameter( "SourceChOffset" )->NumValues() == Parameter( "SourceCh" );
  for( size_t i = 0; i < analogCh.size(); i++ )
  {
    cbPKT_CHANINFO chaninfo;
    if( CereLinkError( cbSdkGetChannelConfig( INST, analogCh[i], &chaninfo ) ) )
      bcierr << "Could not acquire channel info for channel " << analogCh[i] << endl;
    chanlabels << chaninfo.label << " ";
    if( checklabels && Parameter( "ChannelNames" )( i ) != string( chaninfo.label ) )
      checklabels = false;
    changains << chaninfo.physcalin.anagain << " ";
    if( checkgain && Parameter( "SourceChGain" )( i ) != chaninfo.physcalin.anagain )
      checkgain = false;
    if( checkoffset && Parameter( "SourceChOffset" )( i ) != 0 )
      checkoffset = false;
  }

  // Suggest Channel Labels
  if( !checklabels ) bciout << "ChannelNames should be set to " << chanlabels.str() << endl;

  // Validate Channel Gain/Offset
  if( !checkgain ) bcierr << "SourceChGain should be set to " << changains.str() << endl;
  if( !checkoffset ) bcierr << "SourceChOffset should be set to all zeros." << endl;

  // Configure channels to send data
  for( size_t i = 0; i < analogCh.size(); i++ )
  {
    // Configure the channels individually to report continuous data streams
    cbPKT_CHANINFO chaninfo;
    CereLinkError( cbSdkGetChannelConfig( INST, analogCh[i], &chaninfo ) );
    chaninfo.type = cbPKTTYPE_CHANSETSMP;
    chaninfo.smpfilter = ( int )Parameter( "SignalFilter" );
    chaninfo.smpgroup = rate;
    CereLinkError( cbSdkSetChannelConfig( INST, analogCh[i], &chaninfo ) );
  }

  // Disconnect from Blackrock cbMex
  Disconnect();

  Output = SignalProperties( numChannels, ( int )Parameter( "SampleBlockSize" ), SignalType::float32 );
}

void
BlackrockADC::OnInitialize( const SignalProperties& Output )
{
  // Get the sample rate and filtering information from CereLink
  mFilterEnum = Parameter( "SignalFilter" );
  for( int i = 0; i < sizeof( gValidRates ) / sizeof( int ); i++ )
    if( ( int )Parameter( "SamplingRate" ) == gValidRates[i] )
      mSampleRateEnum = i + 1;

  CreateChannelList( "AnalogCh", cbNUM_ANALOG_CHANS, mAnalogCh );

  mSampleBlockSize = Parameter( "SampleBlockSize" );
}

void
BlackrockADC::OnStartAcquisition()
{
  Connect();

  for( size_t i = 0; i < mAnalogCh.size(); i++ )
  {
    // Configure the channels individually to report continuous data streams
    cbPKT_CHANINFO chaninfo;
    CereLinkError( cbSdkGetChannelConfig( INST, mAnalogCh[i], &chaninfo ) );
    chaninfo.type = cbPKTTYPE_CHANSETSMP;
    chaninfo.smpfilter = mFilterEnum;
    chaninfo.smpgroup = mSampleRateEnum;
    CereLinkError( cbSdkSetChannelConfig( INST, mAnalogCh[i], &chaninfo ) );
  }

  // Prime/Clear the data buffer queues
  mDataMutex.Acquire();
  for( int i = 1; i <= cbNUM_ANALOG_CHANS; i++ )
    mDataBuffer[i] = queue< INT16 >();
  mDataMutex.Release();

  // Register a callback for data
  mFramesQueued = 0;
  CereLinkError( cbSdkRegisterCallback( INST, CBSDKCALLBACK_CONTINUOUS, DataCallback, this ) );
}

void
BlackrockADC::DoAcquire( GenericSignal& Output )
{
  while( mFramesQueued < mSampleBlockSize )
    ThreadUtils::SleepFor( 1 );

  // Dequeue data into the signal output
  mDataMutex.Acquire();
  for( int i = 0; i < Output.Channels(); i++ )
  {
    for( int j = 0; j < Output.Elements(); j++ )
    {
      if( !mDataBuffer[ mAnalogCh[i] ].empty() )
      {
        Output( i, j ) = mDataBuffer[ mAnalogCh[i] ].front();
        mDataBuffer[ mAnalogCh[i] ].pop();
      }
    }
  }
  mFramesQueued -= mSampleBlockSize;
  mDataMutex.Release();
}

void
BlackrockADC::StartRun()
{
}

void
BlackrockADC::StopRun()
{
}

void
BlackrockADC::OnStopAcquisition()
{
  Disconnect();
}

bool
BlackrockADC::Connect() const
{
  cbSdkConnectionType conType = CBSDKCONNECTION_DEFAULT;
  if( CereLinkError( cbSdkOpen( INST, conType, cbSdkConnection() ) ) ) return false;

  cbSdkInstrumentType instType;
  if( CereLinkError( cbSdkGetType( INST, &conType, &instType ) ) )
  {
    bcierr << "Unable to determine connection type" << endl;
    return false;
  }

  cbSdkVersion ver;
  if( CereLinkError( cbSdkGetVersion( INST, &ver ) ) )
  {
    bcierr << "Unable to get NSP version.  Is device connected and on?" << endl;
    return false;
  }

  if( conType < 0 || conType > CBSDKCONNECTION_COUNT )
    conType = CBSDKCONNECTION_COUNT;
  if( instType < 0 || instType > CBSDKINSTRUMENT_COUNT )
    instType = CBSDKINSTRUMENT_COUNT;

  string strConnection[] = { "Default", "Central", "UDP", "Closed", "Unknown" };
  string strInstrument[] = { "NSP", "nPlay", "Local NSP", "Remote nPlay", "Unknown" };
  bcidbg( 0 ) << strConnection[ conType ] << " real-time interface to " << strInstrument[ instType ]
              << "(V" << ver.nspmajor << "." << ver.nspminor << "." << ver.nsprelease << "." << ver.nspbeta << ")" << endl;

  return true;
}

void BlackrockADC::DataCallback( UINT32 iInstance, const cbSdkPktType iType, const void* iData, void* iBlackrockADC )
{
  BlackrockADC* bradc = reinterpret_cast< BlackrockADC* >( iBlackrockADC );

  switch( iType )
  {
  case cbSdkPkt_PACKETLOST:
    bcierr << "Packet loss.  Data has been lost.  Reduce system load." << endl;
    break;
  case cbSdkPkt_CONTINUOUS:
    if( bradc && iData )
    {
      // Grab the packet and ensure that it is the right sample group
      const cbPKT_GROUP *pPkt = reinterpret_cast< const cbPKT_GROUP* >( iData );
      if( pPkt->type != bradc->mSampleRateEnum ) break;
      
      // Determine if this packet contains different data than the buffered packets...
      UINT32 length, list[cbNUM_ANALOG_CHANS];
      if( CereLinkError( cbSdkGetSampleGroupList( iInstance, 1, pPkt->type, &length, list ) ) )
        bcierr << "Error retreiving sample group list" << endl;
      else
      {
        // We'll just queue all data received
        bradc->mDataMutex.Acquire();
        for( UINT32 i = 0; i < length; i++ )
          if( find( bradc->mAnalogCh.begin(), bradc->mAnalogCh.end(), list[ i ] )!= bradc->mAnalogCh.end() )
            bradc->mDataBuffer[ list[ i ] ].push( pPkt->data[i] );
        bradc->mFramesQueued++;
        bradc->mDataMutex.Release();
      }
    }
    break;
  default:
    break;
  }
  return;
}


void
BlackrockADC::Disconnect() const
{
  CereLinkError( cbSdkClose( INST ) );
}

// Input:
//   iParameter  -- string name of parameter to use
//   iMaxEntries -- value to assume if parameter has one entry of '0'
//   oVector     -- output vector
// Output: True if succeeded, False if error
bool
BlackrockADC::CreateChannelList( std::string iParameter, int iMaxEntries, std::vector< int > &oVector ) const
{
  // If the entry is 0, assume max value.  If negative, assume the absolute value
  oVector.clear();
  if( Parameter( iParameter )->NumValues() == 1 )
  {
    int entry = ( int )Parameter( "AnalogCh" )( 0 );
    if( entry <= 0 )
      for( int i = 0; i < ( ( entry == 0 ) ? iMaxEntries : abs( entry ) ); i++ )
        oVector.push_back( i + 1 );
  }

  // If we haven't already filled the vector, we must have a manually specified list of channels
  if( oVector.empty() )
  {
    if( Parameter( iParameter )->NumValues() != iMaxEntries )
    {
      bcierr << iParameter << " must be  <= '0' or have " << iMaxEntries << " entries" << endl;
      return false;
    }
    for( int i = 0; i < Parameter( iParameter )->NumValues(); i++ )
      if( ( int )Parameter( iParameter )( i ) )
        oVector.push_back( i + 1 );
  }

  return true;
}

bool
BlackrockADC::CereLinkError( cbSdkResult res )
{
  cbSdkInstrumentType instType;
  switch( res )
  {
  case CBSDKRESULT_WARNCONVERT:
    bciout << "File conversion is needed..." << endl;
    return false;
  case CBSDKRESULT_WARNCLOSED:
    bciout << "Library is already closed" << endl;
    return false;
  case CBSDKRESULT_WARNOPEN:
    bciout << "Library is already opened" << endl;
    return false;
  case CBSDKRESULT_SUCCESS:
    //bcidbg( 0 ) << "Success" << endl;
    return false;
  case CBSDKRESULT_NOTIMPLEMENTED:
    bcierr << "Not implemented" << endl;
    return true;
  case CBSDKRESULT_INVALIDPARAM:
    bcierr << "Invalid parameter" << endl;
    return true;
  case CBSDKRESULT_CLOSED:
    bcierr << "Interface is closed cannot do this operation" << endl;
    return true;
  case CBSDKRESULT_OPEN:
    bcierr << "Interface is open cannot do this operation" << endl;
    return true;
  case CBSDKRESULT_NULLPTR:
    bcierr << "Null pointer" << endl;
    return true;
  case CBSDKRESULT_ERROPENCENTRAL:
    bcierr << "Unable to open Central interface" << endl;
    return true;
  case CBSDKRESULT_ERROPENUDP:
    if( !CereLinkError( cbSdkGetType( INST, NULL, &instType ) ) )
      if( instType == CBSDKINSTRUMENT_NPLAY || instType == CBSDKINSTRUMENT_REMOTENPLAY )
        bcierr << "Unable to open UDP interface to nPlay" << endl;
      else
        bcierr << "Unable to open UDP interface" << endl;
    else
      bcierr << "Unable to open UDP interface (might happen if default)" << endl;
    return true;
  case CBSDKRESULT_ERROPENUDPPORT:
    bcierr << "Unable to open UDP port" << endl;
    return true;
  case CBSDKRESULT_ERRMEMORYTRIAL:
    bcierr << "Unable to allocate RAM for trial cache data" << endl;
    return true;
  case CBSDKRESULT_ERROPENUDPTHREAD:
    bcierr << "Unable to open UDP timer thread" << endl;
    return true;
  case CBSDKRESULT_ERROPENCENTRALTHREAD:
    bcierr << "Unable to open Central communication thread" << endl;
    return true;
  case CBSDKRESULT_INVALIDCHANNEL:
    bcierr << "Invalid channel number" << endl;
    return true;
  case CBSDKRESULT_INVALIDCOMMENT:
    bcierr << "Comment too long or invalid" << endl;
    return true;
  case CBSDKRESULT_INVALIDFILENAME:
    bcierr << "Filename too long or invalid" << endl;
    return true;
  case CBSDKRESULT_INVALIDCALLBACKTYPE:
    bcierr << "Invalid callback type" << endl;
    return true;
  case CBSDKRESULT_CALLBACKREGFAILED:
    bcierr << "Callback register/unregister failed" << endl;
    return true;
  case CBSDKRESULT_ERRCONFIG:
    bcierr << "Trying to run an unconfigured method" << endl;
    return true;
  case CBSDKRESULT_INVALIDTRACKABLE:
    bcierr << "Invalid trackable id, or trackable not present" << endl;
    return true;
  case CBSDKRESULT_INVALIDVIDEOSRC:
    bcierr << "Invalid video source id, or video source not present" << endl;
    return true;
  case CBSDKRESULT_ERROPENFILE:
    bcierr << "Cannot open file" << endl;
    return true;
  case CBSDKRESULT_ERRFORMATFILE:
    bcierr << "Wrong file format" << endl;
    return true;
  case CBSDKRESULT_OPTERRUDP:
    bcierr << "Socket option error (Possibly permission issue)" << endl;
    return true;
  case CBSDKRESULT_MEMERRUDP:
    bcierr << "Socket memory assignment error" << endl
           << " Consider using sysctl -w net.core.rmem_max=8388608" << endl
           << " or sysctl -w kern.ipc.maxsockbuf=8388608" << endl;
    return true;
  case CBSDKRESULT_INVALIDINST:
    bcierr << "Invalid range or instrument address" << endl;
    return true;
  case CBSDKRESULT_ERRMEMORY:
    bcierr << "Library memory allocation error" << endl;
    return true;
  case CBSDKRESULT_ERRINIT:
    bcierr << "Library initialization error" << endl;
    return true;
  case CBSDKRESULT_TIMEOUT:
    bcierr << "Connection timeout error" << endl;
    return true;
  case CBSDKRESULT_BUSY:
    bcierr << "Resource is busy" << endl;
    return true;
  case CBSDKRESULT_ERROFFLINE:
    bcierr << "Instrument is offline" << endl;
    return true;
  case CBSDKRESULT_UNKNOWN:
  default:
    bcierr << "Unknown error.  Sorry!" << endl;
    return true;
  }
}

