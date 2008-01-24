////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A TextToSpeech descendant that acts as a stimulus by speaking
//   text on the Stimulus::Present event.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SPEECH_STIMULUS_H
#define SPEECH_STIMULUS_H

#include "Stimulus.h"
#include "TextToSpeech.h"

class SpeechStimulus : public Stimulus, public TextToSpeech
{
 public:
  SpeechStimulus()
    {}
  virtual ~SpeechStimulus()
    {}

 protected:
  virtual void OnPresent()
    { TextToSpeech::Speak(); }
  virtual void OnConceal()
    {}
};

#endif // SPEECH_STIMULUS_H
