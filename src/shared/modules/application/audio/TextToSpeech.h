////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A simple wrapper class for text-to-speech audio output.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TEXT_TO_SPEECH_H
#define TEXT_TO_SPEECH_H

class TextToSpeech
{
 public:
  TextToSpeech();
  virtual ~TextToSpeech();
  // Properties
  TextToSpeech& SetText( const std::string& s )
    { mText = s; return *this; }
  const std::string& Text() const
    { return mText; }
  TextToSpeech& SetVolume( float f )
    { mVolume = f; return *this; }
  float Volume() const
    { return mVolume; }
  // Actions
  TextToSpeech& Speak();
  TextToSpeech& Stop();

 private:
  static int  sNumInstances;
  std::string mText;
  float       mVolume;
#ifdef _WIN32
  class ISpVoice* mpVoice;
#endif // _WIN32
};

#endif // TEXT_TO_SPEECH_H