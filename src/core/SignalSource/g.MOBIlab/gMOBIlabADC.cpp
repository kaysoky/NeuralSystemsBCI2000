////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org,
//          jawilson@cae.wisc.edu,
//          juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 Source Module for gMOBIlab and gMOBIlab+ devices.
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

#include "gMOBIlabADC.h"

#ifndef GMOBILABPLUS
# include "spa20a.h"
#else // GMOBILABPLUS
# define __CFG GFG__
# include "gMOBIlabplus.h"
#endif // GMOBILABPLUS

const size_t cMaxReadBufSize = 1024; // Restriction of gtec driver.
const int cMinDataTimeoutMs = 1000;  // Minimum data timeout in ms.
const int cMaxVoidLoopCount = 3;     // Maximum number of zero reads in data acquisition loop.

using namespace std;

// Register the source class with the framework.
RegisterFilter( gMOBIlabADC, 1 );

// **************************************************************************
// Function:   gMOBIlabADC
// Purpose:    The constructor for the gMOBIlabADC.
// **************************************************************************
gMOBIlabADC::gMOBIlabADC()
: mpBuffer( NULL ),
  mBufferSize( 0 ),
  mTimeoutMs( 0 ),
  mDevice( NULL )
{
  ::memset( &mOverlapped, 0, sizeof( mOverlapped ) );

  BEGIN_PARAMETER_DEFINITIONS
   "Source string COMport= % COM3: % % "
       "// COMport for MOBIlab",
   "Source int SampleBlockSize= 8 8 1 % "
       "// number of samples per block",
   "Source int SamplingRate= 256 256 256 256 "
       "// the signal sampling rate",
   "Source floatlist SourceChGain= 16 "
       " 0.019 0.019 0.019 0.019 0.019 0.019 0.019 0.019 " // 8 EEG channels 
       " 1 1 1 1 1 1 1 1 " // up to 8 digital input channels
       " 0.019 % % // Conversion factors from AD units into Microvolts",
   "Source floatlist SourceChOffset= 16 "
       " 0 0 0 0 0 0 0 0 "
       " 0 0 0 0 0 0 0 0 "
       " 0 % % // AD offsets for recorded channels",
   "Source int AlignChannels= 1 0 0 1 "
       "// Align channels in time (must be switched on for gMOBIlab devices",
#ifndef GMOBILABPLUS
   "Source int SourceCh= 9 9 1 9 "
       "// number of digitized channels total",
#else // GMOBILABPLUS
   "Source int SourceCh= 8 16 1 16 "
       "// number of digitized channels total",
   "Source int MobiLabTestMode= 0 0 0 1"
   	 "// generate a test signal (boolean)",
   "Source int DigitalEnable= 0 0 0 1 "
     "// read digital inputs 1-8 as channels (boolean)",
   "Source int DigitalOutBlock= 0 0 0 1 "
     "// pulse digital output 7 during data reads (boolean)",
   "Source string DigitalOutputEx= % % % % "
     "// output high on digital output 4 when true (expression)",
#endif // GMOBILABPLUS
  END_PARAMETER_DEFINITIONS
}


gMOBIlabADC::~gMOBIlabADC()
{
  OnHalt();
}


// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally required properties
//             for the output signal; checks whether resources are available.
// Parameters: References to input and output signal properties.
// Returns:    N/A
// **************************************************************************
void
gMOBIlabADC::OnPreflight( SignalProperties& Output ) const
{
  int sourceCh = Parameter( "SourceCh" ),
      sampleBlockSize = Parameter( "SampleBlockSize" );

  if ( Parameter( "SamplingRate" ).InHertz() != 256 )
    bcierr << "MOBIlab sampling rate is fixed at 256 Hz. Change SamplingRate parameter to 256." << endl;
  if ( Parameter( "SourceCh" ) < 1 )
    bcierr << "Number of channels (SourceCh) has to be at least 1." << endl;
#ifndef GMOBILABPLUS
  if ( Parameter( "SourceCh" ) > 9 )
    bcierr << "Number of channels (SourceCh) cannot be more than 9." << endl;
#else // GMOBILABPLUS
  if ( Parameter("SourceCh") > 8 && Parameter("DigitalEnable") == 0 )
     bcierr << "Number of channels (SourceCh) cannot be more than 8 when digital inputs are not used." << endl;
  if ( Parameter("SourceCh") != 16 && Parameter("DigitalEnable") == 1 )
     bcierr << "Number of channels (SourceCh) must equal 16 when digital inputs are used (8 analog + 8 digital channels)." << endl;
  if (Parameter("DigitalEnable") == 0 && Parameter("DigitalOutBlock") == 1)
     bcierr << "DigitalEnable must be checked to use the DigitalOutBlock."<<endl;
  // DigitalOutputExpression check
  Expression( Parameter("DigitalOutputEx") ).Evaluate();
#endif // GMOBILABPLUS

  string COMportParam = Parameter( "COMport" );
  if( COMportParam.empty() )
  {
    bcierr << "No COM port specified for device" << endl;
    return;
  }
  string COMport = BuildComPortString( COMportParam );
  HANDLE hDev = ::GT_OpenDevice( const_cast<char*>( COMport.c_str() ) );
  if( hDev == NULL )
  {
    bcierr << "Could not connect to device at COM port " << COMport << endl;
    return;
  }

#ifndef GMOBILABPLUS

#ifdef TODO
# error Perform a device configuration test for the gMOBIlabPlus as well (requires testing).
#endif // TODO

  _AIN ain;
  _DIO dio;

  ain.ain1 = true;
  ain.ain2 = true;
  ain.ain3 = true;
  ain.ain4 = true;
  ain.ain5 = true;
  ain.ain6 = true;
  ain.ain7 = true;
  ain.ain8 = true;
  dio.scan = true;
  dio.dio1_direction = true; // set dio1 to input
  dio.dio2_direction = true; // set dio2 to input

  // try to initialize the device with all channels
  if( !::GT_InitChannels( hDev, ain, dio ) ) // init analog channels and digital lines on g.MOBIlab
    bcierr << "Could not initialize device at COM port " << COMport << endl;

#endif // GMOBILABPLUS

  ::GT_CloseDevice( hDev );

  // Requested output signal properties.
  Output = SignalProperties( sourceCh, sampleBlockSize, SignalType::int16 );
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the gMOBIlabADC.
//             It is called each time parameters have been changed.
// Parameters: References to input (ignored) and output signal properties.
// Returns:    N/A
// **************************************************************************
void
gMOBIlabADC::OnInitialize( const SignalProperties& Output )
{
  OnHalt();

  mTimeoutMs = static_cast<int>( 1000 * MeasurementUnits::SampleBlockDuration() );
  mBufferSize = Output.Channels() * Output.Elements() * sizeof( int16_t );
  mpBuffer = new uint8_t[mBufferSize];

  string COMport = BuildComPortString( Parameter("COMport") );
  mDevice = ::GT_OpenDevice( const_cast<char*>( COMport.c_str() ) );
  if( mDevice == NULL )
  {
#ifndef GMOBILABPLUS
    bcierr << "Could not open device" << endl;
    return;
#else // GMOBILABPLUS
    //there seems to be a bug (or feature...?) where if you open the device
    //after it is already opened, it actually disconnects the bluetooth connection
    //and it needs to be reconnected. This tests for this bug, since it will reconnect
    //on the 2nd try if possible; if there is another problem, it gives the error and returns
    mDevice = ::GT_OpenDevice( const_cast<char*>( COMport.c_str() ) );
    if (mDevice == NULL)
    {
      UINT lastErr;
      ::GT_GetLastError(&lastErr);
      _ERRSTR errorStr;
      ::GT_TranslateErrorCode(&errorStr,lastErr);
      bcierr << "Could not open device: " << errorStr.Error << endl;
      return;
    }
#endif // GMOBILABPLUS
  }

  int numChans = Output.Channels();
  bool useDigInputs = false;

#ifndef GMOBILABPLUS
  useDigInputs = ( numChans > 8 );
#else // GMOBILABPLUS
  useDigInputs = ( Parameter("DigitalEnable") != 0 );
#endif // GMOBILABPLUS

  _AIN ain;
  ain.ain1 = (numChans > 0); // scan
  ain.ain2 = (numChans > 1); // scan
  ain.ain3 = (numChans > 2); // scan
  ain.ain4 = (numChans > 3); // scan
  ain.ain5 = (numChans > 4); // scan
  ain.ain6 = (numChans > 5); // scan
  ain.ain7 = (numChans > 6); // scan
  ain.ain8 = (numChans > 7); // scan

  _DIO dio;
#ifndef GMOBILABPLUS
  dio.scan = (numChans > 8); // scan digital lines
  dio.dio1_direction = true; // set dio1 to input
  dio.dio2_direction = true; // set dio2 to input
#else // GMOBILABPLUS
  dio.dio1_enable = useDigInputs;
  dio.dio2_enable = useDigInputs;
  dio.dio3_enable = useDigInputs;
  dio.dio4_enable = useDigInputs;
  dio.dio5_enable = useDigInputs;
  dio.dio6_enable = useDigInputs;
  dio.dio7_enable = useDigInputs;
  dio.dio8_enable = useDigInputs;

  dio.dio4_direction = true;
  dio.dio5_direction = true;
  dio.dio6_direction = true;
  dio.dio7_direction = true;

  mEnableDigOut = ( Parameter("DigitalOutBlock") == 1 );
  if( mEnableDigOut )
  {
    dio.dio7_direction = false;   //set to digital output
    ::GT_SetDigitalOut(mDevice, 0x11);
  }

  if( string( Parameter("DigitalOutputEx") ) != "" )
  {
    dio.dio4_direction = false; //set to digital output
    mDigExpression = Expression( Parameter("DigitalOutputEx") );
  }

  ::GT_SetTestmode( mDevice, Parameter("MobiLabTestMode") == 1 );
#endif // GMOBILABPLUS

  if( !::GT_InitChannels( mDevice, ain, dio ) ) // init analog channels and digital lines on g.MOBIlab
    bcierr << "Could not initialize device" << endl;
}

// **************************************************************************
// Function:   OnProcess
// Purpose:    This function is called when the main thread enters the
//             Process() function.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
gMOBIlabADC::OnProcess()
{
#ifdef GMOBILABPLUS
 // Expression output
  if( mDigExpression.IsValid() )
  {
    if( mDigExpression.Evaluate() )
      // Set DIO 5 high
      ::GT_SetDigitalOut( mDevice, 0x88 ); // 10001000 = 136 = 0x88 (in hex)
    else
      // Set DIO 5 low
      ::GT_SetDigitalOut( mDevice, 0x80 ); // 10000000 = 128 = 0x80 (in hex)
  }
#endif // GMOBILABPLUS
}

// **************************************************************************
// Function:   OnStartAcquisition
// Purpose:    This function is called before the data acquisition loop
//             it prepares the device for data acquisition.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
gMOBIlabADC::OnStartAcquisition()
{
  ::memset( &mOverlapped, 0, sizeof( mOverlapped ) );
  mOverlapped.hEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );
  if( !::GT_StartAcquisition( mDevice ) )
    bcierr << "Could not start data acquisition" << endl;
}

// **************************************************************************
// Function:   OnStartAcquisition
// Purpose:    This function is called after the data acquisition loop
//             it stops the device for data acquisition.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
gMOBIlabADC::OnStopAcquisition()
{
  ::CloseHandle( mOverlapped.hEvent );
  ::GT_StopAcquisition( mDevice );
}

// **************************************************************************
// Function:   DoAcquire
// Purpose:    This function is called within the data acquisition loop
//             it fills its output signal with values
//             and does not return until all data has been acquired.
// Parameters: Reference to output signal.
// Returns:    N/A
// **************************************************************************
void
gMOBIlabADC::DoAcquire( GenericSignal& Output )
{
#ifdef GMOBILABPLUS
  if (mEnableDigOut)
    // Set DIO 4 low
    ::GT_SetDigitalOut(mDevice, 0x10); // 00010000 = 16 = 0x10 (in hex)
#endif // GMOBILABPLUS

  enum { ok = 0, error = 1 } result = ok;
  int voidLoopCount = 0,
      totalBytesReceived = 0;
  while( totalBytesReceived < mBufferSize && result == ok )
  {
    _BUFFER_ST buf;
    buf.pBuffer = reinterpret_cast<int16_t*>( mpBuffer + totalBytesReceived );
    buf.size = mBufferSize - totalBytesReceived;
    buf.size = min( buf.size, cMaxReadBufSize );
    buf.validPoints = 0;
    if( !::GT_GetData( mDevice, &buf, &mOverlapped ) )
      result = error;
    int dataTimeout = max( cMinDataTimeoutMs, 2 * mTimeoutMs );
    if( WAIT_OBJECT_0 != ::WaitForSingleObject( mOverlapped.hEvent, dataTimeout ) )
      result = error;
    DWORD bytesReceived = 0;
    if( !::GetOverlappedResult( mDevice, &mOverlapped, &bytesReceived, FALSE ) )
      result = error;
    if( bytesReceived == 0 && ++voidLoopCount > cMaxVoidLoopCount )
      result = error;
    totalBytesReceived += bytesReceived;
  }
  if( result == error )
    bcierr << "Could not read data from amplifier" << endl;

  const int cFirstDigChannel = 8;
  int16_t* pData = reinterpret_cast<int16_t*>( mpBuffer );
  for( int sample = 0; sample < Output.Elements(); ++sample )
  {
    for( int channel = 0; channel < min( cFirstDigChannel, Output.Channels() ); ++channel )
      Output( channel, sample ) = *pData++;
    int16_t digitalLines = *pData;

    for( int channel = cFirstDigChannel; channel < Output.Channels(); ++channel )
    {
#ifdef TODO
# error Unify gMOBIlab and gMOBIlabPlus behavior with regard to digital outputs.
#endif // TODO

#ifndef GMOBILABPLUS // For the gMOBIlab, a single channel holds all digital lines.
      Output( channel, sample ) = digitalLines;
#else // GMOBILABPLUS // For the gMOBIlabPlus, we provide digital lines in individual channels.
      // the digital lines are stored in a single sample, with the values in each bit
      // the order is (MSB) 8 7 6 5 2 4 3 1 (LSB)
      const uint16_t mask[] =
      {
        0x0001,
        0x0008, // intentionally out of sequence
        0x0002,
        0x0004,
        0x0010,
        0x0020,
        0x0040,
        0x0080
	  };
      Output( channel, sample ) = ( digitalLines & mask[ channel - cFirstDigChannel ] ) ? 100 : 0;
#endif // GMOBILABPLUS
    }
  }

#ifdef GMOBILABPLUS
  if (mEnableDigOut)
    // Set DIO 4 high
    ::GT_SetDigitalOut(mDevice, 0x11);
#endif // GMOBILABPLUS
}

// **************************************************************************
// Function:   OnHalt
// Purpose:    This routine shuts down data acquisition
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
gMOBIlabADC::OnHalt()
{
  if( mDevice )
  {
    ::GT_StopAcquisition( mDevice );
    ::GT_CloseDevice( mDevice );
    mDevice = NULL;
  }
  delete[] mpBuffer;
  mpBuffer = NULL;
}


string
gMOBIlabADC::BuildComPortString( const std::string& inParameter )
{
  string result = inParameter;
  if( ::stricmp( result.substr( 0, 3 ).c_str(), "COM" ) == 0 )
    result = result.substr( 3 );
  if( !result.empty() && *result.rbegin() == ':' )
    result = result.substr( 0, result.length() - 1 );
  result = "COM" + result + ":";
  return result;
}
