////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A C++ wrapper for PortAudio streams, providing a functional
//   blocking interface independently of the host api.
//
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
#include "PortAudioStream.h"
#include <limits>
#include <inttypes.h>

using namespace std;

int PortAudioStream::sInstanceCount = 0;

PortAudioStream::PortAudioStream( PaDeviceIndex inDevice, const SignalProperties& inProperties )
: mpStream( 0 ), mpAllocatedBuffer( 0 ), mpBuffer( 0 ), mTimeout( 1000 ), mProperties( inProperties )
{
  if( sInstanceCount++ == 0 )
    if( IsError( Pa_Initialize() ) )
      return;

  ::memset( &mParameters, 0, sizeof( mParameters ) );
  ::memset( &mHostApi, 0, sizeof( mHostApi ) );

  const PaDeviceInfo* pInfo = Pa_GetDeviceInfo( inDevice );
  if( !pInfo )
  {
    mError = "Invalid device";
    return;
  }
  mParameters.device = inDevice;
  mParameters.channelCount = mProperties.Channels();
  mParameters.sampleFormat = paInt16;
  mParameters.suggestedLatency = pInfo->defaultHighInputLatency;
  const PaHostApiInfo* pApiInfo = Pa_GetHostApiInfo( pInfo->hostApi );
  if( !pApiInfo )
  {
    mError = "Invalid host api";
    return;
  }
  InitHostApi( pApiInfo->type );

  if( mProperties.ElementUnit().Symbol() != "s" )
  {
    double rate = pInfo->defaultSampleRate;
    if( rate < numeric_limits<double>::epsilon() )
      rate = 1;
    mProperties.ElementUnit().SetOffset( 0 ).SetGain( 1 / rate ).SetSymbol( "s" );
  }
  if( IsError( Pa_IsFormatSupported( &mParameters, 0, SamplingRate() ) ) )
    return;
  if( mProperties.Elements() < 1 )
    mProperties.SetElements( NativeBlockSize() );
  if( mProperties.Elements() < 1 )
    return;
  CreateStreamObject();
}

PortAudioStream::~PortAudioStream()
{
  DeleteStreamObject();
  if( --sInstanceCount == 0 )
    Pa_Terminate();
}

bool
PortAudioStream::Start()
{
  return !IsError( mHostApi.Start( mpStream, mHostApi.privateData ) );
}

bool
PortAudioStream::Stop()
{
  return !IsError( mHostApi.Stop( mpStream, mHostApi.privateData ) );
}

bool
PortAudioStream::Read( GenericSignal& Output )
{
  const int16_t* data = 0;
  if( IsError( mHostApi.ReadBegin( mpStream, mHostApi.privateData, &data ) ) )
    return false;
  size_t idx = 0;
  for( int el = 0; el < Output.Elements(); el++ )
    for( int ch = 0; ch < Output.Channels(); ch++ )
      Output( ch, el ) = data[idx++];
  if( mHostApi.ReadEnd )
    return !IsError( mHostApi.ReadEnd( mpStream, mHostApi.privateData ) );
  return true;
}

bool
PortAudioStream::IsError( PaError inResult )
{
  if( inResult == paNoError )
    return false;
  mError = Pa_GetErrorText( inResult );
  const PaHostErrorInfo* pInfo = Pa_GetLastHostErrorInfo();
  if( pInfo->hostApiType == mHostApi.id && pInfo->errorCode == inResult && *pInfo->errorText )
  {
    int idx = Pa_HostApiTypeIdToHostApiIndex( PaHostApiTypeId( mHostApi.id ) );
    const PaHostApiInfo* pApiInfo = Pa_GetHostApiInfo( idx );
    if( pApiInfo )
    {
      string name = pApiInfo->name;
      name += " host API";
      if( inResult == paUnanticipatedHostError )
        mError = name;
      else
        mError = name + ": " + mError;
    }
    mError += ": ";
    mError += pInfo->errorText;
  }
  return true;
}

double
PortAudioStream::SamplingRate() const
{
  return mProperties.ElementUnit().PhysicalToRaw( "1s" );
}

namespace {
struct Measure
{
  static const int maxCalls = 15;
  Measure()
    : mCalls( 0 ), mRuns( 0 ), mPrev( 0 ), mResult( 0 )
    {}
  int Result() const
   { return mResult; }

  static int Callback(
    const void*, void*, unsigned long inFrames,
    const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags inFlags,
    void* inInstance )
  {
    Measure* this_ = reinterpret_cast<Measure*>( inInstance );
    if( ++this_->mCalls > maxCalls )
    {
      if( this_->mRuns == 1 )
        this_->mResult = inFrames;
      else
        this_->mResult = 0;
      return paAbort;
    }
    if( inFrames != this_->mPrev )
    {
      this_->mPrev = inFrames;
      ++this_->mRuns;
      this_->mCalls = 0;
    }
    switch( this_->mRuns )
    {
      case 4:
        return paAbort;
      case 3:
      case 2:
        this_->mResult += inFrames;
      case 1:
        return paContinue;
    }
    return paAbort;
  }

 private:
  int mCalls, mRuns, mPrev, mResult;
};
}

unsigned long
PortAudioStream::NativeBlockSize()
{
  mError.clear();
  Measure measure;
  unsigned long blockSize = 0;

  PaStream* pStream = 0;
  PaError result = Pa_OpenStream(
    &pStream, &mParameters, 0, SamplingRate(),
    paFramesPerBufferUnspecified,
    0,
    &Measure::Callback, &measure );
  if( result == paNoError )
  {
    if( mHostApi.Measure )
      mHostApi.Measure( pStream, &blockSize );
    if( blockSize == 0 )
    {
      Pa_StartStream( pStream );
      while( Pa_IsStreamActive( pStream ) )
        Pa_Sleep( 10 );
      Pa_CloseStream( pStream );
      blockSize = measure.Result();
    }
  }
  return blockSize;
}

void
PortAudioStream::CreateStreamObject()
{
  if( mHostApi.ReadBegin == &PaReadSync )
  {
    mpAllocatedBuffer = new int16_t[mProperties.Channels() * mProperties.Elements()];
    mpBuffer = mpAllocatedBuffer;
  }
  PaError result = Pa_OpenStream( &mpStream, &mParameters, 0, SamplingRate(), mProperties.Elements(), 0, mHostApi.callback, this );
  if( IsError( result ) )
    mpStream = 0;
  else if( mHostApi.Init )
  {
    result = mHostApi.Init( mpStream, mProperties.Elements(), &mHostApi.privateData );
    IsError( result );
  }
}

void
PortAudioStream::DeleteStreamObject()
{
  if( mHostApi.Stop )
    mHostApi.Stop( mpStream, mHostApi.privateData );
  if( mHostApi.Cleanup )
    mHostApi.Cleanup( mpStream, mHostApi.privateData );
  if( mpStream )
    Pa_CloseStream( mpStream );
  delete[] mpAllocatedBuffer;
  mpAllocatedBuffer = 0;
}

void
PortAudioStream::InitHostApi( PaHostApiTypeId inId )
{
  mHostApi.id = inId;
  mHostApi.privateData = this;
  mHostApi.Start = &PaStart;
  mHostApi.Stop = &PaStop;
  switch( inId )
  {
    case paWASAPI:
      mHostApi.callback = 0;
      mHostApi.ReadBegin = &PaReadSync;
      mHostApi.ReadEnd = 0;
      break;
    default:
      mHostApi.callback = &StreamCallback;
      mHostApi.ReadBegin = &PaReadBegin;
      mHostApi.ReadEnd = &PaReadEnd;
  }
}

int
PortAudioStream::StreamCallback( const void* inBuffer, void*, unsigned long, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* inInstance )
{
  PortAudioStream* this_ = reinterpret_cast<PortAudioStream*>( inInstance );
  this_->mpBuffer = inBuffer;
  this_->mBufferPrepared.Set();
  this_->mBufferDone.Wait( this_->mTimeout );
  this_->mBufferDone.Reset();
  return paContinue;
}

PaError
PortAudioStream::PaStart( PaStream* inStream, void* inInstance )
{
  PortAudioStream* this_ = reinterpret_cast<PortAudioStream*>( inInstance );
  this_->mBufferPrepared.Reset();
  this_->mBufferDone.Reset();
  return Pa_StartStream( inStream );
}

PaError
PortAudioStream::PaStop( PaStream* inStream, void* inInstance )
{
  PortAudioStream* this_ = reinterpret_cast<PortAudioStream*>( inInstance );
  PaError result = paNoError;
  if( Pa_IsStreamActive( inStream ) == 1 )
    result = Pa_AbortStream( inStream );
  if( result != paNoError )
    return result;
  while( Pa_IsStreamActive( inStream ) == 1 )
    Pa_Sleep( 10 );
  return result;
}

PaError
PortAudioStream::PaReadSync( PaStream*, void* inInstance, const int16_t** outBuffer )
{
  PortAudioStream* this_ = reinterpret_cast<PortAudioStream*>( inInstance );
  *outBuffer = this_->mpAllocatedBuffer;
  return Pa_ReadStream( this_->mpStream, this_->mpAllocatedBuffer, this_->mProperties.Elements() );
}

PaError
PortAudioStream::PaReadBegin( PaStream*, void* inInstance, const int16_t** outBuffer )
{
  PortAudioStream* this_ = reinterpret_cast<PortAudioStream*>( inInstance );
  if( !this_->mBufferPrepared.Wait( this_->mTimeout ) )
    return paTimedOut;
  this_->mBufferPrepared.Reset();
  *outBuffer = reinterpret_cast<const int16_t*>( this_->mpBuffer );
  return paNoError;
}

PaError
PortAudioStream::PaReadEnd( PaStream*, void* inInstance )
{
  PortAudioStream* this_ = reinterpret_cast<PortAudioStream*>( inInstance );
  this_->mBufferDone.Set();
  return paNoError;
}
