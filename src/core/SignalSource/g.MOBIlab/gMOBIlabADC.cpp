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
#include "spa20a.h"

using namespace std;

// Register the source class with the framework.
RegisterFilter( gMOBIlabADC, 1 );

const int cBufBlocks = 32;    // Size of data ring buffer in terms of sample blocks.
const int cMaxReadBuf = 1024; // Restriction of gtec driver.

// **************************************************************************
// Function:   gMOBIlabADC
// Purpose:    The constructor for the gMOBIlabADC.
// **************************************************************************
gMOBIlabADC::gMOBIlabADC()
: mDev( NULL ),
  mpAcquisitionQueue( NULL )
{
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
}


gMOBIlabADC::~gMOBIlabADC()
{
  Halt();
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

  string COMport = Parameter("COMport");
  HANDLE hDev = GT_OpenDevice( const_cast<char*>( COMport.c_str() ) );
  if (hDev == NULL)
  {
    bcierr << "Could not open device at COM port " << COMport << endl;
    return;
  }

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
  delete mpAcquisitionQueue;
  mpAcquisitionQueue = NULL;

  mDev = ::GT_OpenDevice( const_cast<char*>( Parameter("COMport").c_str() ) );
  if (mDev == NULL)
  {
    bcierr << "Could not open device" << endl;
    return;
  }

  _AIN ain;
  _DIO dio;

  int numChans = Output.Channels();
  ain.ain1 = (numChans > 0); // scan
  ain.ain2 = (numChans > 1); // scan
  ain.ain3 = (numChans > 2); // scan
  ain.ain4 = (numChans > 3); // scan
  ain.ain5 = (numChans > 4); // scan
  ain.ain6 = (numChans > 5); // scan
  ain.ain7 = (numChans > 6); // scan
  ain.ain8 = (numChans > 7); // scan
  dio.scan = (numChans > 8); // scan digital lines

  dio.dio1_direction = true; // set dio1 to input
  dio.dio2_direction = true; // set dio2 to input

  bool ret = ::GT_InitChannels(mDev, ain, dio); // init analog channels and digital lines on g.MOBIlab
  if (!ret)
  {
     bcierr << "Could not initialize device" << endl;
     return;
  }

  ret = ::GT_StartAcquisition(mDev);
  if (!ret)
  {
    bcierr << "Could not start data acquisition" << endl;
    return;
  }

  int numSamplesPerScan = numChans * Output.Elements(),
      blockSize = numSamplesPerScan * sizeof( sint16 ),
      blockDuration = 1e3 * Output.Elements() / Parameter( "SamplingRate" );
  mpAcquisitionQueue = new DAQueue( blockSize, blockDuration, mDev );
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
  for( int sample = 0; sample < Output.Elements(); ++sample )
    for( int channel = 0; channel < Output.Channels(); ++channel )
      Output( channel, sample ) = mpAcquisitionQueue->Consume();
}


// **************************************************************************
// Function:   Halt
// Purpose:    This routine shuts down data acquisition
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void gMOBIlabADC::Halt()
{
  delete mpAcquisitionQueue;
  mpAcquisitionQueue = NULL;
  
  if( mDev )
  {
    ::GT_StopAcquisition( mDev );
    ::GT_CloseDevice( mDev );
    mDev = NULL;
  }
}

// DAQueue runs a separate data acquisition thread.
// The data buffer is filled from the separate thread, and is emptied from
// the main thread.
gMOBIlabADC::DAQueue::DAQueue( int inBlockSize, int inTimeout, HANDLE inDevice )
: OSThread( true ),
  mBlockSize( inBlockSize ),
  mTimeout( inTimeout ),
  mBufSize( inBlockSize * cBufBlocks ),
  mWriteCursor( 0 ),
  mReadCursor( 0 ),
  mpBuffer( NULL ),
  mEvent( NULL ),
  mDev( inDevice )
{
  ::memset( &mOv, 0, sizeof( mOv ) );
  mEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );
  mOv.hEvent = mEvent;
  mOv.Offset = 0;
  mOv.OffsetHigh = 0;
  mpBuffer = new uint8[mBufSize];
  this->Resume();
}

gMOBIlabADC::DAQueue::~DAQueue()
{
  ::CloseHandle( mEvent );
  delete[] mpBuffer;
}

void
gMOBIlabADC::DAQueue::AcquireLock()
{
  while( mLock )
    ::Sleep( 0 );
  mLock = true;
}

void
gMOBIlabADC::DAQueue::ReleaseLock()
{
  mLock = false;
}

sint16
gMOBIlabADC::DAQueue::Consume()
{
  while( mReadCursor == mWriteCursor )
    ::WaitForSingleObject( mEvent, mTimeout );
  sint16 value = *reinterpret_cast<sint16*>( mpBuffer + mReadCursor );
  mReadCursor += sizeof( sint16 );
  mReadCursor %= mBufSize;
  return value;
}

int
gMOBIlabADC::DAQueue::Execute()
{
  enum { ok = 0, errorOccurred = 1 };

  while( !this->IsTerminating() )
  {
    DWORD bytesReceived = 0;
    while( mWriteCursor < mBufSize )
    {
      _BUFFER_ST buf;
       buf.pBuffer = reinterpret_cast<SHORT*>( mpBuffer + mWriteCursor );
      buf.size = min( mBlockSize, mBufSize - mWriteCursor );
      buf.size = min( buf.size, cMaxReadBuf );
      buf.validPoints = 0;
      if( !::GT_GetData( mDev, &buf, &mOv ) ) // extract data from driver
        return errorOccurred;
      ::GetOverlappedResult( mDev, &mOv, &bytesReceived, TRUE );
      mWriteCursor += bytesReceived;
    }
    if( mWriteCursor > mBufSize )
      return errorOccurred;
    mWriteCursor = 0;
  }
  return ok;
}


