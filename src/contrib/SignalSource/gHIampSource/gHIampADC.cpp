////////////////////////////////////////////////////////////////////////////////
// $Id: $
// Authors: griffin.milsap@gmail.com
// Description: gHIampADC implementation
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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

#include "gHIampADC.h"
#include "BCIError.h"

#include <sstream>

using namespace std;

// Valid Sampling Rates and Sample Block Sizes
#define NUM_MODES 4
static int ValidRates[NUM_MODES]      = { 256, 256, 512, 512 };
static int ValidBlockSizes[NUM_MODES] = {  16,  32,  16,  32 };

static string ValidModes = 
  "256 Hz: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 32 64 128 256 \n"
  "512 Hz: 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 32 64 128 256 \n"
  "600 Hz: 4 5 6 7 8 9 10 11 12 13 15 16 17 32 64 128 256 \n"
  "1200 Hz: 8 9 10 11 12 13 14 15 16 32 64 128 256 \n"
  "2400 Hz: 16 32 64 128 256 \n"
  "4800 Hz: 32 64 128 256 \n";

RegisterFilter( gHIampADC, 1 );

gHIampADC::gHIampADC()
: mMasterIdx( 0 )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Source:Signal%20Properties int SourceCh= 256 "
      "256 1 % // number of digitized and stored channels",
    "Source:Signal%20Properties int SampleBlockSize= 16 "
      "16 1 % // number of samples transmitted at a time",
    "Source:Signal%20Properties float SamplingRate= 256Hz "
      "256Hz 0.0 % // sample rate",
    "Source:Signal%20Properties string DeviceIDMaster= auto "
      "auto % % // deviceID for the device whose SYNC goes to the slaves",
    "Source:Signal%20Properties list DeviceIDs= 1 auto "
      "// list of HIamps to be used",
    "Source:Signal%20Properties intlist RefChList= 0 "
      "% 0 256 // list of reference channels for each amp",
    "Source:Signal%20Properties list SourceChList= 0 "
      "// list of amp.channel to acquire",
    "Source:Signal%20Properties int FilterEnabled= 0 "
      "0 0 1 // Enable pass band filter (boolean)",
    "Source:Signal%20Properties float FilterHighPass= 0.1 "
      "0.1 0 50 // high pass filter for pass band",
    "Source:Signal%20Properties float FilterLowPass= 60 "
      "60 0 4000 // low pass filter for pass band",
    "Source:Signal%20Properties int FilterModelOrder= 8 "
      "8 1 12 // filter model order for pass band",
    "Source:Signal%20Properties int FilterType= 1 "
      "1 1 3 // pass band filter type: "
        "1 Chebyshev, "
        "2 Butterworth, "
        "3 Bessel "
        "(enumeration)",
    "Source:Signal%20Properties int NotchEnabled= 0 "
      "0 0 1 // Enable notch filter (boolean)",
    "Source:Signal%20Properties float NotchHighPass= 58 "
      "58 0 70 // high pass filter for notch filter",
    "Source:Signal%20Properties float NotchLowPass= 62 "
      "62 0 4000 // low pass filter for notch filter",
    "Source:Signal%20Properties int NotchModelOrder= 4 "
      "4 1 10 // filter model order for notch filter",
    "Source:Signal%20Properties int NotchType= 2 "
      "2 1 3 // pass band filter type: "
        "1 Chebyshev, "
        "2 Butterworth, "
        "3 Bessel "
        "(enumeration)",
    // No other modes supported yet
    //"Source:Signal%20Properties int AcquisitionMode= 0 "
    //  "0 0 2 // data acquisition mode: "
    //    "0 Signal Acquisition, "
    //    "1 Calibration, "
    //    "2 Impedance "
    //    "(enumeration)",
    // Digital "Trigger Lines" support not tested yet either
  END_PARAMETER_DEFINITIONS
}

gHIampADC::~gHIampADC()
{
  mDevices.Close();
}

void
gHIampADC::OnPreflight( SignalProperties& Output ) const
{
  bcidbg( 0 ) << "Using gHIamp driver version " << GT_GetDriverVersion() << endl;

  // Determine if the sampling rate and sample block size is valid
  if( Parameter( "SamplingRate" ).InHertz() < 1.0 )
    bcierr << "SamplingRate cannot be zero" << endl;
  int sampleRate = Parameter( "SamplingRate" );
  int blockSize = Parameter( "SampleBlockSize" );
  gHIampADC::ModeMap modes = ParseModes( ValidModes );
  bool validMode = modes[ sampleRate ].find( blockSize ) != modes[ sampleRate ].end(); 
  if( !validMode )
  {
    bciout << "You are using an unsupported sampling rate/sample block size combination!" << endl
           << "Be aware of your limited karma!" << endl;
    bciout << "Supported SampleRate/SampleBlockSize combinations: \n" << ValidModes;
  }

  // Determine if the requested amps are connected.
  gHIampDeviceContainer devices;
  if( !devices.Detect() )
  {
    bcierr << "No gHIamp devices were detected." << endl;
    return;
  }

  for( size_t i = 0; i < devices.size(); i++ )
    bcidbg( 0 ) << "g.HIamp Detected: Serial " << devices[i].Serial()
                << ", Hardware Version " << devices[i].HWVersion() << endl;

  if( Parameter( "DeviceIDs" )->NumValues() == 0 )
  {
    bcierr << "DeviceIDs cannot be blank. Use \"auto\" (without quotes) to auto-configure." << endl;
    devices.Close();
    return;
  }
  if( Parameter( "DeviceIDMaster" )->NumValues() == 0 )
  {
    bcierr << "DeviceIDMaster cannot be blank. Use \"auto\" (without quotes) to auto-configure." << endl;
    devices.Close();
    return;
  }

  int masterIdx = 0;
  if( devices.size() > 1 )
  {
    if( ( string )Parameter( "DeviceIDs" )(0) == "auto" )
      bcierr << "Cannot auto configure parameter DeviceIDs: more than one gHIamp detected." << endl;
    if( ( string )Parameter( "DeviceIDMaster" ) == "auto" )
      bcierr << "Cannot auto configure parameter DeviceIDMaster: more than one gHIamp detected." << endl;
  }
  if( ( string )Parameter( "DeviceIDs" )(0) != "auto" )
  {
    if( ( string )Parameter( "DeviceIDMaster" ) == "auto" )
      bcierr << "If specifying DeviceIDs, one device must be specified as master in DeviceIDMaster." << endl;
    for( int i = 0; i < Parameter( "DeviceIDs" )->NumValues(); i++ )
    {
      bool detected = false;
      for( size_t j = 0; j < devices.size(); j++ )
      {
        if( devices[j].Serial() == Parameter( "DeviceIDMaster" ) )
          detected = true;
        else if( devices[j].Serial() == Parameter( "DeviceIDs" )(i) )
        {
          // The current gHIamp doesn't support slaving...
          bciout << "Slave mode not supported yet..." << endl;
          devices[j].SetIsSlave( true );
          detected = true;
        }
      }
      if( !detected )
        bcierr << "Device with serial " << Parameter( "DeviceIDs" )(i)
               << " was requested in DeviceIDs but not detected." << endl;
    }
  }
  if( ( string )Parameter( "DeviceIDMaster" ) != "auto" )
  {
    bool detected = false;
    for( size_t i = 0; i < devices.size(); i++ )
    {
      if( ( string )Parameter( "DeviceIDMaster" ) == devices[i].Serial() )
      {
        devices[i].SetIsSlave( false );
        masterIdx = i;
        detected = true;
      }
    }
    if( !detected )
      bcierr << "Device with serial " << Parameter( "DeviceIDMaster" )
             << " was requested as the master device, but not detected." << endl;
  }

  // Check Bandpass Filter Settings
  int filterNo = 0;
  if( ( int )Parameter( "FilterEnabled" ) )
    if( !DetermineFilterNumber( filterNo ) )
      bcierr << "Could not find appropriate pass band filter in gHIamp." << endl;

  // Check Notch Filter Settings
  int notchNo = 0;
  if( ( int )Parameter( "NotchEnabled" ) )
    if( !DetermineNotchNumber( notchNo ) )
      bcierr << "Could not find appropriate notch filter in gHIamp." << endl;

  // Seems all the requested devices are connected and detected;
  // Attempt to determine if the requested channels are valid
  if( Parameter( "SourceChList" )->NumValues() == 0 )
  {
    // No SourceChList has been specified; just record SourceCh channels
    int channels = Parameter( "SourceCh" );
    if( Parameter( "DeviceIDs" )( 0 ) == "auto" )
    {
      for( size_t idx = 0; idx < devices.size(); idx++ )
        channels -= devices[idx].MapAllAnalogChannels( ( int )Parameter( "SourceCh" ) - channels, channels );
    } else {
      for( int dev = 0; dev < Parameter( "DeviceIDs" )->NumValues(); dev++ )
        for( size_t idx = 0; idx < devices.size(); idx++ )
          if( Parameter( "DeviceIDs" )(dev) == devices[idx].Serial() )
            channels -= devices[idx].MapAllAnalogChannels( ( int )Parameter( "SourceCh" ) - channels, channels );
    }
    if( channels != 0 )
      bcierr << "SourceCh is " << Parameter( "SourceCh" ) << ", but the sum of "
             << "available analog channels on all connected devices is "
             << Parameter( "SourceCh" ) - channels << endl;
  } else {
    if( Parameter( "SourceChList" )->NumValues() != Parameter( "SourceCh" ) )
      bcierr << "Number of entries in SourceChList must match the number in SourceCh" << endl;
    if( Parameter( "DeviceIDs" )(0) == "auto" ) {
      bcierr << "If you're going to specify SourceChList, you need "
             << "a corresponding mapping of amps in DeviceIDs." << endl;
      return;
    }
    for( int i = 0; i < Parameter( "SourceChList" )->NumValues(); i++ )
    {
      // We need to process and map channels accordingly
      SrcCh s = SrcCh( Parameter( "SourceChList" )(i) );
      if( s.Amp() == 0 )
        bcierr << "Invalid formatting in SourceChList" << endl;
      for( size_t dev = 0; dev < devices.size(); dev++ )
      {
        if( devices[dev].Serial() == Parameter( "DeviceIDs" )( s.Amp() - 1 ) )
        {
          if( s.IsDigital() )
          {
            if( !devices[dev].MapDigitalChannel( s.Channel(), i ) )
              bcierr << "Error mapping digital channel " << s.Channel()
                     << " on device " << devices[dev].Serial() << endl;
          } else {
            if( !devices[dev].MapAnalogChannel( s.Channel(), i ) )
              bcierr << "Error mapping channel " << s.Channel()
                     << " on device " << devices[dev].Serial() << endl;
          }
        }
      }
    }
  }

  // Set up referencing
  if( Parameter( "RefChList" )->NumValues() != 0 )
  {
    if( Parameter( "RefChList" )->NumValues() != Parameter( "DeviceIDs" )->NumValues() )
      bcierr << "Number of entries in RefChList must equal number of entries in DeviceIDs." << endl;

    for( int i = 0; i < Parameter( "RefChList" )->NumValues(); i++ )
    {
      for( size_t dev = 0; dev < devices.size(); dev++ )
      {
        if( Parameter( "DeviceIDs" )( i ) == "auto" )
          devices[i].SetRefChan( ( ( int )Parameter( "RefChList" )( i ) ) - 1 );
        else if( devices[dev].Serial() == Parameter( "DeviceIDs" )( i ) )
          devices[dev].SetRefChan( ( ( int )Parameter( "RefChList" )( i ) ) - 1 );
      }
    }
  }

  // Close the devices
  devices.Close();

  // Inform the system what our output will look like
  int numberOfChannels = Parameter( "SourceCh" );
  int samplesPerBlock  = Parameter( "SampleBlockSize" );
  SignalType sigType = SignalType::float32;
  Output = SignalProperties( numberOfChannels, samplesPerBlock, sigType );
}


void
gHIampADC::OnInitialize( const SignalProperties& Output )
{
  // Detect and query all connected gHIamp devices
  if( !mDevices.Detect() )
    bcierr << "Error connecting to gHIamp devices" << endl
           << "-- Was able to find devices in preflight phase, but unable to find them later" << endl;

  // Remove all entries in mDevices that we don't want to log from
  if( Parameter( "DeviceIDs" )(0) != "auto" )
  {
    for( gHIampDeviceContainer::iterator itr = mDevices.begin(); itr != mDevices.end(); /* empty */ )
    {
      bool found = false;
      for( int i = 0; i < Parameter( "DeviceIDs" )->NumValues(); i++ )
        if( Parameter( "DeviceIDs" )(i) == itr->Serial() )
          found = true;
      if( !found )
        mDevices.Remove( itr++ );
      else
        ++itr;
    }
  }

  // Set Bandpass Filter
  if( ( int )Parameter( "FilterEnabled" ) )
  {
    int filterNo = 0;
    DetermineFilterNumber( filterNo );
    for( size_t i = 0; i < mDevices.size(); i++ )
      mDevices[i].SetFilter( filterNo );
  }

  // Set Notch Filter
  if( ( int )Parameter( "NotchEnabled" ) )
  {
    int notchNo = 0;
    DetermineNotchNumber( notchNo );
    for( size_t i = 0; i < mDevices.size(); i++ )
      mDevices[i].SetNotch( notchNo );
  }

  // Determine the master device index
  mMasterIdx = 0;
  if( Parameter( "DeviceIDMaster" ) != "auto" )
    for( size_t i = 0; i < mDevices.size(); i++ )
      if( Parameter( "DeviceIDMaster" ) == mDevices[i].Serial() )
        mMasterIdx = i;
  mDevices[ mMasterIdx ].SetIsSlave( false );

  // Map Channels
  if( Parameter( "SourceChList" )->NumValues() == 0 )
  {
    // No SourceChList has been specified; just record SourceCh channels
    int channels = Parameter( "SourceCh" );
    if( Parameter( "DeviceIDs" )(0) == "auto" )
    {
      for( size_t idx = 0; idx < mDevices.size(); idx++ )
        channels -= mDevices[idx].MapAllAnalogChannels( ( int )Parameter( "SourceCh" ) - channels, channels );
    } else {
      for( int dev = 0; dev < Parameter( "DeviceIDs" )->NumValues(); dev++ )
        for( size_t idx = 0; idx < mDevices.size(); idx++ )
          if( Parameter( "DeviceIDs" )(dev) == mDevices[idx].Serial() )
            channels -= mDevices[idx].MapAllAnalogChannels( ( int )Parameter( "SourceCh" ) - channels, channels );
    }
  } else {
    for( int i = 0; i < Parameter( "SourceChList" )->NumValues(); i++ )
    {
      // We need to process and map channels accordingly
      SrcCh s = SrcCh( Parameter( "SourceChList" )(i) );
      for( size_t dev = 0; dev < mDevices.size(); dev++ )
        if( mDevices[dev].Serial() == Parameter( "DeviceIDs" )( s.Amp() - 1 ) )
          if( s.IsDigital() )
            mDevices[dev].MapDigitalChannel( s.Channel() - 1, i );
          else
            mDevices[dev].MapAnalogChannel( s.Channel() - 1, i );
    }
  }

  // Set up referencing
  if( Parameter( "RefChList" )->NumValues() != 0 )
  {
    for( int i = 0; i < Parameter( "RefChList" )->NumValues(); i++ )
    {
      for( size_t dev = 0; dev < mDevices.size(); dev++ )
      {
        if( Parameter( "DeviceIDs" )( i ) == "auto" )
          mDevices[i].SetRefChan( ( ( int )Parameter( "RefChList" )( i ) ) - 1 );
        else if( mDevices[dev].Serial() == Parameter( "DeviceIDs" )( i ) )
          mDevices[dev].SetRefChan( ( ( int )Parameter( "RefChList" )( i ) ) - 1 );
      }
    }
  }

  // Configure the gHIamp devices
  for( size_t i = 0; i < mDevices.size(); i++ )
    mDevices[i].SetConfiguration( Parameter( "SamplingRate" ), Parameter( "SampleBlockSize" ) );
}

void
gHIampADC::OnStartAcquisition()
{
  // Start slaves first
  for( size_t i = 0; i < mDevices.size(); i++ )
    if( i != mMasterIdx ) mDevices[i].BeginAcquisition();

  // Then start the master
  mDevices[mMasterIdx].BeginAcquisition();
}

void
gHIampADC::OnStopAcquisition()
{
  // Stop slaves first
  for( size_t i = 0; i < mDevices.size(); i++ )
    if( i != mMasterIdx ) mDevices[i].EndAcquisition();

  // Then stop the master
  mDevices[mMasterIdx].EndAcquisition();
}

void
gHIampADC::OnHalt()
{
  mDevices.Close();
}

void
gHIampADC::DoAcquire( GenericSignal& Output )
{
  for( size_t i = 0; i < mDevices.size(); i++ )
    mDevices[i].GetData( Output );
}

// **************************************************************************
// Function:   DetermineFilterNumber
// Purpose:    This routine determines the pass band filter number that
//             a particular parameter setting corresponds to
// Parameters: oFilterNumber - int value which will hold the filter number
// Returns:    true if filter found, false if not
// **************************************************************************
bool
gHIampADC::DetermineFilterNumber( int& oFilterNumber ) const
{
  int nof;
  FILT *filt;
  bool found = false;
  int samplingrate = static_cast< int >( Parameter( "SamplingRate" ).InHertz() );

  GT_GetNumberOfFilter( &nof );
  filt = new _FILT[nof];
  GT_GetFilterSpec( filt );
  for( int no_filt = 0; no_filt < nof; no_filt++ )
  {
    // Output Filter Information
    bcidbg( 0 ) << "Bandpass Filter " << no_filt << ": High Pass = " << filt[no_filt].fu
                << ", LowPass = " << filt[no_filt].fo << ", Sampling Rate = " << filt[no_filt].fs
                << ", Order = " << filt[no_filt].order << ", Type = ";
    switch( ( int )filt[no_filt].type )
    {
    case F_CHEBYSHEV:
        bcidbg( 0 ) << "Chebyshev (" << F_CHEBYSHEV << ")"; break;
    case F_BUTTERWORTH:
        bcidbg( 0 ) << "Butterworth (" << F_BUTTERWORTH << ")"; break;
    case F_BESSEL:
        bcidbg( 0 ) << "Bessel (" << F_BESSEL << ")"; break;
    default:
        bcidbg( 0 ) << "Unknown"; break;
    }
    bcidbg( 0 ) << endl;

    if( ( fabs( filt[no_filt].fu - Parameter( "FilterHighPass" ) ) < 0.0001 ) &&
      ( fabs( filt[no_filt].fo - Parameter( "FilterLowPass" ) ) < 0.0001 ) &&
      ( filt[no_filt].fs == samplingrate ) &&
      ( filt[no_filt].order == Parameter( "FilterModelOrder" ) ) &&
      ( filt[no_filt].type == ( int )Parameter( "FilterType" ) - 1 ) )
    {
      oFilterNumber = no_filt;
      found = true;
    }
  }
  delete [] filt;
  return found;
}

// **************************************************************************
// Function:   DetermineNotchNumber
// Purpose:    This routine determines the notch filter number that
//             a particular parameter setting corresponds to
// Parameters: N/A
// Returns:    >=0: notch filter number
//             -1 filter number not found
// **************************************************************************
bool
gHIampADC::DetermineNotchNumber( int& oFilterNumber ) const
{
  int nof;
  FILT *filt;
  bool found = false;
  int samplingrate = static_cast< int >( Parameter( "SamplingRate" ).InHertz() );

  bcidbg( 0 ) << "Requested Notch: High Pass = " << Parameter( "NotchHighPass" )
                << ", LowPass = " << Parameter( "NotchLowPass" ) << ", Sampling Rate = " << samplingrate
                << ", Order = " << Parameter( "NotchModelOrder" ) << ", Type = " << ( int )Parameter( "NotchType" ) - 1 << endl;

  GT_GetNumberOfNotch( &nof );
  filt = new _FILT[nof];
  GT_GetNotchSpec( filt );
  for( int no_filt = 0; no_filt < nof; no_filt++ )
  {
    // Output Notch Information
    bcidbg( 0 ) << "Notch Filter " << no_filt << ": High Pass = " << filt[no_filt].fu
                << ", LowPass = " << filt[no_filt].fo << ", Sampling Rate = " << filt[no_filt].fs
                << ", Order = " << filt[no_filt].order << ", Type =  ";
    switch( ( int )filt[no_filt].type )
    {
    case F_CHEBYSHEV:
        bcidbg( 0 ) << "Chebyshev (" << F_CHEBYSHEV << ")"; break;
    case F_BUTTERWORTH:
        bcidbg( 0 ) << "Butterworth (" << F_BUTTERWORTH << ")"; break;
    case F_BESSEL:
        bcidbg( 0 ) << "Bessel (" << F_BESSEL << ")"; break;
    default:
        bcidbg( 0 ) << "Unknown"; break;
    }
    bcidbg( 0 ) << endl;

    if( ( fabs( filt[no_filt].fu - Parameter( "NotchHighPass" ) ) < 0.0001 ) &&
      ( fabs( filt[no_filt].fo - Parameter( "NotchLowPass" ) ) < 0.0001 ) &&
      ( filt[no_filt].fs == samplingrate ) &&
      ( filt[no_filt].order == Parameter( "NotchModelOrder" ) ) &&
      ( filt[no_filt].type == ( int )Parameter( "NotchType" ) - 1 ) )
    {
      oFilterNumber = no_filt;
      found = true;
    }
  }
  delete [] filt;
  return found;
}

gHIampADC::ModeMap gHIampADC::ParseModes( string modes ) const
{
  gHIampADC::ModeMap ret;
  istringstream ss( modes );
  string line;
  char label[4];
  while( getline( ss, line ) )
  {
    stringstream sl( line );
    int sampleRate = 0;
    int blockSize = 0;
    sl >> sampleRate >> label;
    while( sl >> blockSize )
      ret[ sampleRate ].insert( blockSize );
  }
  return ret;
}

// **************************************************************************
// SrcCh
// Purpose: Parsing SourceChList
// **************************************************************************
gHIampADC::SrcCh::SrcCh( string s )
: mAmp( 0 ),
  mChannel( 0 ),
  mDigital( false )
{
  size_t dotpos = s.find( "." );
  if( dotpos != string::npos )
  {
    mAmp = atoi( s.substr( 0, dotpos ).c_str() );
    string chan = s.substr( dotpos + 1 );
    size_t dpos = chan.find( "D" );
    if( dpos != string::npos )
    {
      mDigital = true;
      mChannel = atoi( chan.substr( dpos + 1 ).c_str() );
    }
    else
      mChannel = atoi( chan.c_str() );
  }
  else
    mChannel = atoi( s.c_str() );
}
