/////////////////////////////////////////////////////////////////////////////
//
// File: WavePlayer.cpp
//
// Date: Oct 29, 2001
//
// Authors: Sebastian Halder, Juergen Mellinger
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
//          Dec 14, 2004, halder@informatik.uni-tuebingen.de
//           Reimplemented class using DirectX to enable simultaneous
//           playback of audio files, with different volume and pan settings,
//           on one device.
//
//////////////////////////////////////////////////////////////////////////////
#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include <windows.h>
#include <math.h>
#include <cassert>
#include <iostream>

#include "WavePlayer.h"

LPDIRECTSOUND       TWavePlayer::sPDS = NULL;
LPDIRECTSOUNDBUFFER TWavePlayer::sPrimarySoundBuffer = NULL;
int                 TWavePlayer::sNumInstances = 0;


TWavePlayer::TWavePlayer()
: mVolume( 1.0 ),
  mPan( 0.0 ),
  mSamplingRate( 0 ),
  mPlaying( false ),
  mSecondaryBuffer( NULL )
{
  Construct();
}

TWavePlayer::TWavePlayer( const TWavePlayer& inOriginal )
  : mVolume( 1.0 ),
  mPan( 0.0 ),
  mSamplingRate( 0 ),
  mPlaying( false ),
  mSecondaryBuffer( NULL )
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

  if(sNumInstances<=1)
  {
    HINSTANCE dll_handle=::LoadLibrary("dsound.dll");
    typedef WORD (WINAPI* LPFUNC_DSOUNDCREATE)(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN);
    LPFUNC_DSOUNDCREATE mod_dsoundcreate= reinterpret_cast<LPFUNC_DSOUNDCREATE>(
      ::GetProcAddress(dll_handle,"DirectSoundCreate") );
    if(mod_dsoundcreate!=NULL)
    {
      //create an object that utilizes the direct sound interface
      //NULL causes it to use the default device
      //sPDS will recieve the DirectSound pointer
      //last variable must be null because aggregation(?) is not supported
      if(DS_OK==mod_dsoundcreate(NULL,&sPDS, NULL))
      {
        //set cooplevel to e.g. DSSCL_PRIORITY
        //SetCooperative level determines how this instance of the device
        //interacts with the rest of the system
        if(DS_OK==sPDS->SetCooperativeLevel(::GetDesktopWindow(),DSSCL_EXCLUSIVE))
        {

          DSBUFFERDESC dsbd;
          ::ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
          dsbd.dwSize        = sizeof(DSBUFFERDESC);
          dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
          dsbd.dwBufferBytes = 0;
          dsbd.lpwfxFormat   = NULL;

          //create LPDIRECTSOUNDBUFFER (the actual primary buffer),DSBUFFERDESC, call CreateSoundBuffer()
          sPDS->CreateSoundBuffer( &dsbd, &sPrimarySoundBuffer, NULL );
        }
      }
    }
    if( dll_handle != NULL )
      ::FreeLibrary( dll_handle );
  }
}

void
TWavePlayer::Destruct()
{
  DetachFile();

  --sNumInstances;
  if(sNumInstances<1)
  {
    if(sPrimarySoundBuffer!=NULL)
    {
      sPrimarySoundBuffer->Release();
      sPrimarySoundBuffer=NULL;
    }
    if(sPDS!=NULL)
    {
      sPDS->Release();
      sPDS=NULL;
    }
  }
}

void
TWavePlayer::Assign( const TWavePlayer& inOriginal )
{
  AttachFile( inOriginal.GetFile().c_str() );
  SetVolume(inOriginal.GetVolume());
  SetPan(inOriginal.GetPan());
}

TWavePlayer::Error
TWavePlayer::AttachFile( const char* inFileName )
{
  DetachFile();

  if( inFileName == NULL || *inFileName == '\0' )
    return noError;

  if(sPDS==NULL)
    return initError;

  Error err = noError;

  HMMIO fileHandle = mmioOpen( ( char* )inFileName, NULL, MMIO_READ | MMIO_ALLOCBUF );
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

  // Read the fmt chunk into a WAVEFORMATEX structure.
  WAVEFORMATEX fileFormat;

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

  long dataLength = childChunkInfo.cksize;

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
    mSamplingRate = fileFormat.nSamplesPerSec;
    mBitsPerSample = fileFormat.wBitsPerSample;

    DSBUFFERDESC bufdesc;
    ::memset(&bufdesc, 0, sizeof(DSBUFFERDESC));
    bufdesc.dwSize = sizeof(DSBUFFERDESC);

    bufdesc.dwFlags = DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLPAN|DSBCAPS_GLOBALFOCUS;
    bufdesc.dwBufferBytes = childChunkInfo.cksize;
    bufdesc.lpwfxFormat = &fileFormat;

    err = genError;
    if( DS_OK == sPDS->CreateSoundBuffer( &bufdesc, &mSecondaryBuffer, NULL ) )
    {
      char* pWrite = NULL;
      unsigned long length = 0;
      if( DS_OK == mSecondaryBuffer->Lock( 0, childChunkInfo.cksize,
                     &reinterpret_cast<void*>( pWrite ), &length, NULL, NULL, DSBLOCK_ENTIREBUFFER ) )
      {
        ::mmioRead( fileHandle, pWrite, length );
        if( DS_OK == mSecondaryBuffer->Unlock( pWrite, length, NULL, NULL ) )
          err = noError;
      }
    }
    ::mmioClose( fileHandle, 0 );
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
  if(IsPlaying())
    Stop();

  if(mSecondaryBuffer!=NULL)
  {
    mSecondaryBuffer->Release();
    mSecondaryBuffer=NULL;
  }

  mCurrentFileName = "";
  mSamplingRate = 0;
}

void
TWavePlayer::Play()
{
  if(mSecondaryBuffer!=NULL)
  {
    mSecondaryBuffer->SetCurrentPosition(0);
    mSecondaryBuffer->Play(0,0,0);
    mPlaying=true;
  }
}

TWavePlayer::Error
TWavePlayer::SetVolume( float inVolume )
{
  if(inVolume<0||inVolume>1)
    return invalidParams;

  if(mSecondaryBuffer==NULL)
    return initError;

  if(DS_OK!=mSecondaryBuffer->SetVolume(DSBVOLUME_MIN+(inVolume)*(DSBVOLUME_MAX-DSBVOLUME_MIN)))
    return genError;
  else
    return noError;
}

TWavePlayer::Error
TWavePlayer::SetPan(float inPan)
{
  if(inPan<-1||inPan>1)
    return invalidParams;

  if(mSecondaryBuffer==NULL)
    return initError;

  if(DS_OK!=mSecondaryBuffer->SetPan(DSBPAN_CENTER+(inPan)*(DSBPAN_RIGHT-DSBPAN_CENTER)))
    return genError;
  else
    return noError;
}

void
TWavePlayer::Stop()
{
  if(mSecondaryBuffer!=NULL&&IsPlaying())
    mSecondaryBuffer->Stop();
  mPlaying=false;
}

bool
TWavePlayer::IsPlaying() const
{
  unsigned long retval=false;
  if(mSecondaryBuffer!=NULL)
  {
    mSecondaryBuffer->GetStatus(&retval);
  }
  return (retval==DSBSTATUS_PLAYING);
}

float
TWavePlayer::GetPos() const
{
  if( !IsPlaying()||(mSecondaryBuffer==NULL))
    return 0.0;

  unsigned long       pos;
  float               result;

  mSecondaryBuffer->GetCurrentPosition(&pos,NULL);

  result=(pos*8)/(mSamplingRate*mBitsPerSample);

  return result*1000;
}


