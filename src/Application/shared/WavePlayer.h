/////////////////////////////////////////////////////////////////////////////
//
// File: WavePlayer.h
//
// Date: Oct 29, 2001
//
// Author: Juergen Mellinger
//
// Description: A PCM audio output interface class.
//
// Changes: Jan 9, 2004, juergen.mellinger@uni-tuebingen.de:
//           Added copy constructor, assignment operator and related private
//           member functions.
//          Nov 11, 2004, juergen.mellinger@uni-tuebingen.de:
//           Added volume and balance setting.
//           Note that waveOutSetVolume does not work as described in the
//           Win32 documentation but will change the global wave out volume
//           in the windows mixer instead.
//          Dec 14, 2004, halder@informatik.uni-tuebingen.de
//           Reimplemented class using DirectX to enable simultaneous
//           playback of audio files, with different volume and pan settings,
//           on one device.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef WAVEPLAYERH
#define WAVEPLAYERH

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <string>

class TWavePlayer
{
  public:
    enum Error
    {
      noError,
      fileOpeningError,
      featureNotSupported,
      invalidParams,
      initError,
      genError
    };

  public:
                            TWavePlayer();
                            TWavePlayer( const TWavePlayer& );
                            TWavePlayer& operator=( const TWavePlayer& );
  virtual                   ~TWavePlayer();

  private:
                void        Construct();
                void        Destruct();
                void        Assign( const TWavePlayer& );

  public:
                Error       AttachFile( const char* inFileName );
                void        DetachFile();
                const std::string& GetFile() const { return mCurrentFileName; }

                Error       SetVolume( float ); // silent:0 ... 1:max
                float       GetVolume() const  { return mVolume; }
                Error       SetPan( float ); // left:-1 ... 1:right
                float       GetPan() const { return mPan; }

                void        Play();
                void        Stop();
                // Returns the current playing position from start
                // in milliseconds. Zero if not playing.
                float       GetPos() const;
                bool        IsPlaying() const;


  private:
                float       mVolume,
                            mPan;
                bool        mPlaying;
                std::string mCurrentFileName;

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
#endif // WAVEPLAYERH
