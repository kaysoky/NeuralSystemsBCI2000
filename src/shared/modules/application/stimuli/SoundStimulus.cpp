////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A stimulus that plays a sound on the Stimulus::Present event.
//   To allow for finishing playback even after the SoundStimulus object has
//   been deleted, underlying WavePlayer instances are pooled, and only
//   deallocated when playback has finished.
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
#include "SoundStimulus.h"

#include "BCIException.h"

using namespace std;

vector<SoundStimulus::WavePlayerEntry> SoundStimulus::sWavePlayerInstances;

WavePlayer*
SoundStimulus::AllocateWavePlayer()
{
  for( vector<WavePlayerEntry>::iterator i = sWavePlayerInstances.begin(); i != sWavePlayerInstances.end(); ++i )
    if( !i->owned && i->instance && !i->instance->IsPlaying() )
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
    throw bciexception(
      "Trying to deallocate a WavePlayer object"
      " that has not been allocated with"
      " SoundStimulus::AllocateWavePlayer()"
      );
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


