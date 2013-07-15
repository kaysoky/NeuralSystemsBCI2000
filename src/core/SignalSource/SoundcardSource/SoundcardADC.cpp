////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A BufferedADC that gets input from a sound card.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "SoundcardADC.h"
#include "BCIStream.h"
#include "BCIException.h"
#include "WildcardMatch.h"

using namespace std;
using namespace bci;

RegisterFilter( SoundcardADC, 1 );

static size_t
HostApiPreference( int inId )
{
  static const PaHostApiTypeId pref[] =
  {
    // Windows
    paWDMKS,
    paASIO,
    paWASAPI,
    paDirectSound,
    paMME,
    // Linux
    paALSA,
    paOSS,
  };
  size_t count = sizeof( pref ) / sizeof( *pref ),
         idx = 0;
  while( idx < count && pref[idx] != inId )
    ++idx;
  return idx;
}


SoundcardADC::SoundcardADC()
: mDevice( paNoDevice ),
  mpStream( 0 ),
  mpBuffer( 0 )
{
  if( Pa_Initialize() != paNoError )
    throw std_runtime_error( "Could not initialize PortAudio library" );

  BEGIN_PARAMETER_DEFINITIONS
    "Source float SamplingRate= auto",
    "Source int SampleBlockSize= auto",
    "Source int SourceCh= auto",
    "Source list SourceChOffset= 1 auto",
    "Source list SourceChGain= 1 auto",
    "Source list ChannelNames= 1 auto",
    "Source int PortAudioHostAPI= auto",
    "Source int PortAudioDeviceName= auto",
    "Source int PortAudioAcquisitionMode= 0 0 0 2 // "
      "0: auto, 1: callback, 2: blocking (enumeration)",
  END_PARAMETER_DEFINITIONS

  for( PaHostApiIndex i = 0; i < Pa_GetHostApiCount(); ++i )
  {
    const PaHostApiInfo* p = Pa_GetHostApiInfo( i );
    if( p->defaultInputDevice != paNoDevice )
      mHostApis[p->type] = i;
  }
  if( !mHostApis.empty() )
  {
    ostringstream oss;
    for( ApiList::const_iterator i = mHostApis.begin(); i != mHostApis.end(); ++i )
      oss << ", " << i->first << ": " << Pa_GetHostApiInfo( i->second )->name;
    oss << "(enumeration)";
    Parameter( "PortAudioHostAPI" )->SetComment( oss.str().substr( 2 ) );
  }
}

SoundcardADC::~SoundcardADC()
{
  delete[] mpBuffer;
  Pa_Terminate();
}

void
SoundcardADC::OnAutoConfig()
{
  int api = 0;
  for( ApiList::const_iterator i = mHostApis.begin(); i != mHostApis.end(); ++i )
    if( HostApiPreference( i->first ) <= HostApiPreference( api ) )
      api = i->first;
  Parameter( "PortAudioHostAPI" ) = api;

  api = ActualParameter( "PortAudioHostAPI" );
  if( mHostApis.find( api ) == mHostApis.end() )
  {
    bcierr << "Selected PortAudioHostAPI (" << api << ") is not available on this machine.";
    return;
  }
  PaHostApiIndex apiIdx = mHostApis[api];
  const PaHostApiInfo& apiInfo = *Pa_GetHostApiInfo( apiIdx );
  Parameter( "PortAudioDeviceName" ) = Pa_GetDeviceInfo( apiInfo.defaultInputDevice )->name;

  ostringstream oss;
  string deviceName = ActualParameter( "PortAudioDeviceName" );
  mDevice = paNoDevice;
  int prevApiIdx = -1;
  for( PaDeviceIndex device = 0; device < Pa_GetDeviceCount(); ++device )
  {
    const PaDeviceInfo& deviceInfo = *Pa_GetDeviceInfo( device );
    if( deviceInfo.hostApi == apiIdx )
    {
      bool match = WildcardMatch( deviceName, deviceInfo.name )
                 || WildcardMatch( deviceName + "\\>*", deviceInfo.name );
      if( match )
        mDevice = device;
    }
    if( prevApiIdx != deviceInfo.hostApi )
    {
      prevApiIdx = deviceInfo.hostApi;
      oss << " " << Pa_GetHostApiInfo( deviceInfo.hostApi )->name << ":\n";
    }
    oss << "  \"" << deviceInfo.name << "\": ";
    if( deviceInfo.maxInputChannels )
      oss << deviceInfo.maxInputChannels << "x in, ";
    if( deviceInfo.maxOutputChannels )
      oss << deviceInfo.maxOutputChannels << "x out, ";
    oss << deviceInfo.defaultSampleRate << "Hz\n";
  }
  if( mDevice == paNoDevice )
  {
    bcierr << "No device named \"" << deviceName << "\" could be found for the \""
           << apiInfo.name << "\" host API.\n"
           << "Available devices are:\n" << oss.str();
    return;
  }
  else
    bcidbg << "\nDevices found:\n" << oss.str();

  const PaDeviceInfo& info = *Pa_GetDeviceInfo( mDevice );
  Parameter( "SamplingRate" ) << info.defaultSampleRate << "Hz";
  Parameter( "SourceCh" ) = info.maxInputChannels;

  int actualChannels = ActualParameter( "SourceCh" );
  Parameter( "SourceChOffset" )->SetNumValues( actualChannels );
  Parameter( "SourceChGain" )->SetNumValues( actualChannels );
  for( int i = 0; i < Parameter( "SourceChGain" )->NumValues(); ++i )
  {
    Parameter( "SourceChOffset" )( i ) = 0;
    Parameter( "SourceChGain" )( i ) << 0.71 << "V"; // 1V RMS
  }

  if( actualChannels <= 2 )
    Parameter( "ChannelNames" )->SetNumValues( actualChannels );
  switch( actualChannels )
  {
    case 2:
      Parameter( "ChannelNames" )( 1 ) = "audioR";
      /* no break */
    case 1:
      Parameter( "ChannelNames" )( 0 ) = "audioL";
      break;
    default:
      for( int i = 0; i < Parameter( "ChannelNames" )->NumValues(); ++i )
        Parameter( "ChannelNames" )( i ) << "audio" << ( i + 1 );
  }

  unsigned long nativeBlockSize = NativeBlockSize();
  if( nativeBlockSize == 0 )
    Parameter( "SampleBlockSize" ) = ::ceil( ActualParameter( "SamplingRate" ).InHertz() * 1e-2 );
  else
    Parameter( "SampleBlockSize" ) = nativeBlockSize;

  bciout << "Using " << Pa_GetHostApiInfo( info.hostApi )->name
         << " device \"" << info.name
         << "\", " << ActualParameter( "SamplingRate" )
         << ", block size: " << ActualParameter( "SampleBlockSize" )
         << ", native block size: "
         << ( nativeBlockSize ? Parameter( "SampleBlockSize" ).c_str() : "?" );
}

void
SoundcardADC::OnPreflight( SignalProperties& Output ) const
{
  PaStreamParameters p = StreamParameters();
  PaError result = Pa_IsFormatSupported( &p, 0, Parameter( "SamplingRate" ) );
  if( result != paFormatIsSupported )
    bcierr << Pa_GetErrorText( result );
  Output = SignalProperties(
    Parameter( "SourceCh" ),
    Parameter( "SampleBlockSize" ),
    SignalType::float32
  );
}

void
SoundcardADC::OnInitialize( const SignalProperties& Output )
{
  delete[] mpBuffer;
  mpBuffer = 0;
  if( Parameter( "PortAudioAcquisitionMode" ) == blocking )
    mpBuffer = new float[Output.Channels() * Output.Elements()];

  if( mpStream )
    Pa_CloseStream( mpStream );
  PaStreamParameters p = StreamParameters();
  PaStreamCallback* callback = UseAcquisitionThread() ? 0 : &AcquisitionCallback;
  PaError result = Pa_OpenStream(
    &mpStream, &p, 0,
    Parameter( "SamplingRate" ).InHertz(),
    Output.Elements(),
    0, callback, this );
  if( result != paNoError )
  {
    mpStream = 0;
    bcierr << Pa_GetErrorText( result );
  }
}

void
SoundcardADC::OnStartAcquisition()
{
  ::memset( &mCallbackState, 0, sizeof( mCallbackState ) );
  Pa_StartStream( mpStream );
}

void
SoundcardADC::OnStopAcquisition()
{
  Pa_AbortStream( mpStream );
}

// Acquisition in blocking mode.
void
SoundcardADC::DoAcquire( GenericSignal& Output )
{
  PaError result = Pa_ReadStream( mpStream, mpBuffer, Output.Elements() );
  switch( result )
  {
    case paNoError:
      break;
    case paInputOverflowed:
      bciwarn << Pa_GetErrorText( result );
      break;
    default:
      bcierr << Pa_GetErrorText( result );
  }
  size_t idx = 0;
  for( int el = 0; el < Output.Elements(); el++ )
    for( int ch = 0; ch < Output.Channels(); ch++ )
      Output( ch, el ) = mpBuffer[idx++];
}

// Acquisition in callback mode.
int
SoundcardADC::AcquisitionCallback( const void* inpBuffer, void*, unsigned long inFrameCount,
                                   const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* inpThis )
{
  SoundcardADC* this_ = reinterpret_cast<SoundcardADC*>( inpThis );
  GenericSignal*& pOutput = this_->mCallbackState.pOutput;
  int& outputIdx = this_->mCallbackState.outputIdx;

  const float* pInput = reinterpret_cast<const float*>( inpBuffer );
  int inputIdx = 0,
      inputCount = 0;
  do 
  {
    if( !pOutput )
      pOutput = this_->GetBuffer();
    inputCount = pOutput->Channels() * inFrameCount;
    int outputCount = pOutput->Channels() * pOutput->Elements();

    while( inputIdx < inputCount && outputIdx < outputCount )
    {
      int ch = outputIdx / pOutput->Elements(),
          el = outputIdx % pOutput->Elements();
      ( *pOutput )( ch, el ) = pInput[inputIdx];
      ++inputIdx, ++outputIdx;
    }
    if( outputIdx == outputCount )
    {
      outputIdx = 0;
      this_->ReleaseBuffer( pOutput );
      pOutput = 0;
    }
  } while( inputIdx < inputCount );
  return paContinue;
}

PaStreamParameters
SoundcardADC::StreamParameters() const
{
  PaStreamParameters p = { 0 };
  p.device = mDevice;
  p.channelCount = ActualParameter( "SourceCh" );
  p.sampleFormat = paFloat32;
  const PaDeviceInfo* pInfo = Pa_GetDeviceInfo( mDevice );
  if( pInfo )
    p.suggestedLatency = pInfo->defaultLowInputLatency;
  return p;
}

namespace {
struct Measure
{
  unsigned long blockSize;
  int count;

  static int Callback(
    const void*, void*, unsigned long inFrames,
    const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags,
    void* inpThis )
  {
    Measure* this_ = reinterpret_cast<Measure*>( inpThis );
    if( --this_->count < 1 )
      this_->blockSize = 0;
    else
      this_->blockSize = max( this_->blockSize, inFrames );
    if( this_->blockSize == 0 || this_->blockSize == inFrames )
      return paAbort;
    return paContinue;
  }
};
}

unsigned long
SoundcardADC::NativeBlockSize() const
{
  Measure data = { 0 };
  data.count = 20;

  PaStream* pStream = 0;
  PaStreamParameters p = StreamParameters();
  PaError result = Pa_OpenStream(
    &pStream, &p, 0,
    ActualParameter( "SamplingRate" ).InHertz(),
    paFramesPerBufferUnspecified,
    0,
    &Measure::Callback, &data );
  if( result == paNoError )
  {
    Pa_StartStream( pStream );
    while( Pa_IsStreamActive( pStream ) == 1 )
      Pa_Sleep( 10 );
    Pa_CloseStream( pStream );
  }
  return data.blockSize;
}
