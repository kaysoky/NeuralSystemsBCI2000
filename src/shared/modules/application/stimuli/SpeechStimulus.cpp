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
#include "SpeechStimulus.h"

using namespace std;

vector<SpeechStimulus::TTSEntry> SpeechStimulus::sTTSInstances;

TextToSpeech*
SpeechStimulus::AllocateTTS()
{
  for( vector<TTSEntry>::iterator i = sTTSInstances.begin(); i != sTTSInstances.end(); ++i )
    if( !i->owned && !i->instance->IsSpeaking() )
    {
      delete i->instance;
      i->instance = NULL;
    }

  vector<TTSEntry>::iterator i = sTTSInstances.begin();
  for( ; i != sTTSInstances.end(); ++i )
    if( i->instance == NULL )
      break;
  if( i == sTTSInstances.end() )
  {
    sTTSInstances.push_back( TTSEntry() );
    i = sTTSInstances.end();
    --i;
  }
  i->instance = new TextToSpeech;
  i->owned = true;
  return i->instance;
}

void
SpeechStimulus::FreeTTS( TextToSpeech* inpTTS )
{
  vector<TTSEntry>::iterator i = sTTSInstances.begin();
  for( ; i != sTTSInstances.end(); ++i )
    if( i->instance == inpTTS )
      break;
  if( i == sTTSInstances.end() )
    throw "SpeechStimulus::FreeTTS: Trying to deallocate a TextToSpeech object"
          " that has not been allocated with SpeechStimulus::AllocateTTS()";
  if( !i->instance->IsSpeaking() )
  {
    delete i->instance;
    sTTSInstances.erase( i );
  }
  else
  {
    i->owned = false;
  }
}


