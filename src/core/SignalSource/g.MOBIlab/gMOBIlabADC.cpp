////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org
// Description: BCI2000 Source Module for gMOBIlab devices.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "gMOBIlabADC.h"

using namespace std;

// Register the source class with the framework.
RegisterFilter( gMOBIlabADC, 1 );

// **************************************************************************
// Function:   gMOBIlabADC
// Purpose:    The constructor for the gMOBIlabADC.
// **************************************************************************
gMOBIlabADC::gMOBIlabADC()
: mpBuffer( NULL ),
  mBufsize( 0 ),
  mNumChans( 0 ),
  mEvent( NULL ),
  mDev( NULL )
{
 ::memset( &mOv, 0, sizeof( mOv ) );

 // add all the parameters that this ADC requests to the parameter list
 BEGIN_PARAMETER_DEFINITIONS
   "Source string COMport=      COM2: COM2: a z "
       "// COMport for MOBIlab",
   "Source int SourceCh=      16 16 1 128 "
       "// number of digitized channels total",
   "Source int SampleBlockSize= 8 5 1 128 "
       "// number of samples per block",
   "Source int SamplingRate=    256 256 1 40000 "
       "// the signal sampling rate",
 END_PARAMETER_DEFINITIONS

  // set the priority of the Source module a little higher
  // (since its output clocks the whole system)
  // this really seems to help against data loss; even under extreme load,
  // data acquisition continues even though the display might be jerky
  SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
}


gMOBIlabADC::~gMOBIlabADC()
{
  Halt();
  if( mEvent )
    CloseHandle( mEvent );

}


// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally required properties
//             for the output signal; checks whether resources are available.
// Parameters: References to input and output signal properties.
// Returns:    N/A
// **************************************************************************
void gMOBIlabADC::Preflight( const SignalProperties&,
                                   SignalProperties& Output ) const
{
  int sourceCh = Parameter( "SourceCh" ),
      sampleBlockSize = Parameter( "SampleBlockSize" );
      
  if ( Parameter("SamplingRate") != 256 )
     bcierr << "MOBIlab sampling rate is fixed at 256 Hz. Change SamplingRate parameter to 256." << endl;
  if ( Parameter("SourceCh") < 1 )
     bcierr << "Number of channels (SourceCh) has to be at least 1." << endl;
  if ( Parameter("SourceCh") > 9 )
     bcierr << "Number of channels (SourceCh) cannot be more than 9." << endl;

  // buffersize can't be > 1024
  int numSamplesPerScan = sourceCh * sampleBlockSize;
  if ( numSamplesPerScan * sizeof( sint16 ) > 1024 )
     bcierr << "Buffer size cannot be larger than 1024 bytes. Please decrease the SourceCh or SampleBlocksize." << endl;

  string COMport = Parameter("COMport");
  HANDLE hDev = GT_OpenDevice((LPSTR)COMport.c_str());
  if (hDev == NULL)
     bcierr << "Could not open device at COM port " << COMport << endl;

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
  bool ret = GT_InitChannels(hDev, ain, dio); // init analog channels and digital lines on g.MOBIlab
  if (!ret) bcierr << "Could not initialize device at COM port " << COMport << endl;

  GT_CloseDevice(hDev);

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
void gMOBIlabADC::Initialize( const SignalProperties&,
                              const SignalProperties& Output )
{
  mNumChans = Output.Channels();
  int numSamplesPerScan = mNumChans * Output.Elements();
  mBufsize = numSamplesPerScan * sizeof( sint16 );
  delete[] mpBuffer;
  mpBuffer = new sint16[numSamplesPerScan];

  if( mEvent )
    CloseHandle( mEvent );
  mEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
  mOv.hEvent = mEvent;
  mOv.Offset = 0;
  mOv.OffsetHigh = 0;

  mDev = GT_OpenDevice((LPSTR)Parameter("COMport").c_str());
  if (mDev == NULL)
  {
   bcierr << "Could not open device" << endl;
   return;
  }

  _AIN ain;
  _DIO dio;

  ain.ain1 = (mNumChans > 0); // scan
  ain.ain2 = (mNumChans > 1); // scan
  ain.ain3 = (mNumChans > 2); // scan
  ain.ain4 = (mNumChans > 3); // scan
  ain.ain5 = (mNumChans > 4); // scan
  ain.ain6 = (mNumChans > 5); // scan
  ain.ain7 = (mNumChans > 6); // scan
  ain.ain8 = (mNumChans > 7); // scan
  dio.scan = (mNumChans > 8); // scan digital lines

  dio.dio1_direction = true; // set dio1 to input
  dio.dio2_direction = true; // set dio2 to input

  bool ret = GT_InitChannels(mDev, ain, dio); // init analog channels and digital lines on g.MOBIlab
  if (!ret)
  {
     bcierr << "Could not initialize device" << endl;
     return;
  }

  ret = GT_StartAcquisition(mDev);
  if (!ret)
     bcierr << "Could not start data acquisition" << endl;
}


// **************************************************************************
// Function:   Process
// Purpose:    This function is called within the data acquisition loop
//             it fills its output signal with values
//             and does not return until all data has been acquired.
// Parameters: References to input signal (ignored) and output signal.
// Returns:    N/A
// **************************************************************************
void gMOBIlabADC::Process( const GenericSignal&, GenericSignal& Output )
{
  DWORD dwBytesReceived = 0;
  int   totalbytesreceived = 0;
  while (totalbytesreceived < mBufsize)
  {
    _BUFFER_ST buf;
    uint8* p = reinterpret_cast<uint8*>( mpBuffer );
    buf.pBuffer = reinterpret_cast<SHORT*>( p + totalbytesreceived );
    buf.size = mBufsize - totalbytesreceived;
    buf.validPoints = 0;
    bool ret = GT_GetData(mDev, &buf, &mOv); // extract data from driver
    if ( !ret )
    {
      bcierr << "Unexpected fatal error on GT_GetData()" << endl;
      return;
    }
    GetOverlappedResult(mDev, &mOv, &dwBytesReceived, TRUE);
    totalbytesreceived += dwBytesReceived;
  }

  for( int channel = 0; channel < mNumChans; ++channel )
    for( int sample = 0; sample < Output.Elements(); ++sample )
      Output( channel, sample ) = mpBuffer[ mNumChans * sample + channel ];
}


// **************************************************************************
// Function:   Halt
// Purpose:    This routine shuts down data acquisition
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void gMOBIlabADC::Halt()
{
  if (mDev)
  {
    GT_StopAcquisition(mDev);
    GT_CloseDevice(mDev);
    mDev = NULL;
  }

  delete[] mpBuffer;
  mpBuffer = NULL;
}

