////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A stimulus that plays a sound, or speaks a text, when it is
//   presented.
//   When a sound string is enclosed in single quotes ('text'), it is rendered
//   using the OS's text-to-speech engine.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "AudioStimulus.h"
#include "SpeechStimulus.h"
#include "SoundStimulus.h"

using namespace std;

AudioStimulus::AudioStimulus()
: mpStimulus( NULL ),
  mVolume( 1.0 )
{
}

AudioStimulus::~AudioStimulus()
{
  delete mpStimulus;
}


AudioStimulus&
AudioStimulus::SetSound( const string& inSound )
{
  delete mpStimulus;
  mSound = "";

  if( !inSound.empty() )
  {
    mSound = inSound;
    if( inSound[ 0 ] == cSpeechQuote )
    {
      string text = inSound.substr( 1, inSound.rfind( cSpeechQuote ) );
      SpeechStimulus* pSpeech = new SpeechStimulus;
      pSpeech->SetText( text );
      mpStimulus = pSpeech;
    }
    else
    {
      SoundStimulus* pSound = new SoundStimulus;
      pSound->SetFile( inSound );
      mpStimulus = pSound;
    }
  }
  return *this;
}

const string&
AudioStimulus::Sound() const
{
  return mSound;
}

AudioStimulus&
AudioStimulus::SetVolume( float inVolume )
{
  mVolume = inVolume;
  return *this;
}

float
AudioStimulus::Volume() const
{
  return mVolume;
}

void
AudioStimulus::OnPresent()
{
  WavePlayer* pWavePlayer = dynamic_cast<WavePlayer*>( mpStimulus );
  if( pWavePlayer != NULL )
    pWavePlayer->SetVolume( mVolume );

  TextToSpeech* pTextToSpeech = dynamic_cast<TextToSpeech*>( mpStimulus );
  if( pTextToSpeech != NULL )
    pTextToSpeech->SetVolume( mVolume );

  if( mpStimulus != NULL )
    mpStimulus->Present();
}
