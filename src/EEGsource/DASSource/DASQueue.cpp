////////////////////////////////////////////////////////////////////////////////
//
// File: DASQueue.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Sep 23, 2003
//
// Description: A class that interfaces with A/D boards supported by
//              MeasurementComputing's Universal Library. 
//
////////////////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "DASQueue.h"

#include "DAS_lib/cbw.h"
#include "DAS_lib/DASUtils.h"
#include "UBCIError.h"
#include <sstream>
#include <typeinfo>

using namespace std;

DASQueue::DASQueue()
: mFailstate( ok ),
  mBoardNumber( NOTUSED ),
  mDataBuffer( NULL ),
  mDataBufferSize( 0 ),
  mReadCursor( 0 ),
  mWriteCursor( 0 ),
  mFreqMultiplier( 1 ),
  mChannels( 0 ),
  mShouldBeOpen( false ),
  mTimeoutInterval( 0 ),
  mDataAvailableEvent( ::CreateEvent( NULL, FALSE, FALSE, NULL ) )
{
  float currentRevNum = CURRENTREVNUM;
  ::cbDeclareRevision( &currentRevNum );
  ::cbErrHandling( DONTPRINT, DONTSTOP );
}

DASQueue::~DASQueue()
{
  close();
  ::CloseHandle( mDataAvailableEvent );
}

void
DASQueue::open( const DASInfo& inInfo )
{
  if( mFailstate != ok )
    return;

  if( is_open() )
    close();

  mShouldBeOpen = true;
  mBoardNumber = inInfo.boardNumber;
  mTimeoutInterval = cTimeoutFactor * ( 1000 * inInfo.sampleBlockSize ) / inInfo.samplingRate;
  if( mTimeoutInterval < 1 )
    mTimeoutInterval = 1;
    
  // Check whether there are version conflicts between DLL and board driver.
  float ignored;
  int result = ::cbGetRevision( &ignored, &ignored );
  if( result == NOERRORS )
  {
    int ADRange = DASUtils::GetADRangeCode( inInfo.adRangeMin, inInfo.adRangeMax ),
        hwRange = NOTUSED;
    result = ::cbGetConfig( BOARDINFO, mBoardNumber, 0, BIRANGE, &hwRange );
    if( result == NOERRORS )
    {
      if( hwRange != NOTUSED && hwRange != ADRange )
      {
        bcierr << "The specified A/D range does not match the associated "
                  "hardware switch setting" << endl;
        ADRange = hwRange;
      }

      int hwChannels = 0;
      mChannels = inInfo.numChannels;
      result = ::cbGetConfig( BOARDINFO, mBoardNumber, 0, BINUMADCHANS, &hwChannels );
      if( result == NOERRORS )
      {
        if( hwChannels < mChannels )
        {
          bcierr << "Requested number of channels exceeds number of available"
                 << " hardware channels" << endl;
          mChannels = hwChannels;
        }

        long hwBlockSize;
        result = DASUtils::GetTransferBlockSize( mBoardNumber, hwBlockSize );
        if( result != NOERRORS )
          bcierr << "Could not measure the board's transfer block size" << endl;
        else if( !DASUtils::IsPowerOf2( hwBlockSize ) )
          bciout << "The board's transfer block size ("
                 << hwBlockSize << ") is not a power of 2.\n"
                 << "For non-demo boards this may indicate a problem"
                 << " with the method used to measure the block size"
                 << endl;

        // For large hardware buffers, we need to increase the sampling rate to
        // obtain the block update rate implied by the parameters.
        mFreqMultiplier = ( 2 * hwBlockSize + 1 ) / ( 2 * mChannels * inInfo.sampleBlockSize );
        mDataBufferSize = cBlocksInBuffer * hwBlockSize;

        long hwSamplingRate = inInfo.samplingRate * mFreqMultiplier;
        int  options = CONTINUOUS | BACKGROUND;
        result = DASUtils::GetBoardOptions( mBoardNumber, mChannels,
                                            mDataBufferSize, hwSamplingRate,
                                            ADRange, options );
        if( result == NOERRORS )
        {
          // The board may have changed the sample rate.
          if( hwSamplingRate / mFreqMultiplier != inInfo.samplingRate )
            bcierr << "Sampling rate not supported by A/D board"
                   << " (try " << hwSamplingRate / mFreqMultiplier << "/s)"
                   << endl;
          // A memory allocation failure will trigger an error below.
          mDataBuffer = ( USHORT* )::cbWinBufAlloc( mDataBufferSize );
          mReadCursor = 0;
          mWriteCursor = 0;
          result = DASUtils::BackgroundScan( mBoardNumber, 0, mChannels - 1,
                                             mDataBufferSize, hwSamplingRate,
                                             ADRange, mDataBuffer, options,
                                             BoardEventCallback, this );
          if( result == NOERRORS )
            ReceiveData();
        }
      }
    }
  }
  if( result != NOERRORS )
  {
    mFailstate |= initFail;
    bcierr << "A/D error: "
           << DASUtils::GetErrorMessage( result )
           << endl;
  }
}

void
DASQueue::close()
{
  DASUtils::StopBackground( mBoardNumber );
  ::cbWinBufFree( mDataBuffer );
  mDataBuffer = NULL;
  mReadCursor = 0;
  mWriteCursor = 0;
  mShouldBeOpen = false;
}

const short&
DASQueue::front()
{
  while( empty() && mFailstate == ok )
    ReceiveData();
  if( empty() )
  {
    static short null = 0;
    return null;
  }
  return std::queue<short>::front();
}

// This is a _blocking_ function that waits until at least one sample arrived.
// We modify the queue here and not directly in the callback because the
// callback runs in a different thread and thus might access the queue data
// concurrently.
void
DASQueue::ReceiveData()
{
  if( ::WaitForSingleObject( mDataAvailableEvent, mTimeoutInterval ) == WAIT_OBJECT_0 )
  {
    for( ; mReadCursor != mWriteCursor; mReadCursor = ( mReadCursor + 1 ) % mDataBufferSize )
      if( !IgnoredSample( mReadCursor ) )
        push( mDataBuffer[ mReadCursor ] - ( 1 << 15 ) );
  }
}

inline
bool
DASQueue::IgnoredSample( long inIndex )
{
  long posInQuasiSample = inIndex % ( mFreqMultiplier * mChannels );
  return posInQuasiSample < ( mFreqMultiplier - 1 ) * mChannels; 
}

// This is not a member function but we get an instance pointer in the
// user data argument and cast it to a quasi-this pointer.
void
CALLBACK
DASQueue::BoardEventCallback( int      inBoardNumber,
                              unsigned inEventType,
                              unsigned inEventData,
                              void*    inUserData )
{
  DASQueue* _this = static_cast<DASQueue*>( inUserData );
  bool saneArguments = ( _this != NULL && inBoardNumber == _this->mBoardNumber );
  if( saneArguments )
  {
    switch( inEventType )
    {
      case ON_DATA_AVAILABLE:
        _this->mWriteCursor = inEventData % _this->mDataBufferSize;
        ::SetEvent( _this->mDataAvailableEvent );
        break;
      case ON_SCAN_ERROR:
        {
          bcierr << "A/D error: "
                 << DASUtils::GetErrorMessage( inEventData )
                 << endl;
          DASUtils::StopBackground( inBoardNumber );
        }
        break;
      default:
        saneArguments = false;
    }
  }
  if( !saneArguments )
    bcierr << "A/D callback function called with inconsistent arguments"
           << endl;
}

