////////////////////////////////////////////////////////////////////////////////
//
// File: FFTLibWrap.cpp
//
// Date: Dec 8, 2003
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: An interface layer to a FFT DLL that allows for user
//        installation of the GPL protected FFTW library. Its sole purpose
//        is to allow using FFTW from a non-GPLed project by requiring
//        user interaction and avoiding references to copyrighted FFTW code.
//
////////////////////////////////////////////////////////////////////////////////
#include "FFTLibWrap.h"

#include <windows.h>
#include <assert>

const char* FFTLibWrapper::sLibName = "fftw3";
void* FFTLibWrapper::sLibRef = ::LoadLibrary( sLibName );
void* ( *FFTLibWrapper::libInit )( int, number*, number*, int, unsigned ) = NULL;
void  ( *FFTLibWrapper::libExecute )( void* ) = NULL;
void  ( *FFTLibWrapper::libDestroy )( void* ) = NULL;
void  ( *FFTLibWrapper::libCleanup )() = NULL;
void* ( *FFTLibWrapper::libMalloc )( unsigned long ) = NULL;
void  ( *FFTLibWrapper::libFree )( void* ) = NULL;

FFTLibWrapper::ProcNameEntry FFTLibWrapper::sProcNames[] =
{
  { ( void** )&libInit,    "fftw_plan_r2r_1d" },
  { ( void** )&libExecute, "fftw_execute" },
  { ( void** )&libDestroy, "fftw_destroy_plan" },
  { ( void** )&libCleanup, "fftw_cleanup" },
  { ( void** )&libMalloc,  "fftw_malloc" },
  { ( void** )&libFree,    "fftw_free" },
};

FFTLibWrapper::FFTLibWrapper()
: mFFTSize( 0 ),
  mInputData( NULL ),
  mOutputData( NULL ),
  mLibPrivateData( NULL )
{
  if( sLibRef && !libInit )
  {
    bool foundAllProcs = true;
    for( int i = 0; foundAllProcs && i < sizeof( sProcNames ) / sizeof( *sProcNames ); ++i )
    {
      void* libProc = ::GetProcAddress( sLibRef, sProcNames[ i ].mName );
      if( libProc == NULL )
        foundAllProcs = false;
      *sProcNames[ i ].mProc = libProc;
    }
    if( !foundAllProcs )
      sLibRef = NULL;
  }
}

FFTLibWrapper::~FFTLibWrapper()
{
  if( LibAvailable() )
    Cleanup();
}

void
FFTLibWrapper::Cleanup()
{
  if( mLibPrivateData )
  {
    libDestroy( mLibPrivateData );
    mLibPrivateData = NULL;
  }
  if( mInputData )
  {
    libFree( mInputData );
    mInputData = NULL;
  }
  if( mOutputData )
  {
    libFree( mOutputData );
    mOutputData = NULL;
  }
  mFFTSize = 0;
  libCleanup();
}

bool
FFTLibWrapper::Initialize( int inFFTSize, FFTDirection inDirection )
{
  Cleanup();
  mFFTSize = inFFTSize;
  mInputData = reinterpret_cast<number*>( libMalloc( mFFTSize * sizeof( number ) ) );
  mOutputData = reinterpret_cast<number*>( libMalloc( mFFTSize * sizeof( number ) ) );
  mLibPrivateData = libInit( mFFTSize, mInputData, mOutputData, inDirection, 1U << 6 );
  return mInputData && mOutputData && mLibPrivateData;
}

void
FFTLibWrapper::Transform()
{
  libExecute( mLibPrivateData );
}


