////////////////////////////////////////////////////////////////////////////////
// $Id: $
// Authors: griffin.milsap@gmail.com
// Description: A class which manages a gHIamp device
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

#include "gHIampDevice.h"
#include "BCIError.h"

using namespace std;

#define QUEUE_SIZE 4
string GetDeviceErrorMessage();

void
gHIampDevice::Init( HANDLE device )
{
  mDevice = device;
  if( !GT_GetSerial( mDevice, mSerial, 16 ) )
    bcierr << "Could not retreive serial from gHIamp device. "
           << GetDeviceErrorMessage() << endl;
  if( !GT_GetConfiguration( mDevice, &mConfig ) )
    bcierr << "Could not get configuration from gHIamp: serial " << mSerial
           << GetDeviceErrorMessage() << endl;
  mpBuffers = NULL;
  mpOverlapped = NULL;

  // Initial configuration
  for( size_t i = 0; i < 256; i++ )
  {
    mConfig.Channels[i].Acquire = true;
    mConfig.Channels[i].BandpassFilterIndex = -1;
    mConfig.Channels[i].NotchFilterIndex = -1;
  }
  mConfig.TriggerLineEnabled = true;
  mConfig.IsSlave = true;
  mConfig.Mode = M_NORMAL;
  mConfigured = false;
  mRefIdx = -1;
}

void
gHIampDevice::Cleanup()
{
  for( size_t i = 0; i < QUEUE_SIZE; i++ )
  {
    if( mpOverlapped ) WaitForSingleObject( mpOverlapped[i].hEvent, 1000 );
	if( mpOverlapped ) CloseHandle( mpOverlapped[i].hEvent );
    if( mpBuffers ) delete [] mpBuffers[i];
  }
  delete [] mpBuffers; mpBuffers = NULL;
  delete [] mpOverlapped; mpOverlapped = NULL;
}

// Allocate memory needed for the driver to save data
void
gHIampDevice::BeginAcquisition()
{
  if( !mConfigured )
    bcierr << "gHIamp " << Serial() << " has not been configured and cannot be acquired from." << endl;
  Cleanup();

  // Determine the number of channels we should acquire
  mNumChannels = 256 + 1;
  int nPoints = mNumScans * mNumChannels;
  mBufferSizeBytes = nPoints * sizeof( float );
  mpBuffers = new BYTE*[ QUEUE_SIZE ];
  mpOverlapped = new OVERLAPPED[ QUEUE_SIZE ];
  for( size_t i = 0; i < QUEUE_SIZE; i++ )
  {
    mpBuffers[i] = new BYTE[ mBufferSizeBytes ];
    SecureZeroMemory( &( mpOverlapped[i] ), sizeof( OVERLAPPED ) );
    mpOverlapped[i].hEvent = CreateEvent( NULL, false, false, NULL );
  }

  // Start the device
  if( !GT_Start( mDevice ) )
    bcierr << "Error starting acquisition from gHIamp device: serial " << Serial() << endl
           << GetDeviceErrorMessage() << endl;

  // Queue transfer requests
  for( size_t i = 0; i < QUEUE_SIZE; i++ )
    if( !GT_GetData( mDevice, mpBuffers[i], mBufferSizeBytes, &mpOverlapped[i] ) )
      bcierr << "Error sending transfer request to gHIamp device: serial " << Serial() << endl
             << GetDeviceErrorMessage() << endl;

  mQueueIndex = 0;
}

void
gHIampDevice::GetData( GenericSignal &Output )
{
  // Block until new data is accessable
  if( WaitForSingleObject( mpOverlapped[mQueueIndex].hEvent, 1000 ) == WAIT_TIMEOUT )
    bcierr << "Timeout occurred while waiting for data from gHIamp: serial " << Serial() << endl
           << GetDeviceErrorMessage() << endl;

  // Figure out how many bytes have been received
  DWORD numBytesReceived = 0;
  if( !GT_GetOverlappedResult( mDevice, &mpOverlapped[mQueueIndex], &numBytesReceived, false ) )
    bcierr << "Could not determine number of transferred bytes from gHIamp: serial " << Serial()
           << ", Windows error code: " << GetLastError << endl;

  // Check if any data has been lost
  if( numBytesReceived != mBufferSizeBytes )
    bcierr << "Error on Data transfer from gHIamp: serial " << Serial()
           << " -- Samples have been lost." << endl;

  // Fill the output as necessary
  float* data = reinterpret_cast< float* >( mpBuffers[mQueueIndex] );
  for( int sample = 0; sample < mNumScans; sample++ )
  {
    map< int, int >::iterator itr = mAnalogChannelMap.begin();
    int idx = 0;
    if( mRefIdx == -1 ) // Unreferenced Mode
      for( ; itr != mAnalogChannelMap.end(); itr++, idx++ )
        Output( itr->second, sample ) = data[ ( mNumChannels * sample ) + itr->first ];
    else // Referenced Mode
      for( ; itr != mAnalogChannelMap.end(); itr++, idx++ )
        Output( itr->second, sample ) = data[ ( mNumChannels * sample ) + itr->first ]
                                      - data[ ( mNumChannels * sample ) + mRefIdx];

    // This might break in the future.  This should really be a uint16_t
    unsigned short digital = ( unsigned short )( mpBuffers[mQueueIndex][ 256 * ( sample + 1 ) ] );
    itr = mDigitalChannelMap.begin();
    for( ; itr != mDigitalChannelMap.end(); itr++ )
    {
      unsigned short mask = 1 << itr->first;
      Output( itr->second, sample ) = ( digital & mask ) ? 100.0f : 0.0f;
    }
  }

  // Queue a new GetData call to replace the one which just expired
  if( !GT_GetData( mDevice, mpBuffers[mQueueIndex], mBufferSizeBytes, &mpOverlapped[mQueueIndex] ) )
    bcierr << "Unable to queue another data request for gHIamp: serial " << Serial() << endl
           << GetDeviceErrorMessage() << endl;
			
  // Move to the next data request
  mQueueIndex = ( mQueueIndex + 1 ) % QUEUE_SIZE;
}

void 
gHIampDevice::EndAcquisition()
{
  // Attempt to stop the device
  if( !GT_Stop( mDevice ) )
    bcierr << "Error trying to stop acquisition from gHIamp: serial " << Serial() << endl
           << GetDeviceErrorMessage() << endl;

  // Clean up allocated resources
  Cleanup();
}

// Try to map numch available analog channels on this device 
// starting with output channel startch and return the number of
// mapped channels
int 
gHIampDevice::MapAllAnalogChannels( int startch, int numch )
{
  int numMapped = 0;
  for( int i = 0; i < numch; i++ )
    if( MapAnalogChannel( i, startch + i, false ) )
      numMapped++;
    else
      break;
  return numMapped;
}

bool
gHIampDevice::MapAnalogChannel( unsigned int devicech, unsigned int sourcech, bool err )
{
  if( devicech > 256 )
  {
    if( err ) bcierr << "Requested channel " << devicech + 1 
                     << " from g.HIamp which only has 256 channels" << endl;
    return false;
  }
  if( mAnalogChannelMap.find( devicech ) != mAnalogChannelMap.end() )
  {
    if( err ) bcierr << "Channel already mapped." << endl;
    return false;
  }
  if( !mConfig.Channels[devicech].Available )
  {
    if( err ) bciout << "Channel " << devicech + 1 << " on amp: " 
                     << Serial() << " is not available." << endl;
    return false;
  }
  mAnalogChannelMap[ devicech ] = sourcech;
  return true;
}

bool
gHIampDevice::MapDigitalChannel( unsigned int devicech, unsigned int sourcech )
{
  if( devicech > 16 )
  {
    bcierr << "Requested digital channel " << devicech + 1 
           << " from g.HIamp which only has 16 digital channels" << endl;
    return false;
  }
  if( mDigitalChannelMap.find( devicech ) != mDigitalChannelMap.end() )
  {
    bcierr << "Channel already mapped." << endl;
    return false;
  }
  mDigitalChannelMap[ devicech ] = sourcech;
  return true;
}

bool gHIampDevice::SetRefChan( int devicech ) 
{ 
  if( devicech > 256 )
    return false;
  if( !mConfig.Channels[ devicech ].Available )
    return false;
  mRefIdx = devicech;
  return true;
}

void
gHIampDevice::SetNotch( int iNotchNo )
{
  for( size_t i = 0; i < 256; i++ )
    mConfig.Channels[i].NotchFilterIndex = iNotchNo;
}

void
gHIampDevice::SetFilter( int iFilterNo )
{
  for( size_t i = 0; i < 256; i++ )
    mConfig.Channels[i].BandpassFilterIndex = iFilterNo;
}

void
gHIampDevice::SetConfiguration( int iSampleRate, int iSampleBlockSize )
{
  // Configure the amp to work with our system setup
  mNumScans = iSampleBlockSize;
  mConfig.SampleRate = iSampleRate;
  mConfig.BufferSize = mNumScans;
  mConfig.HoldEnabled = false;
  mConfig.Mode = M_NORMAL;

  // Configure the Internal Signal Generator
  mConfig.InternalSignalGenerator.WaveShape = WS_SQUARE;
  mConfig.InternalSignalGenerator.Frequency = 10; // Hz
  mConfig.InternalSignalGenerator.Amplitude = 10000; // muV
  mConfig.InternalSignalGenerator.Offset = 0; // muV

  // Configure device channels
  for( int j = 0; j < 256; j++ )
    if( mConfig.Channels[j].Available )
      mConfig.Channels[j].BipolarChannel = 0;

  // Set the digital input flag accordingly
  if( !GT_SetConfiguration( mDevice, mConfig ) )
    bcierr << "Could not set configuration for gHIamp device: serial " << Serial() << endl
           << GetDeviceErrorMessage() << endl;

  mConfigured = true;
}

// **************************************************************************
// DeviceContainer
// Purpose: This class auto-detects, opens, and closes gHIamp devices
// **************************************************************************
bool 
gHIampDeviceContainer::Detect()
{
  Close();
  for( int port = 0; port < 16; port++ )
  {
    HANDLE hdev = GT_OpenDevice( port );
    if( hdev )
      this->push_back( gHIampDevice( hdev ) );
  }
  return !this->empty();
}

void
gHIampDeviceContainer::Close()
{
  for( gHIampDeviceContainer::iterator itr = begin(); itr != end(); itr++ )
    if( !GT_CloseDevice( &( itr->mDevice ) ) )
      bcierr << "Could not close gHIamp device: serial " << itr->Serial() << endl
             << GetDeviceErrorMessage() << endl;
  clear();
}

void
gHIampDeviceContainer::Remove( gHIampDeviceContainer::iterator& itr )
{
  if( !GT_CloseDevice( &( itr->mDevice ) ) )
      bcierr << "Could not close gHIamp device: serial " << itr->Serial() << endl
             << GetDeviceErrorMessage() << endl;
  erase( itr );
  itr--;
}

// **************************************************************************
// Function:   GetDeviceErrorMessage
// Purpose:    Retreives a formatted error string from the gHIamp device API
// Parameters: N/A
// Returns:    std::string formatted error
// **************************************************************************
string 
GetDeviceErrorMessage()
{
  WORD errorCode = 0;
  char errorMessage[256];

  if( !GT_GetLastError( &errorCode, errorMessage ) )
    return string( "(reason unknown: error code could not be retrieved from device)" );

  char exceptionMessage[512];
  sprintf_s( exceptionMessage, 512, "(#%d: %s)", errorCode, errorMessage );

  return string( exceptionMessage );
}
