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
// Changes:
//
//////////////////////////////////////////////////////////////////////////////

#ifndef WAVEPLAYERH
#define WAVEPLAYERH

#include <mmsystem.h>

class TWavePlayer
{
  public:
    enum Error
    {
      noError,
      fileOpeningError,
      genError
    };
                        TWavePlayer();
    virtual             ~TWavePlayer();

                Error  AttachFile( const char* inFileName );
                void   DetachFile();
                void   Play();
                void   Stop();
                // Returns the current playing position from start
                // in milliseconds. Zero if not playing.
                // May be used for delay measurements.
                // Returns its value multiplied by -1.0 if the device driver
                // does not support sample-accurate positions.
                // In this case, the position returned may be ahead of the
                // actual sound output by some milliseconds.
                float  GetPos() const;
                bool   IsPlaying() const { return playing; }

  private:
                bool   playing;

// OS specific members go here.
#ifdef _WIN32
            enum TOperationMode
            {
                unknown,
                singleDeviceInstance,
                multipleDeviceInstances
            };
            HWAVEOUT        deviceHandle;
            HMMIO           fileHandle;
            WAVEFORMATEX    fileFormat;
            WAVEHDR         soundHeader;
            DWORD           samplingRate;
            DWORD           soundFlags;

    static  void CALLBACK   MsgHandler( HWAVEOUT, UINT, DWORD, DWORD, DWORD );

    // Not all wave out drivers support multiple instances of a wave out
    // device. We need to work differently then.
    static  HWAVEOUT        currentDeviceHandle;
    static  WAVEHDR         *currentHeader;
    static  TOperationMode  mode;
    static  bool            positionAccurate;
    static  int             numInstances;
#endif // WIN32

};

#endif // WAVEPLAYERH
