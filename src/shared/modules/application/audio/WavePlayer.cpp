/////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de,
//          halder@informatik.uni-tuebingen.de
// Description: The Win32 implementation of the WavePlayer interface.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "WavePlayer.h"
#include "FileUtils.h"
#include "BCIError.h"

using namespace std;

#if USE_DSOUND

LPDIRECTSOUND       WavePlayer::sPDS = NULL;
LPDIRECTSOUNDBUFFER WavePlayer::sPrimarySoundBuffer = NULL;
int                 WavePlayer::sNumInstances = 0;


WavePlayer::WavePlayer()
: mVolume( 1.0 ),
  mPan( 0.0 ),
  mSamplingRate( 0 ),
  mSecondaryBuffer( NULL ),
  mErrorState( noError )
{
  Construct();
}

WavePlayer::WavePlayer( const WavePlayer& inOriginal )
: mVolume( 1.0 ),
  mPan( 0.0 ),
  mSamplingRate( 0 ),
  mSecondaryBuffer( NULL ),
  mErrorState( noError )
{
  Construct();
  Assign( inOriginal );
}

WavePlayer&
WavePlayer::operator=( const WavePlayer& inOriginal )
{
  if( &inOriginal != this )
  {
    Destruct();
    Construct();
    Assign( inOriginal );
  }
  return *this;
}

WavePlayer::~WavePlayer()
{
  Destruct();
}

void
WavePlayer::Construct()
{
  ++sNumInstances;

  if( sNumInstances <= 1 )
  {
    HINSTANCE dllHandle = ::LoadLibrary( "dsound.dll" );
    typedef WORD ( WINAPI* LPFUNC_DSOUNDCREATE )( LPCGUID, LPDIRECTSOUND*, LPUNKNOWN );
    LPFUNC_DSOUNDCREATE DSoundCreate = reinterpret_cast<LPFUNC_DSOUNDCREATE>(
      ::GetProcAddress( dllHandle, "DirectSoundCreate" ) );
    if( DSoundCreate != NULL )
    {
      // Create an object that utilizes the direct sound interface.
      //  NULL causes it to use the default device,
      //  sPDS will receive the DirectSound pointer.
      //  The last variable must be null because aggregation is not supported.
      if( DS_OK == DSoundCreate( NULL, &sPDS, NULL ) )
      {
        // SetCooperative level determines how this instance of the device
        // interacts with the rest of the system.
        if( DS_OK == sPDS->SetCooperativeLevel( ::GetDesktopWindow(), DSSCL_EXCLUSIVE ) )
        {
          DSBUFFERDESC dsbd;
          ::ZeroMemory( &dsbd, sizeof( DSBUFFERDESC ) );
          dsbd.dwSize        = sizeof( DSBUFFERDESC );
          dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
          dsbd.dwBufferBytes = 0;
          dsbd.lpwfxFormat   = NULL;
          sPDS->CreateSoundBuffer( &dsbd, &sPrimarySoundBuffer, NULL );
        }
      }
    }
    if( dllHandle != NULL )
      ::FreeLibrary( dllHandle );
  }
  if( sPrimarySoundBuffer == NULL )
    mErrorState = initError;
}

void
WavePlayer::Destruct()
{
  Clear();

  --sNumInstances;
  if( sNumInstances < 1 )
  {
    if( sPrimarySoundBuffer != NULL )
    {
      sPrimarySoundBuffer->Release();
      sPrimarySoundBuffer = NULL;
    }
    if( sPDS != NULL )
    {
      sPDS->Release();
      sPDS=NULL;
    }
  }
}

void
WavePlayer::Assign( const WavePlayer& inOriginal )
{
  SetFile( inOriginal.File() );
  SetVolume( inOriginal.Volume() );
  SetPan( inOriginal.Pan() );
}

void
WavePlayer::Clear()
{
  if( IsPlaying() )
    Stop();

  if( mSecondaryBuffer != NULL )
  {
    mSecondaryBuffer->Release();
    mSecondaryBuffer = NULL;
  }
  mFile = "";
  mSamplingRate = 0;
}

WavePlayer&
WavePlayer::SetFile( const string& inFileName )
{
  Clear();

  if( inFileName.empty() )
    return *this;

  Error err = noError;

  string fileName = FileUtils::AbsolutePath( inFileName );
  HMMIO fileHandle = mmioOpen( const_cast<char*>( fileName.c_str() ), NULL, MMIO_READ | MMIO_ALLOCBUF );
  if( fileHandle == NULL )
    err = fileOpeningError;

  MMCKINFO parentChunkInfo,
           childChunkInfo;
  MMIOINFO info;
  MMRESULT sysErr;

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
    if( mmioRead( fileHandle, ( char* )&fileFormat, childChunkInfo.cksize )
        != ( long )childChunkInfo.cksize )
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

  long dataLength = 0;
  if( err == noError )
  {
    // Resize the file buffer to contain all data.
    dataLength = childChunkInfo.cksize;
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
    if( sysErr != MMSYSERR_NOERROR || info.cchBuffer != dataLength )
      err = fileOpeningError;
  }

  if( err == noError )
  {
    mSamplingRate = fileFormat.nSamplesPerSec;
    mBitsPerSample = fileFormat.wBitsPerSample;

    DSBUFFERDESC bufdesc;
    ::memset( &bufdesc, 0, sizeof( DSBUFFERDESC ) );
    bufdesc.dwSize = sizeof( DSBUFFERDESC );
    bufdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_GLOBALFOCUS;
    bufdesc.dwBufferBytes = childChunkInfo.cksize;
    bufdesc.lpwfxFormat = &fileFormat;

    err = genError;
    if( DS_OK == sPDS->CreateSoundBuffer( &bufdesc, &mSecondaryBuffer, NULL ) )
    {
      void* pWrite = NULL;
      unsigned long length = 0;
      if( DS_OK == mSecondaryBuffer->Lock( 0, childChunkInfo.cksize,
                     &pWrite, &length, NULL, NULL, DSBLOCK_ENTIREBUFFER ) )
      {
        ::mmioRead( fileHandle, reinterpret_cast<char*>( pWrite ), length );
        if( DS_OK == mSecondaryBuffer->Unlock( pWrite, length, NULL, 0 ) )
          err = noError;
      }
    }
    ::mmioClose( fileHandle, 0 );
  }
  if( err != noError )
    Clear();
  else
    mFile = inFileName;

  mErrorState = err;
  return *this;
}

WavePlayer&
WavePlayer::Play()
{
  if( mSecondaryBuffer != NULL )
  {
    mSecondaryBuffer->SetCurrentPosition( 0 );
    mSecondaryBuffer->Play( 0, 0, 0 );
  }
  return *this;
}

WavePlayer&
WavePlayer::SetVolume( float inVolume )
{
  LONG value = static_cast<LONG>( DSBVOLUME_MIN + inVolume *( DSBVOLUME_MAX - DSBVOLUME_MIN ) );
  if( inVolume < 0 || inVolume > 1 )
    mErrorState = invalidParams;
  else if( mSecondaryBuffer == NULL )
    mErrorState = initError;
  else if( DS_OK != mSecondaryBuffer->SetVolume( value ) )
    mErrorState = genError;
  else
    mErrorState = noError;
  return *this;
}

WavePlayer&
WavePlayer::SetPan( float inPan )
{
  LONG value = static_cast<LONG>( DSBPAN_CENTER + inPan * ( DSBPAN_RIGHT - DSBPAN_CENTER ) );
  if( inPan < -1 || inPan > 1 )
    mErrorState = invalidParams;
  else if( mSecondaryBuffer == NULL )
    mErrorState = initError;
  else if( DS_OK != mSecondaryBuffer->SetPan( value ) )
    mErrorState = genError;
  else
    mErrorState = noError;
  return *this;
}

WavePlayer&
WavePlayer::Stop()
{
  if( mSecondaryBuffer != NULL && IsPlaying() )
    mSecondaryBuffer->Stop();
  return *this;
}

bool
WavePlayer::IsPlaying() const
{
  bool result = false;
  if( mSecondaryBuffer != NULL )
  {
    DWORD status;
    if( DS_OK == mSecondaryBuffer->GetStatus( &status ) )
      result = ( status & DSBSTATUS_PLAYING );
  }
  return result;
}

float
WavePlayer::PlayingPos() const
{
  if( !IsPlaying() || ( mSecondaryBuffer == NULL ) )
    return 0.0;

  unsigned long pos;
  mSecondaryBuffer->GetCurrentPosition( &pos, NULL );
  return 1e3f * ( pos * 8 ) / ( mSamplingRate * mBitsPerSample );
}

#else // USE_DSOUND

WavePlayer::WavePlayer()
: mVolume( 1.0 ),
  mPan( 0.0 ),
  mErrorState( noError ),
  mpSound( NULL ),
  mVolumeWarningIssued( false ),
  mPanWarningIssued( false )
{
  Construct();
}

WavePlayer::WavePlayer( const WavePlayer& inOriginal )
: mVolume( 1.0 ),
  mPan( 0.0 ),
  mErrorState( noError ),
  mpSound( NULL ),
  mVolumeWarningIssued( false ),
  mPanWarningIssued( false )
{
  Construct();
  Assign( inOriginal );
}

WavePlayer&
WavePlayer::operator=( const WavePlayer& inOriginal )
{
  if( &inOriginal != this )
  {
    Destruct();
    Construct();
    Assign( inOriginal );
  }
  return *this;
}

WavePlayer::~WavePlayer()
{
  Destruct();
}

void
WavePlayer::Construct()
{
}

void
WavePlayer::Destruct()
{
  Clear();
}

void
WavePlayer::Assign( const WavePlayer& inOriginal )
{
  SetFile( inOriginal.File() );
  SetVolume( inOriginal.Volume() );
  SetPan( inOriginal.Pan() );
}

void
WavePlayer::Clear()
{
  if( IsPlaying() )
    Stop();

  delete mpSound;
  mpSound = NULL;
  mFile = "";
}

WavePlayer&
WavePlayer::SetFile( const string& inFileName )
{
  Clear();
  if( !inFileName.empty() )
  {
    mFile = inFileName;
    mpSound = new QSound( inFileName.c_str() );
  }
  return *this;
}

WavePlayer&
WavePlayer::Play()
{
  if( mpSound )
    mpSound->play();
  return *this;
}

WavePlayer&
WavePlayer::SetVolume( float inVolume )
{
  if( inVolume != 1.0 && !mVolumeWarningIssued )
  {
    bciout << "Cannot adjust volume in non-windows builds" << endl;
    mVolumeWarningIssued = true;
  }
  mVolume = 1.0;
  return *this;
}

WavePlayer&
WavePlayer::SetPan( float inPan )
{
  if( inPan != 0.0 && !mPanWarningIssued )
  {
    bciout << "Cannot adjust pan in non-windows builds" << endl;
    mPanWarningIssued = true;
  }
  mPan = 0.0;
  return *this;
}

WavePlayer&
WavePlayer::Stop()
{
  if( mpSound )
    mpSound->stop();
  return *this;
}

bool
WavePlayer::IsPlaying() const
{
  return mpSound && !mpSound->isFinished();
}

float
WavePlayer::PlayingPos() const
{
  return IsPlaying() ? -1.0 : 0.0;
}

#endif // USE_DSOUND
