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
//
//////////////////////////////////////////////////////////////////////////////

#ifndef WAVEPLAYERH
#define WAVEPLAYERH

#include <mmsystem.h>
#include <string>

class TWavePlayer
{
  public:
    enum Error
    {
      noError,
      fileOpeningError,
      featureNotSupported,
      genError
    };

  public:
                            TWavePlayer();
                            TWavePlayer( const TWavePlayer& );
                            TWavePlayer& operator=( const TWavePlayer& );
    virtual                 ~TWavePlayer();

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
                Error       SetBalance( float ); // left:-1 ... 1:right
                float       GetBalance() const { return mBalance; }
                Error       SetVolumeAndBalance( float, float );

                void        Play();
                void        Stop();
                // Returns the current playing position from start
                // in milliseconds. Zero if not playing.
                // May be used for delay measurements.
                // Returns its value multiplied by -1.0 if the device driver
                // does not support sample-accurate positions.
                // In this case, the position returned may be ahead of the
                // actual sound output by some milliseconds.
                float              GetPos() const;
                bool               IsPlaying() const { return mPlaying; }


  private:
                float       mVolume,
                            mBalance;
                bool        mPlaying;
                std::string mCurrentFileName;

// OS specific members go here.
#ifdef _WIN32
            enum TOperationMode
            {
                unknown,
                singleDeviceInstance,
                multipleDeviceInstances
            };
            HWAVEOUT        mDeviceHandle;
            HMMIO           mFileHandle;
            WAVEFORMATEX    mFileFormat;
            WAVEHDR         mSoundHeader;
            DWORD           mSamplingRate;
            DWORD           mSoundFlags;

    static  void CALLBACK   MsgHandler( HWAVEOUT, UINT, DWORD, DWORD, DWORD );

    // Not all wave out drivers support multiple instances of a wave out
    // device. We need to work differently then.
    static  HWAVEOUT        sCurrentDeviceHandle;
    static  WAVEHDR         *spCurrentHeader;
    static  TOperationMode  sMode;
    static  bool            sPositionAccurate;
    static  int             sNumInstances;
#endif // WIN32

};

#endif // WAVEPLAYERH
