////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An interface to the FFTW library, loading the DLL dynamically.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
////////////////////////////////////////////////////////////////////////////////
#include "FFTLibWrap.h"

#if USE_DLL
# include <windows.h>
#else
# include <fftw3.h>
#endif

const char* FFTLibWrapper::sLibName = "fftw3";
#if USE_DLL
void* FFTLibWrapper::sLibRef = ::LoadLibrary( sLibName );
FFTLibWrapper::LibInitFn FFTLibWrapper::LibInit = NULL;
FFTLibWrapper::LibExecuteFn FFTLibWrapper::LibExecute = NULL;
FFTLibWrapper::LibDestroyFn FFTLibWrapper::LibDestroy = NULL;
FFTLibWrapper::LibCleanupFn FFTLibWrapper::LibCleanup = NULL;
FFTLibWrapper::LibMallocFn FFTLibWrapper::LibMalloc = NULL;
FFTLibWrapper::LibFreeFn FFTLibWrapper::LibFree = NULL;

FFTLibWrapper::ProcNameEntry FFTLibWrapper::sProcNames[] =
{
  { ( void** )&LibInit,    "fftw_plan_r2r_1d" },
  { ( void** )&LibExecute, "fftw_execute" },
  { ( void** )&LibDestroy, "fftw_destroy_plan" },
  { ( void** )&LibCleanup, "fftw_cleanup" },
  { ( void** )&LibMalloc,  "fftw_malloc" },
  { ( void** )&LibFree,    "fftw_free" },
};
#else // USE_DLL
void* FFTLibWrapper::sLibRef = reinterpret_cast<void*>( 1 );
FFTLibWrapper::LibInitFn FFTLibWrapper::LibInit
  = reinterpret_cast<FFTLibWrapper::LibInitFn>( fftw_plan_r2r_1d );
FFTLibWrapper::LibExecuteFn FFTLibWrapper::LibExecute
  = reinterpret_cast<FFTLibWrapper::LibExecuteFn>( fftw_execute );
FFTLibWrapper::LibDestroyFn FFTLibWrapper::LibDestroy
  = reinterpret_cast<FFTLibWrapper::LibDestroyFn>( fftw_destroy_plan );
FFTLibWrapper::LibCleanupFn FFTLibWrapper::LibCleanup
  = reinterpret_cast<FFTLibWrapper::LibCleanupFn>( fftw_cleanup );
FFTLibWrapper::LibMallocFn FFTLibWrapper::LibMalloc
  = reinterpret_cast<FFTLibWrapper::LibMallocFn>( fftw_malloc );
FFTLibWrapper::LibFreeFn FFTLibWrapper::LibFree
  = reinterpret_cast<FFTLibWrapper::LibFreeFn>( fftw_free );
#endif // USE_DLL


FFTLibWrapper::FFTLibWrapper()
: mFFTSize( 0 ),
  mInputData( NULL ),
  mOutputData( NULL ),
  mLibPrivateData( NULL )
{
#if USE_DLL
  if( sLibRef && !LibInit )
  {
    bool foundAllProcs = true;
    for( size_t i = 0; foundAllProcs && i < sizeof( sProcNames ) / sizeof( *sProcNames ); ++i )
    {
      void* libProc = ( void* )::GetProcAddress( ( HMODULE )sLibRef, sProcNames[ i ].mName );
      if( libProc == NULL )
        foundAllProcs = false;
      *sProcNames[ i ].mProc = libProc;
    }
    if( !foundAllProcs )
      sLibRef = NULL;
  }
#endif // USE_DLL
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
    LibDestroy( mLibPrivateData );
    mLibPrivateData = NULL;
  }
  if( mInputData )
  {
    LibFree( mInputData );
    mInputData = NULL;
  }
  if( mOutputData )
  {
    LibFree( mOutputData );
    mOutputData = NULL;
  }
  mFFTSize = 0;
  LibCleanup();
}

bool
FFTLibWrapper::Initialize( int inFFTSize, FFTDirection inDirection )
{
  Cleanup();
  mFFTSize = inFFTSize;
  mInputData = reinterpret_cast<number*>( LibMalloc( mFFTSize * sizeof( number ) ) );
  mOutputData = reinterpret_cast<number*>( LibMalloc( mFFTSize * sizeof( number ) ) );
  mLibPrivateData = LibInit( mFFTSize, mInputData, mOutputData, inDirection, 1U << 6 );
  return mInputData && mOutputData && mLibPrivateData;
}

int
FFTLibWrapper::Size() const
{
  return mFFTSize;
}

void
FFTLibWrapper::Compute()
{
  LibExecute( mLibPrivateData );
}


