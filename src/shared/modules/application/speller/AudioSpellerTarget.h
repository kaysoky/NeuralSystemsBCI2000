////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A speller target that optionally plays a sound or speaks a text
//   when selected.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef AUDIO_SPELLER_TARGET_H
#define AUDIO_SPELLER_TARGET_H

#include "Speller.h"
#include "AudioStimulus.h"

class AudioSpellerTarget : public SpellerTarget
{
 public:
  explicit AudioSpellerTarget( Speller& s )
    : SpellerTarget( s )
    {}
  virtual ~AudioSpellerTarget()
    {}
  // Properties
  AudioSpellerTarget& SetSound( const std::string& s )
    { mAudioStimulus.SetSound( s ); return *this; }
  const std::string& Sound() const
    { return mAudioStimulus.Sound(); }

 protected:
  virtual void OnSelect()
    { mAudioStimulus.Present(); SpellerTarget::OnSelect(); }

 private:
  AudioStimulus mAudioStimulus;
};

#endif // AUDIO_SPELLER_TARGET_H

