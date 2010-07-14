////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A stimulus that plays a sound on the Stimulus::Present event.
//   To allow for finishing playback even after the SoundStimulus object has
//   been deleted, underlying WavePlayer instances are pooled, and only
//   deallocated when playback has finished.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SOUND_STIMULUS_H
#define SOUND_STIMULUS_H

#include "Stimulus.h"
#include "WavePlayer.h"

#include <vector>
#include <string>

class SoundStimulus : public Stimulus
{
 public:
  SoundStimulus()
  : mpWavePlayer( AllocateWavePlayer() )
    {}
  virtual ~SoundStimulus()
    { FreeWavePlayer( mpWavePlayer ); }

  SoundStimulus& SetFile( const std::string& inFileName )
    { mpWavePlayer->SetFile( inFileName ); return *this; }
  const std::string& File() const
    { return mpWavePlayer->File(); }

  SoundStimulus& SetVolume( float f )
    { mpWavePlayer->SetVolume( f ); return *this; }
  float Volume() const
    { return mpWavePlayer->Volume(); }
  SoundStimulus& SetPan( float f )
    { mpWavePlayer->SetPan( f ); return *this; }
  float Pan() const
    { return mpWavePlayer->Pan(); }

  WavePlayer::Error ErrorState() const
    { return mpWavePlayer->ErrorState(); }

 protected:
  virtual void OnPresent()
    { mpWavePlayer->Play(); }
  virtual void OnConceal()
    {}

 private:
  WavePlayer* mpWavePlayer;

  static WavePlayer* AllocateWavePlayer();
  static void FreeWavePlayer( WavePlayer* );

  struct WavePlayerEntry
  {
    WavePlayer* instance;
    bool        owned;
  };
  static std::vector<WavePlayerEntry> sWavePlayerInstances;
};

#endif // SOUND_STIMULUS_H
