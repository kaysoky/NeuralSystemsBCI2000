////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A WavePlayer descendant that acts as a stimulus by playing
//   a sound on the Stimulus::Present event.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SOUND_STIMULUS_H
#define SOUND_STIMULUS_H

#include "Stimulus.h"
#include "WavePlayer.h"

class SoundStimulus : public Stimulus, public WavePlayer
{
 public:
  SoundStimulus()
    {}
  virtual ~SoundStimulus()
    {}

 protected:
  virtual void OnPresent()
    { WavePlayer::Play(); }
  virtual void OnConceal()
    {}
};

#endif // SOUND_STIMULUS_H
