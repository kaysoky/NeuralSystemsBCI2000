/////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de,
//          halder@informatik.uni-tuebingen.de
// Description: A PCM audio output interface class.
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
//////////////////////////////////////////////////////////////////////////////
#ifndef WAVE_PLAYER_H
#define WAVE_PLAYER_H

#if !defined( USE_DSOUND ) && defined( _WIN32 )
# define USE_DSOUND 1
#endif // USE_DSOUND, _WIN32

#if USE_DSOUND
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#else //USE_DSOUND
#include <QSound>
#endif // USE_DSOUND
#include <string>

class WavePlayer
{
 public:
  WavePlayer();
  WavePlayer( const WavePlayer& );
  WavePlayer& operator=( const WavePlayer& );
  virtual ~WavePlayer();

 private:
  void Construct();
  void Destruct();
  void Assign( const WavePlayer& );
  void Clear();

 public:
  WavePlayer& SetFile( const std::string& inFileName );
  const std::string& File() const
    { return mFile; }

  WavePlayer& SetVolume( float ); // silent:0 ... 1:max
  float Volume() const
    { return mVolume; }
  WavePlayer& SetPan( float );    // left:-1 ... 1:right
  float Pan() const
    { return mPan; }

  WavePlayer& Play();
  WavePlayer& Stop();
  // Returns the current playing position from start
  // in milliseconds. Zero if not playing.
  float PlayingPos() const;
  bool  IsPlaying() const;

  enum Error
  {
    noError,
    fileOpeningError,
    featureNotSupported,
    invalidParams,
    initError,
    genError
  };
  Error ErrorState() const
    { return mErrorState; }

 private:
  float       mVolume,
              mPan;
  std::string mFile;
  Error       mErrorState;

  static  int sNumInstances;

#if USE_DSOUND
          DWORD               mSamplingRate;
          DWORD               mBitsPerSample;
          LPDIRECTSOUNDBUFFER mSecondaryBuffer;
  static  LPDIRECTSOUND       sPDS;
  static  LPDIRECTSOUNDBUFFER sPrimarySoundBuffer;
#else // USE_DSOUND
  QSound* mpSound;
  bool    mVolumeWarningIssued,
          mPanWarningIssued;
#endif // USE_DSOUND

};
#endif // WAVE_PLAYER_H

