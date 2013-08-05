////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A BufferedADC that gets input through the PortAudio interface.
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

#include "PortAudioADC.h"
#include "BCIStream.h"
#include "BCIException.h"
#include "WildcardMatch.h"

using namespace std;
using namespace bci;

RegisterFilter( PortAudioADC, 1 );

enum { auto_ = 0, callback = 1, blocking = 2 };

static size_t
HostApiPreference( int inId )
{
  const size_t lastKnownHostApi = paAudioScienceHPI;
  if( inId == 0 )
    return lastKnownHostApi + 1;

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

int PortAudioADC::sInstanceCount = 0;

PortAudioADC::PortAudioADC()
: mDevice( paNoDevice ),
  mpStream( 0 )
{
  if( sInstanceCount == 0 && Pa_Initialize() != paNoError )
    throw std_runtime_error( "Could not initialize PortAudio library" );
  ++sInstanceCount;

  for( PaHostApiIndex i = 0; i < Pa_GetHostApiCount(); ++i )
  {
    const PaHostApiInfo* p = Pa_GetHostApiInfo( i );
    if( p->defaultInputDevice != paNoDevice )
      mHostApis[p->type] = i;
  }
  ostringstream oss;
  oss << "Source int PortAudioHostAPI= 0 0 0 " << HostApiPreference( 0 ) - 1
      << " // 0: auto";
  for( size_t i = 1; i < HostApiPreference( 0 ); ++i )
  {
    const char* name = "%20%3Cnot%20available%3E%20";
    if( mHostApis.find( i ) != mHostApis.end() )
      name = Pa_GetHostApiInfo( mHostApis[i] )->name;
    oss << ", " << i << ": " << name;
  }
  oss << "(enumeration)";
  string PortAudioHostAPI = oss.str();

  BEGIN_PARAMETER_DEFINITIONS
    "Source float SamplingRate= auto",
    "Source int SampleBlockSize= auto",
    "Source int SourceCh= auto",
    "Source list SourceChOffset= 1 auto",
    "Source list SourceChGain= 1 auto",
    "Source list ChannelNames= 1 auto",
    PortAudioHostAPI.c_str(),
    "Source int PortAudioDeviceName= auto",
  END_PARAMETER_DEFINITIONS
}

PortAudioADC::~PortAudioADC()
{
  delete mpStream;
  if( --sInstanceCount == 0 )
    Pa_Terminate();
}

void
PortAudioADC::OnAutoConfig()
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
    Parameter( "SourceChGain" )( i ) << ::sqrt(0.5)/((1<<15)-1) << "V"; // 1V RMS = sqrt(2)V amplitude corresponds to a signal value of 2^15
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

  SignalProperties s( ActualParameter( "SourceCh" ), 0 );
  s.ElementUnit().SetOffset( 0 ).SetGain( 1 / ActualParameter( "SamplingRate" ).InHertz() ).SetSymbol( "s" );
  s = PortAudioStream( mDevice, s ).Properties();
  const double suggestedUpdateRate = 15; // Hz
  int suggestedBlockSize = s.Elements();
  if( suggestedBlockSize < 1 )
    suggestedBlockSize = Ceil( ActualParameter( "SamplingRate" ).InHertz() / suggestedUpdateRate );
  else while( s.ElementUnit().RawToPhysicalValue( suggestedBlockSize ) < 1 / suggestedUpdateRate )
    suggestedBlockSize += s.Elements();
  Parameter( "SampleBlockSize" ) = suggestedBlockSize;

  oss.str( "" );
  oss.clear();
  oss << "Using " << Pa_GetHostApiInfo( info.hostApi )->name
      << " device \"" << info.name
      << "\", " << ActualParameter( "SamplingRate" )
      << ", block size: " << ActualParameter( "SampleBlockSize" )
      << ", native block size: ";
  if( s.Elements() )
    oss << s.Elements() << ", suggested block size: " << suggestedBlockSize
        << " (" << s.ElementUnit().RawToPhysical( suggestedBlockSize ) << ")";
  else
    oss << "?";
  bciout << oss.str();
}

void
PortAudioADC::OnPreflight( SignalProperties& Output ) const
{
  const int rawMax = ( 1 << 15 ) / 3;
  for( int ch = 0; ch < Output.Channels(); ++ch )
    Output.ValueUnit( ch ).SetRawMin( -rawMax ).SetRawMax( rawMax );
  PortAudioStream s( mDevice, Output );
  if( !s.Error().empty() )
    bcierr << s.Error();
}

void
PortAudioADC::OnInitialize( const SignalProperties& Output )
{
  mpStream = new PortAudioStream( mDevice, Output );
  if( !mpStream->Error().empty() )
    bcierr << mpStream->Error();
}

void
PortAudioADC::OnHalt()
{
  delete mpStream;
  mpStream = 0;
}

void
PortAudioADC::OnStartAcquisition()
{
  mpStream->Start();
}

void
PortAudioADC::OnStopAcquisition()
{
  mpStream->Stop();
}

void
PortAudioADC::DoAcquire( GenericSignal& Output )
{
  if( !mpStream->Read( Output ) )
    Error( mpStream->Error() );
}
