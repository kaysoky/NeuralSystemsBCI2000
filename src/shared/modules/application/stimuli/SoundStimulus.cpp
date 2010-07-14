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
#include "SoundStimulus.h"

using namespace std;

vector<SoundStimulus::WavePlayerEntry> SoundStimulus::sWavePlayerInstances;

WavePlayer*
SoundStimulus::AllocateWavePlayer()
{
  for( vector<WavePlayerEntry>::iterator i = sWavePlayerInstances.begin(); i != sWavePlayerInstances.end(); ++i )
    if( !i->owned && !i->instance->IsPlaying() )
    {
      delete i->instance;
      i->instance = NULL;
    }

  vector<WavePlayerEntry>::iterator i = sWavePlayerInstances.begin();
  for( ; i != sWavePlayerInstances.end(); ++i )
    if( i->instance == NULL )
      break;
  if( i == sWavePlayerInstances.end() )
  {
    sWavePlayerInstances.push_back( WavePlayerEntry() );
    i = sWavePlayerInstances.end();
    --i;
  }
  i->instance = new WavePlayer;
  i->owned = true;
  return i->instance;
}

void
SoundStimulus::FreeWavePlayer( WavePlayer* inpWavePlayer )
{
  vector<WavePlayerEntry>::iterator i = sWavePlayerInstances.begin();
  for( ; i != sWavePlayerInstances.end(); ++i )
    if( i->instance == inpWavePlayer )
      break;
  if( i == sWavePlayerInstances.end() )
    throw "SoundStimulus::FreeWavePlayer: Trying to deallocate a WavePlayer object"
          " that has not been allocated with SoundStimulus::AllocateWavePlayer()";
  if( !i->instance->IsPlaying() )
  {
    delete i->instance;
    sWavePlayerInstances.erase( i );
  }
  else
  {
    i->owned = false;
  }
}


