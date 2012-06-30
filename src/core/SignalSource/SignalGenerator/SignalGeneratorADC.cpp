////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: An ADC class for testing purposes.
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

#include "SignalGeneratorADC.h"
#include "BCIError.h"
#include "GenericSignal.h"
#include "OSThread.h"

#include <cmath>
#if _WIN32
# include <Windows.h>
#elif USE_QT
# include <QCursor>
# include <QApplication>
# include <QDesktopWidget>
#endif

using namespace std;

// Register the source class with the framework.
RegisterFilter( SignalGeneratorADC, 1 );

SignalGeneratorADC::SignalGeneratorADC()
: mRandomGenerator( this ),
  mSineFrequency( 0 ),
  mSineAmplitude( 0 ),
  mNoiseAmplitude( 0 ),
  mDCOffset( 0 ),
  mSineChannelX( 0 ),
  mSineChannelY( 0 ),
  mSineChannelZ( 0 ),
  mModulateAmplitude( 1 ),
  mAmplitudeX( 1 ),
  mAmplitudeY( 1 ),
  mAmplitudeZ( 1 ),
  mSinePhase( 0 ),
  mLasttime( 0 )
{
}

SignalGeneratorADC::~SignalGeneratorADC()
{
}

void
SignalGeneratorADC::Publish()
{
  BEGIN_PARAMETER_DEFINITIONS
    "Source:Signal%20Properties int SourceCh= 16 "
       "16 1 % // number of digitized and stored channels",
    "Source:Signal%20Properties int SampleBlockSize= 32 "
       "32 1 % // number of samples transmitted at a time",
    "Source:Signal%20Properties int SamplingRate= 256Hz "
       "256Hz 1 % // sample rate",
#if _WIN32 || USE_QT
    "Source int ModulateAmplitude= 0 0 0 1 "
      "// Modulate the amplitude with the mouse (0=no, 1=yes) (boolean)",
#endif // _WIN32 || USE_QT
    "Source int SineChannelX= 0 0 0 % "
      "// Channel number of sinewave controlled by mouse x position",
    "Source int SineChannelY= 0 0 0 % "
      "// Channel number of sinewave controlled by mouse y position (0 for all)",
    "Source int SineChannelZ= 0 0 0 % "
      "// Channel number of sinewave controlled by mouse key state",
    "Source float SineFrequency= 10Hz 10Hz % % "
      "// Frequency of sine wave",
    "Source int SineAmplitude= 100muV 100muV % % "
      "// Amplitude of sine wave",
    "Source int NoiseAmplitude= 30muV 30muV % % "
      "// Amplitude of white noise (common to all channels)",
    "Source int DCOffset= 0muV 0muV % % "
      "// DC offset (common to all channels)",
    "Source string OffsetMultiplier= % StimulusType % % "
      "// Expression to multiply offset by",
    "Source int SignalType= 0 0 0 2 "
      "// numeric type of output signal: "
        " 0: int16,"
        " 1: float32,"
        " 2: int32 "
        "(enumeration)",
  END_PARAMETER_DEFINITIONS
}


void
SignalGeneratorADC::Preflight( const SignalProperties&,
                                     SignalProperties& Output ) const
{
  Parameter( "SourceChGain" );
  Parameter( "SourceChOffset" );
  Parameter( "SineFrequency" ).InHertz();
  Parameter( "SineAmplitude" ).InMicrovolts();
  Parameter( "NoiseAmplitude" ).InMicrovolts();
  if( Parameter( "DCOffset" ).InMicrovolts() != 0 )
    Expression( Parameter( "OffsetMultiplier" ) ).Evaluate();
  Parameter( "RandomSeed" );

  // Resource availability checks.
  /* The random source does not depend on external resources. */

  // Input signal checks.
  /* The input signal will be ignored. */

  // Requested output signal properties.
  SignalType signalType;
  switch( int( Parameter( "SignalType" ) ) )
  {
    case 0:
      signalType = SignalType::int16;
      break;
    case 1:
      signalType = SignalType::float32;
      break;
    case 2:
      signalType = SignalType::int32;
      break;
    default:
      bcierr << "Unknown SignalType value" << endl;
  }
  Output = SignalProperties(
    Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), signalType );
}


void
SignalGeneratorADC::Initialize( const SignalProperties&, const SignalProperties& )
{
  mSourceChGain.resize( Parameter( "SourceChGain" )->NumValues() );
  for( size_t i = 0; i < mSourceChGain.size(); ++i )
    mSourceChGain[i] = Parameter( "SourceChGain" )( i );
  mSourceChOffset.resize( Parameter( "SourceChOffset" )->NumValues() );
  for( size_t i = 0; i < mSourceChOffset.size(); ++i )
    mSourceChOffset[i] = Parameter( "SourceChOffset" )( i );
  mSineFrequency = Parameter( "SineFrequency" ).InHertz() / Parameter( "SamplingRate" ).InHertz();
  mSineAmplitude = Parameter( "SineAmplitude" ).InMicrovolts();
  mSinePhase = M_PI / 2;
  mNoiseAmplitude = Parameter( "NoiseAmplitude" ).InMicrovolts();
  mDCOffset = Parameter( "DCOffset" ).InMicrovolts();
  if( mDCOffset == 0 )
    mOffsetMultiplier = Expression( "" );
  else
    mOffsetMultiplier = Expression( Parameter( "OffsetMultiplier" ) );
  mSineChannelX = Parameter( "SineChannelX" );
  mSineChannelY = Parameter( "SineChannelY" );
  mSineChannelZ = Parameter( "SineChannelZ" );
  mAmplitudeX = 1.0;
  mAmplitudeY = 1.0;
  mAmplitudeZ = 1.0;

#if _WIN32 || USE_QT
  mModulateAmplitude = ( Parameter( "ModulateAmplitude" ) != 0 );
#endif // _WIN32 || USE_QT

  mLasttime = PrecisionTime::Now();
}


void
SignalGeneratorADC::StartRun()
{
  if( Parameter( "RandomSeed" ) != 0 )
    mSinePhase = 0;
}


void
SignalGeneratorADC::Process( const GenericSignal&, GenericSignal& Output )
{
#if _WIN32
  if( mModulateAmplitude )
  {
    POINT p = { 0, 0 };
    if( ::GetCursorPos( &p ) )
    {
      int width = ::GetSystemMetrics( SM_CXVIRTUALSCREEN ),
          height = ::GetSystemMetrics( SM_CYVIRTUALSCREEN );
      mAmplitudeX = float( p.x ) / width;
      mAmplitudeY = 1.0 - float( p.y ) / height;
    }
    enum { isPressed = 0x8000 };
    bool leftButton = ::GetAsyncKeyState( VK_LBUTTON ) & isPressed,
         rightButton = ::GetAsyncKeyState( VK_RBUTTON ) & isPressed;
    mAmplitudeZ = 0.5 + ( leftButton ? -0.5 : 0 ) + ( rightButton ? 0.5 : 0 );
  }
#elif USE_QT
  if( mModulateAmplitude )
  {
    QPoint p = QCursor::pos();
    QRect r = QApplication::desktop()->geometry();
    mAmplitudeX = float( p.x() ) / r.width();
    mAmplitudeY = 1.0 - float( p.y() ) / r.height();
  }
#endif // !_WIN32, !USE_QT

  double maxVal = Output.Type().Max(),
         minVal = Output.Type().Min();

  for( int sample = 0; sample < Output.Elements(); ++sample )
  {
    mSinePhase += 2 * M_PI * mSineFrequency;
    mSinePhase = ::fmod( mSinePhase, 2 * M_PI );
    double sineValue = ::sin( mSinePhase ) * mSineAmplitude;

    double offset = mDCOffset;
    if( offset != 0 )
      offset *= mOffsetMultiplier.Evaluate();
    for( int ch = 0; ch < Output.Channels(); ++ch )
    {
      double value = offset;
      value += ( mRandomGenerator.Random() * mNoiseAmplitude / mRandomGenerator.RandMax() - mNoiseAmplitude / 2 );
      if( mSineChannelX == ch + 1 )
        value += sineValue * mAmplitudeX;
      if( mSineChannelY == 0 || mSineChannelY == ch + 1 )
        value += sineValue * mAmplitudeY;
      if( mSineChannelZ == ch + 1 )
        value += sineValue * mAmplitudeZ;

      value /= mSourceChGain[ch];
      value += mSourceChOffset[ch];

      value = max( value, minVal );
      value = min( value, maxVal );

      Output( ch, sample ) = value;
    }
  }
  // Wait for the amount of time that corresponds to the length of a data block.
  int blockDuration = static_cast<int>( 1e3 * MeasurementUnits::SampleBlockDuration() );
  OSThread::PrecisionSleepUntil( mLasttime + blockDuration );
  mLasttime = PrecisionTime::Now();
}


void
SignalGeneratorADC::Halt()
{
}



