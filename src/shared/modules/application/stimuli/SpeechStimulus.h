////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A stimulus that speaks text on the Stimulus::Present event.
//   To allow for finishing speech even after the SpeechStimulus object has
//   been deleted, underlying TextToSpeech instances are pooled, and only
//   deallocated after speech has been finished.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SPEECH_STIMULUS_H
#define SPEECH_STIMULUS_H

#include "Stimulus.h"
#include "TextToSpeech.h"

#include <vector>

class SpeechStimulus : public Stimulus
{
 public:
  SpeechStimulus()
  : mpTTS( AllocateTTS() )
    {}
  virtual ~SpeechStimulus()
    { FreeTTS( mpTTS ); }

  SpeechStimulus& SetText( const std::string& s )
    { mpTTS->SetText( s ); return *this; }
  const std::string& Text() const
    { return mpTTS->Text(); }
  SpeechStimulus& SetVolume( float f )
    { mpTTS->SetVolume( f ); return *this; }
  float Volume() const
    { return mpTTS->Volume(); }

 protected:
  virtual void OnPresent()
    { mpTTS->Speak(); }
  virtual void OnConceal()
    {}

 private:
  TextToSpeech* mpTTS;

  static TextToSpeech* AllocateTTS();
  static void FreeTTS( TextToSpeech* );

  struct TTSEntry
  {
    TextToSpeech* instance;
    bool          owned;
  };
  static std::vector<TTSEntry> sTTSInstances;
};

#endif // SPEECH_STIMULUS_H
