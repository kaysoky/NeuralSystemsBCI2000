/////////////////////////////////////////////////////////////////////////////
//
// File: WavePlayer.cpp
//
// Date: Oct 29, 2001
//
// Author: Juergen Mellinger
//
// Description: The WIN32 implementation of the TWavePlayer interface.
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
#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include <math.h>
#include <assert>

#include "WavePlayer.h"

HWAVEOUT TWavePlayer::sCurrentDeviceHandle = NULL;
WAVEHDR* TWavePlayer::spCurrentHeader = NULL;

TWavePlayer::TOperationMode TWavePlayer::sMode = unknown;
bool                        TWavePlayer::sPositionAccurate = false;
int                         TWavePlayer::sNumInstances = 0;


TWavePlayer::TWavePlayer()
: mVolume( 1.0 ),
  mBalance( 0.0 ),
  mSamplingRate( 0 ),
  mPlaying( false ),
  mDeviceHandle( NULL ),
  mFileHandle( NULL )
{
    Construct();
}

TWavePlayer::TWavePlayer( const TWavePlayer& inOriginal )
: mVolume( 1.0 ),
  mBalance( 0.0 ),
  mSamplingRate( 0 ),
  mPlaying( false ),
  mDeviceHandle( NULL ),
  mFileHandle( NULL )
{
    Construct();
    Assign( inOriginal );
}

TWavePlayer&
TWavePlayer::operator=( const TWavePlayer& inOriginal )
{
    if( &inOriginal != this )
    {
      Destruct();
      Construct();
      Assign( inOriginal );
    }
    return *this;
}

TWavePlayer::~TWavePlayer()
{
    Destruct();
}

void
TWavePlayer::Construct()
{
    ++sNumInstances;
    if( sMode == unknown )
    {
        // Determine if the wave out driver supports multiple device instances.
        const int   rate = 22050,   // should be supported by any driver
                    channels = 1,
                    bitsPerSamplePerChannel = 8;

        HWAVEOUT    deviceHandle1,
                    deviceHandle2;
        MMRESULT    sysErr;
        WAVEFORMATEX format =
        {
            WAVE_FORMAT_PCM, channels, rate,
            rate * channels * bitsPerSamplePerChannel / 8,
            channels * bitsPerSamplePerChannel / 8,
            bitsPerSamplePerChannel
        };

        sysErr = ::waveOutOpen( &deviceHandle1, WAVE_MAPPER, &format,
                                            NULL, 0, CALLBACK_NULL );
        if( sysErr != MMSYSERR_NOERROR )
            sMode = singleDeviceInstance;
        else
        {
            sysErr = ::waveOutOpen( &deviceHandle2, WAVE_MAPPER, &format,
                                                NULL, 0, CALLBACK_NULL );
            if( sysErr != MMSYSERR_NOERROR )
            {
                sMode = singleDeviceInstance;
                ::waveOutClose( deviceHandle1 );
            }
            else
            {
                sMode = multipleDeviceInstances;
                ::waveOutClose( deviceHandle1 );
                ::waveOutClose( deviceHandle2 );
            }
        }

        // Determine if the device supports retrieving a sample-accurate position.
        WAVEOUTCAPS caps;
        sysErr = ::waveOutGetDevCaps( WAVE_MAPPER, &caps, sizeof( caps ) );
        if( sysErr == MMSYSERR_NOERROR )
            sPositionAccurate = ( caps.dwSupport & WAVECAPS_SAMPLEACCURATE );
    }
}

void
TWavePlayer::Destruct()
{
    --sNumInstances;

    // This happens in singleDeviceInstance mode.
    if( sNumInstances < 1 && sCurrentDeviceHandle != NULL )
    {
        // If the device is open, close it.
        ::waveOutReset( sCurrentDeviceHandle );
        if( spCurrentHeader != NULL )
        {
            ::waveOutUnprepareHeader( sCurrentDeviceHandle,
                                    spCurrentHeader, sizeof( mSoundHeader ) );
            spCurrentHeader = NULL;
        }
        ::waveOutClose( sCurrentDeviceHandle );
        sCurrentDeviceHandle = NULL;
    }

    DetachFile();
}

void
TWavePlayer::Assign( const TWavePlayer& inOriginal )
{
  AttachFile( inOriginal.GetFile().c_str() );
  SetVolumeAndBalance( inOriginal.GetVolume(), inOriginal.GetBalance() );
}

TWavePlayer::Error
TWavePlayer::AttachFile( const char* inFileName )
{
    DetachFile();

    if( inFileName == NULL || *inFileName == '\0' )
      return noError;

    Error err = noError;

    // Open the file with buffered I/O. If the file is on HD,
    // the OS should map its contents into memory instead of
    // maintaining a separate buffer. This is what we want.

    mFileHandle = mmioOpen( ( char* )inFileName, NULL, MMIO_READ | MMIO_ALLOCBUF );
    if( mFileHandle == NULL )
        err = fileOpeningError;

    MMCKINFO    parentChunkInfo,
                childChunkInfo;
    MMIOINFO    info;
    MMRESULT    sysErr;

    if( err == noError )
    {
        // Read the chunks we need to play the file.
        parentChunkInfo.fccType = mmioFOURCC( 'W', 'A', 'V', 'E' );
        sysErr = mmioDescend( mFileHandle, &parentChunkInfo, NULL, MMIO_FINDRIFF );
        if( sysErr != MMSYSERR_NOERROR )
            // The file doesn't contain a WAVE chunk.
            err = fileOpeningError;
    }

    if( err == noError )
    {
        childChunkInfo.ckid = mmioFOURCC( 'f', 'm', 't', ' ' );
        sysErr = mmioDescend( mFileHandle, &childChunkInfo, &parentChunkInfo, MMIO_FINDCHUNK );
        if( sysErr != MMSYSERR_NOERROR )
            // The file doesn't contain a fmt chunk.
            err = fileOpeningError;
    }

    // Read the fmt chunk into the WAVEFORMATEX structure.
    if( err == noError )
    {
        mFileFormat.cbSize = 0;
        if( childChunkInfo.cksize > sizeof( mFileFormat ) )
            // The chunk does not fit into the WAVEFORMATEX structure.
            err = fileOpeningError;
    }

    if( err == noError )
    {
        if( mmioRead( mFileHandle, ( char* )&mFileFormat, childChunkInfo.cksize ) != ( long )childChunkInfo.cksize )
            // There is not enough data in the file.
            err = fileOpeningError;
    }

    if( err == noError )
    {
        mmioAscend( mFileHandle, &childChunkInfo, 0 );
        // Get to the data chunk.
        childChunkInfo.ckid = mmioFOURCC( 'd', 'a', 't', 'a' );
        sysErr = mmioDescend( mFileHandle, &childChunkInfo, &parentChunkInfo, MMIO_FINDCHUNK );
        if( sysErr != MMSYSERR_NOERROR )
            // The file doesn't contain a data chunk.
            err = fileOpeningError;
    }

    long    dataLength = childChunkInfo.cksize;

    if( err == noError )
    {
        // Resize the file buffer to contain all data.
        sysErr = mmioSetBuffer( mFileHandle, NULL, dataLength, 0 );
        if( sysErr != MMSYSERR_NOERROR )
            // The new buffer size could not be set. Should not happen
            // in a WIN32 environment if virtual memory is configured
            // reasonably.
            err = fileOpeningError;
    }

    if( err == noError )
    {
        // Make sure the data is in the buffer.
        sysErr = mmioAdvance( mFileHandle, NULL, MMIO_READ );
        if( sysErr != MMSYSERR_NOERROR )
            err = fileOpeningError;
    }

    if( err == noError )
    {
        // Get a pointer to the buffer and enter it into the WAVEHDR structure.
        sysErr = mmioGetInfo( mFileHandle, &info, 0 );
        if( sysErr != MMSYSERR_NOERROR )
            err = fileOpeningError;
    }

    if( err == noError )
    {
        assert( info.cchBuffer == dataLength );
        mSoundHeader.lpData = info.pchBuffer;
        mSoundHeader.dwBufferLength = dataLength;
        mSoundHeader.dwBytesRecorded = 0;
        mSoundHeader.dwUser = 0;
        mSoundHeader.dwFlags = 0;
        mSoundHeader.dwLoops = 0;
        mSoundHeader.lpNext = NULL;
        mSoundHeader.reserved = 0;

        mSamplingRate = mFileFormat.nSamplesPerSec;

        switch( sMode )
        {
            case singleDeviceInstance:
                // We cannot open the device and keep it open because the device cannot
                // be opened twice.
                break;

            case multipleDeviceInstances:
                // Try opening a wave output device suited for the file's format.
                sysErr = ::waveOutOpen( &mDeviceHandle, WAVE_MAPPER, &mFileFormat,
                                        ( DWORD )MsgHandler, ( DWORD )this, CALLBACK_FUNCTION );
                if( sysErr != MMSYSERR_NOERROR )
                    err = genError;

                if( err == noError )
                {
                    // Prepare the sound header for playing.
                    sysErr = ::waveOutPrepareHeader( mDeviceHandle, &mSoundHeader, sizeof( mSoundHeader ) );
                    if( sysErr != MMSYSERR_NOERROR )
                        err = genError;
                    else
                        mSoundFlags = mSoundHeader.dwFlags;
                }
                break;
            default:
                // This should not happen.
                assert( false );
        }
    }

    if( err != noError )
      DetachFile();
    else
      mCurrentFileName = inFileName;

    return err;
}

void
TWavePlayer::DetachFile()
{
    switch( sMode )
    {
        case singleDeviceInstance:
            // If this instance of TWavePlayer is playing, stop playback and
            // unprepare our header.
            if( ( sCurrentDeviceHandle != NULL ) && ( spCurrentHeader == &mSoundHeader ) )
            {
                // Stop playback immediately.
                ::waveOutReset( sCurrentDeviceHandle );
                // Free internal buffer memory.
                ::waveOutUnprepareHeader( sCurrentDeviceHandle, spCurrentHeader, sizeof( mSoundHeader ) );
                // Close the device.
                ::waveOutClose( sCurrentDeviceHandle );
                sCurrentDeviceHandle = NULL;
                spCurrentHeader = NULL;
            }
            break;

        case multipleDeviceInstances:
            if( mDeviceHandle != NULL )
            {
                // Stop playback immediately.
                ::waveOutReset( mDeviceHandle );
                // Free internal buffer memory.
                ::waveOutUnprepareHeader( mDeviceHandle, &mSoundHeader, sizeof( mSoundHeader ) );
                // Close device instance.
                ::waveOutClose( mDeviceHandle );
                mDeviceHandle = NULL;
            }
            break;

        default:
            assert( false );
    }

    if( mFileHandle != NULL )
        mmioClose( mFileHandle, 0 );
    mFileHandle = NULL;
    mCurrentFileName = "";
    mSamplingRate = 0;
}

void
TWavePlayer::Play()
{
    if( mFileHandle == NULL )
        return;

    SetVolume( mVolume );
    SetBalance( mBalance );

    MMRESULT    sysErr;

    switch( sMode )
    {
        case singleDeviceInstance:
            if( ( sCurrentDeviceHandle != NULL ) && ( spCurrentHeader != NULL ) )
            {
                // If the device is open, close it.
                ::waveOutReset( sCurrentDeviceHandle );
                ::waveOutUnprepareHeader( sCurrentDeviceHandle,
                                        spCurrentHeader, sizeof( mSoundHeader ) );
                ::waveOutClose( sCurrentDeviceHandle );
                spCurrentHeader = NULL;
            }

            sysErr = ::waveOutOpen( &sCurrentDeviceHandle, WAVE_MAPPER, &mFileFormat,
                                ( DWORD )MsgHandler, ( DWORD )this, CALLBACK_FUNCTION );
            if( sysErr != MMSYSERR_NOERROR )
            {
                sCurrentDeviceHandle = NULL;
                return;
            }
            spCurrentHeader = &mSoundHeader;
            sysErr = ::waveOutPrepareHeader( sCurrentDeviceHandle,
                                        spCurrentHeader, sizeof( mSoundHeader ) );
            if( sysErr != MMSYSERR_NOERROR )
                return;

            mPlaying = true;
            sysErr = ::waveOutWrite( sCurrentDeviceHandle,
                            spCurrentHeader, sizeof( mSoundHeader ) );
            if( sysErr != MMSYSERR_NOERROR )
            {
                mPlaying = false;
                return;
            }

            break;

        case multipleDeviceInstances:
            // The call to ::waveOutReset() does not seem to result in a
            // noteworthy amount of additional delay time.
            ::waveOutReset( mDeviceHandle );
            // Resetting the flags seems to reduce the onset delay.
            // May only be done after ::waveOutReset().
            mSoundHeader.dwFlags = mSoundFlags;

            mPlaying = true;
            sysErr = ::waveOutWrite( mDeviceHandle, &mSoundHeader, sizeof( mSoundHeader ) );
            if( sysErr != MMSYSERR_NOERROR )
            {
                mPlaying = false;
                return;
            }
            break;

        default:
            assert( false );
    }
}

TWavePlayer::Error
TWavePlayer::SetVolume( float inVolume )
{
  return SetVolumeAndBalance( inVolume, mBalance );
}

TWavePlayer::Error
TWavePlayer::SetBalance( float inBalance )
{
  return SetVolumeAndBalance( mVolume, inBalance );
}

TWavePlayer::Error
TWavePlayer::SetVolumeAndBalance( float inVolume, float inBalance )
{
  mVolume = inVolume;
  mBalance = inBalance;
  const int maxValue = ( 1 << 16 ) - 1;
  int leftVolume = ::floor( ( 1.0 - inBalance ) / 2.0 * mVolume * maxValue ),
      rightVolume = ::floor( ( 1.0 + inBalance ) / 2.0 * mVolume * maxValue );
  if( leftVolume < 0 )
    leftVolume = 0;
  else if( leftVolume > maxValue )
    leftVolume = maxValue;
  if( rightVolume < 0 )
    rightVolume = 0;
  else if( rightVolume > maxValue )
    rightVolume = maxValue;

  HWAVEOUT device = NULL;
  switch( sMode )
  {
    case singleDeviceInstance:
      device = sCurrentDeviceHandle;
      break;

    case multipleDeviceInstances:
      device = mDeviceHandle;
      break;

    default:
      assert( false );
  }
  Error err = genError;
  switch( ::waveOutSetVolume( device, leftVolume | ( rightVolume << 16 ) ) )
  {
    case MMSYSERR_NOERROR:
      err = noError;
      break;

    case MMSYSERR_NOTSUPPORTED:
      err = featureNotSupported;
      break;

    default:
      err = genError;
  }
  return err;
}

void
TWavePlayer::Stop()
{
    switch( sMode )
    {
        case singleDeviceInstance:
            // If this instance of TWavePlayer is playing, stop playback and
            // unprepare our header.
            if( ( sCurrentDeviceHandle != NULL ) && ( spCurrentHeader == &mSoundHeader ) )
            {
                // Stop playback immediately.
                ::waveOutReset( sCurrentDeviceHandle );
            }
            break;

        case multipleDeviceInstances:
            if( mDeviceHandle != NULL )
            {
                // Stop playback immediately.
                ::waveOutReset( mDeviceHandle );
            }
            break;

        default:
            assert( false );
    }
}

float
TWavePlayer::GetPos() const
{
    if( !mPlaying )
        return 0.0;

    HWAVEOUT    device = NULL;
    switch( sMode )
    {
        case singleDeviceInstance:
            if( spCurrentHeader == &mSoundHeader )
                device = sCurrentDeviceHandle;
            break;
        case multipleDeviceInstances:
            device = mDeviceHandle;
            break;
        default:
            assert( false );
    }

    if( device == NULL )
        return 0.0;

    MMTIME      pos;
    MMRESULT    sysErr;
    pos.wType = TIME_SAMPLES;
    sysErr = ::waveOutGetPosition( device, &pos, sizeof( pos ) );
    if( sysErr != MMSYSERR_NOERROR )
        return 0.0;
    assert( pos.wType == TIME_SAMPLES );
    float retVal = ( ( float )pos.u.sample * 1000.0 ) / ( float )mSamplingRate;
    if( !sPositionAccurate )
        retVal *= -1.0;
    return retVal;
}

void
CALLBACK
TWavePlayer::MsgHandler( HWAVEOUT   inDeviceHandle,
                         UINT       inMsg,
                         DWORD      inInstance,
                         DWORD,
                         DWORD )
{
    TWavePlayer *instance = ( TWavePlayer* )inInstance;
    switch( inMsg )
    {
        case WOM_OPEN:
            break;
        case WOM_CLOSE:
        case WOM_DONE:
            instance->mPlaying = false;
            break;
        default:
            assert( false );
    }
}
