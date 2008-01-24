/////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de,
//          halder@informatik.uni-tuebingen.de
// Description: A PCM audio output interface class.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////
#ifndef WAVE_PLAYER_H
#define WAVE_PLAYER_H

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
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

  // OS specific members go here.
  static  int                 sNumInstances;
#ifdef _WIN32
          DWORD               mSamplingRate;
          DWORD               mBitsPerSample;
          LPDIRECTSOUNDBUFFER mSecondaryBuffer;
  static  LPDIRECTSOUND       sPDS;
  static  LPDIRECTSOUNDBUFFER sPrimarySoundBuffer;
#endif // WIN32

};
#endif // WAVE_PLAYER_H

