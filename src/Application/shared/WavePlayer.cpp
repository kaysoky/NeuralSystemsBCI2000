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
// Changes:
//
//////////////////////////////////////////////////////////////////////////////
#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include <assert>

#include "WavePlayer.h"

HWAVEOUT TWavePlayer::currentDeviceHandle = NULL;
WAVEHDR* TWavePlayer::currentHeader = NULL;

TWavePlayer::TOperationMode TWavePlayer::mode = unknown;
bool                        TWavePlayer::positionAccurate = false;
int                         TWavePlayer::numInstances = 0;


TWavePlayer::TWavePlayer()
: samplingRate( 0 ),
  playing( false ),
  deviceHandle( NULL ),
  fileHandle( NULL )
{
    ++numInstances;
    if( mode == unknown )
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

        sysErr = waveOutOpen( &deviceHandle1, WAVE_MAPPER, &format,
                                            NULL, 0, CALLBACK_NULL );
        if( sysErr != MMSYSERR_NOERROR )
            mode = singleDeviceInstance;
        else
        {
            sysErr = waveOutOpen( &deviceHandle2, WAVE_MAPPER, &format,
                                                NULL, 0, CALLBACK_NULL );
            if( sysErr != MMSYSERR_NOERROR )
            {
                mode = singleDeviceInstance;
                waveOutClose( deviceHandle1 );
            }
            else
            {
                mode = multipleDeviceInstances;
                waveOutClose( deviceHandle1 );
                waveOutClose( deviceHandle2 );
            }
        }

        // Determine if the device supports retrieving a sample-accurate position.
        WAVEOUTCAPS caps;
        sysErr = waveOutGetDevCaps( WAVE_MAPPER, &caps, sizeof( caps ) );
        if( sysErr == MMSYSERR_NOERROR )
            positionAccurate = ( caps.dwSupport & WAVECAPS_SAMPLEACCURATE );
    }
}

TWavePlayer::~TWavePlayer()
{
    --numInstances;

    // This happens in singleDeviceInstance mode.
    if( numInstances < 1 && currentDeviceHandle != NULL )
    {
        // If the device is open, close it.
        waveOutReset( currentDeviceHandle );
        if( currentHeader != NULL )
        {
            waveOutUnprepareHeader( currentDeviceHandle,
                                    currentHeader, sizeof( soundHeader ) );
            currentHeader = NULL;
        }
        waveOutClose( currentDeviceHandle );
        currentDeviceHandle = NULL;
    }

    DetachFile();
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

    fileHandle = mmioOpen( ( char* )inFileName, NULL, MMIO_READ | MMIO_ALLOCBUF );
    if( fileHandle == NULL )
        err = fileOpeningError;

    MMCKINFO    parentChunkInfo,
                childChunkInfo;
    MMIOINFO    info;
    MMRESULT    sysErr;

    if( err == noError )
    {
        // Read the chunks we need to play the file.
        parentChunkInfo.fccType = mmioFOURCC( 'W', 'A', 'V', 'E' );
        sysErr = mmioDescend( fileHandle, &parentChunkInfo, NULL, MMIO_FINDRIFF );
        if( sysErr != MMSYSERR_NOERROR )
            // The file doesn't contain a WAVE chunk.
            err = fileOpeningError;
    }

    if( err == noError )
    {
        childChunkInfo.ckid = mmioFOURCC( 'f', 'm', 't', ' ' );
        sysErr = mmioDescend( fileHandle, &childChunkInfo, &parentChunkInfo, MMIO_FINDCHUNK );
        if( sysErr != MMSYSERR_NOERROR )
            // The file doesn't contain a fmt chunk.
            err = fileOpeningError;
    }

    // Read the fmt chunk into the WAVEFORMATEX structure.
    if( err == noError )
    {
        fileFormat.cbSize = 0;
        if( childChunkInfo.cksize > sizeof( fileFormat ) )
            // The chunk does not fit into the WAVEFORMATEX structure.
            err = fileOpeningError;
    }

    if( err == noError )
    {
        if( mmioRead( fileHandle, ( char* )&fileFormat, childChunkInfo.cksize ) != ( long )childChunkInfo.cksize )
            // There is not enough data in the file.
            err = fileOpeningError;
    }

    if( err == noError )
    {
        mmioAscend( fileHandle, &childChunkInfo, 0 );
        // Get to the data chunk.
        childChunkInfo.ckid = mmioFOURCC( 'd', 'a', 't', 'a' );
        sysErr = mmioDescend( fileHandle, &childChunkInfo, &parentChunkInfo, MMIO_FINDCHUNK );
        if( sysErr != MMSYSERR_NOERROR )
            // The file doesn't contain a data chunk.
            err = fileOpeningError;
    }

    long    dataLength = childChunkInfo.cksize;

    if( err == noError )
    {
        // Resize the file buffer to contain all data.
        sysErr = mmioSetBuffer( fileHandle, NULL, dataLength, 0 );
        if( sysErr != MMSYSERR_NOERROR )
            // The new buffer size could not be set. Should not happen
            // in a WIN32 environment if virtual memory is configured
            // reasonably.
            err = fileOpeningError;
    }

    if( err == noError )
    {
        // Make sure the data is in the buffer.
        sysErr = mmioAdvance( fileHandle, NULL, MMIO_READ );
        if( sysErr != MMSYSERR_NOERROR )
            err = fileOpeningError;
    }

    if( err == noError )
    {
        // Get a pointer to the buffer and enter it into the WAVEHDR structure.
        sysErr = mmioGetInfo( fileHandle, &info, 0 );
        if( sysErr != MMSYSERR_NOERROR )
            err = fileOpeningError;
    }

    if( err == noError )
    {
        assert( info.cchBuffer == dataLength );
        soundHeader.lpData = info.pchBuffer;
        soundHeader.dwBufferLength = dataLength;
        soundHeader.dwBytesRecorded = 0;
        soundHeader.dwUser = 0;
        soundHeader.dwFlags = 0;
        soundHeader.dwLoops = 0;
        soundHeader.lpNext = NULL;
        soundHeader.reserved = 0;

        samplingRate = fileFormat.nSamplesPerSec;

        switch( mode )
        {
            case singleDeviceInstance:
                // We cannot open the device and keep it open because the device cannot
                // be opened twice.
                break;

            case multipleDeviceInstances:
                // Try opening a wave output device suited for the file's format.
                sysErr = waveOutOpen( &deviceHandle, WAVE_MAPPER, &fileFormat,
                                        ( DWORD )MsgHandler, ( DWORD )this, CALLBACK_FUNCTION );
                if( sysErr != MMSYSERR_NOERROR )
                    err = genError;

                if( err == noError )
                {
                    // Prepare the sound header for playing.
                    sysErr = waveOutPrepareHeader( deviceHandle, &soundHeader, sizeof( soundHeader ) );
                    if( sysErr != MMSYSERR_NOERROR )
                        err = genError;
                    else
                        soundFlags = soundHeader.dwFlags;
                }
                break;
            default:
                // This should not happen.
                assert( false );
        }
    }

    if( err != noError )
      DetachFile();

    return err;
}

void
TWavePlayer::DetachFile()
{
    switch( mode )
    {
        case singleDeviceInstance:
            // If this instance of TWavePlayer is playing, stop playback and
            // unprepare our header.
            if( ( currentDeviceHandle != NULL ) && ( currentHeader == &soundHeader ) )
            {
                // Stop playback immediately.
                waveOutReset( currentDeviceHandle );
                // Free internal buffer memory.
                waveOutUnprepareHeader( currentDeviceHandle, currentHeader, sizeof( soundHeader ) );
                // Close the device.
                waveOutClose( currentDeviceHandle );
                currentDeviceHandle = NULL;
                currentHeader = NULL;
            }
            break;

        case multipleDeviceInstances:
            if( deviceHandle != NULL )
            {
                // Stop playback immediately.
                waveOutReset( deviceHandle );
                // Free internal buffer memory.
                waveOutUnprepareHeader( deviceHandle, &soundHeader, sizeof( soundHeader ) );
                // Close device instance.
                waveOutClose( deviceHandle );
                deviceHandle = NULL;
            }
            break;

        default:
            assert( false );
    }

    if( fileHandle != NULL )
        mmioClose( fileHandle, 0 );
    fileHandle = NULL;
    samplingRate = 0;
}

void
TWavePlayer::Play()
{
    if( fileHandle == NULL )
        return;

    MMRESULT    sysErr;

    switch( mode )
    {
        case singleDeviceInstance:
            if( ( currentDeviceHandle != NULL ) && ( currentHeader != NULL ) )
            {
                // If the device is open, close it.
                waveOutReset( currentDeviceHandle );
                waveOutUnprepareHeader( currentDeviceHandle,
                                        currentHeader, sizeof( soundHeader ) );
                waveOutClose( currentDeviceHandle );
                currentHeader = NULL;
            }

            sysErr = waveOutOpen( &currentDeviceHandle, WAVE_MAPPER, &fileFormat,
                                ( DWORD )MsgHandler, ( DWORD )this, CALLBACK_FUNCTION );
            if( sysErr != MMSYSERR_NOERROR )
            {
                currentDeviceHandle = NULL;
                return;
            }
            currentHeader = &soundHeader;
            sysErr = waveOutPrepareHeader( currentDeviceHandle,
                                        currentHeader, sizeof( soundHeader ) );
            if( sysErr != MMSYSERR_NOERROR )
                return;

            playing = true;
            sysErr = waveOutWrite( currentDeviceHandle,
                            currentHeader, sizeof( soundHeader ) );
            if( sysErr != MMSYSERR_NOERROR )
            {
                playing = false;
                return;
            }

            break;

        case multipleDeviceInstances:
            // The call to waveOutReset() does not seem to result in a
            // noteworthy amount of additional delay time.
            waveOutReset( deviceHandle );
            // Resetting the flags seems to reduce the onset delay.
            // May only be done after waveOutReset().
            soundHeader.dwFlags = soundFlags;

            playing = true;
            sysErr = waveOutWrite( deviceHandle, &soundHeader, sizeof( soundHeader ) );
            if( sysErr != MMSYSERR_NOERROR )
            {
                playing = false;
                return;
            }
            break;

        default:
            assert( false );
    }
}

void
TWavePlayer::Stop()
{
    switch( mode )
    {
        case singleDeviceInstance:
            // If this instance of TWavePlayer is playing, stop playback and
            // unprepare our header.
            if( ( currentDeviceHandle != NULL ) && ( currentHeader == &soundHeader ) )
            {
                // Stop playback immediately.
                waveOutReset( currentDeviceHandle );
            }
            break;

        case multipleDeviceInstances:
            if( deviceHandle != NULL )
            {
                // Stop playback immediately.
                waveOutReset( deviceHandle );
            }
            break;

        default:
            assert( false );
    }
}

float
TWavePlayer::GetPos() const
{
    if( !playing )
        return 0.0;

    HWAVEOUT    device = NULL;
    switch( mode )
    {
        case singleDeviceInstance:
            if( currentHeader == &soundHeader )
                device = currentDeviceHandle;
            break;
        case multipleDeviceInstances:
            device = deviceHandle;
            break;
        default:
            assert( false );
    }

    if( device == NULL )
        return 0.0;

    MMTIME      pos;
    MMRESULT    sysErr;
    pos.wType = TIME_SAMPLES;
    sysErr = waveOutGetPosition( device, &pos, sizeof( pos ) );
    if( sysErr != MMSYSERR_NOERROR )
        return 0.0;
    assert( pos.wType == TIME_SAMPLES );
    float retVal = ( ( float )pos.u.sample * 1000.0 ) / ( float )samplingRate;
    if( !positionAccurate )
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
            instance->playing = false;
            break;
        default:
            assert( false );
    }
}
