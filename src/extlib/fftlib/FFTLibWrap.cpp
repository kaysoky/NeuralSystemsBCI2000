////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An interface to the FFTW library, loading the DLL dynamically.
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
////////////////////////////////////////////////////////////////////////////////
#include "FFTLibWrap.h"

#if USE_DLL
# include <windows.h>
#else
# include <fftw3.h>
#endif

const char* FFTLibWrapper::sLibName = "fftw3";
int FFTLibWrapper::sNumInstances = 0;

#if USE_DLL
void* FFTLibWrapper::sLibRef = ::LoadLibrary( sLibName );
FFTLibWrapper::LibInitRealFn FFTLibWrapper::LibInitReal = NULL;
FFTLibWrapper::LibInitComplexFn FFTLibWrapper::LibInitComplex = NULL;
FFTLibWrapper::LibExecuteFn FFTLibWrapper::LibExecute = NULL;
FFTLibWrapper::LibDestroyFn FFTLibWrapper::LibDestroy = NULL;
FFTLibWrapper::LibCleanupFn FFTLibWrapper::LibCleanup = NULL;
FFTLibWrapper::LibMallocFn FFTLibWrapper::LibMalloc = NULL;
FFTLibWrapper::LibFreeFn FFTLibWrapper::LibFree = NULL;

FFTLibWrapper::ProcNameEntry FFTLibWrapper::sProcNames[] =
{
  { ( void** )&LibInitReal,   "fftw_plan_r2r_1d" },
  { ( void** )&LibInitComplex,"fftw_plan_dft_1d" },
  { ( void** )&LibExecute,    "fftw_execute" },
  { ( void** )&LibDestroy,    "fftw_destroy_plan" },
  { ( void** )&LibCleanup,    "fftw_cleanup" },
  { ( void** )&LibMalloc,     "fftw_malloc" },
  { ( void** )&LibFree,       "fftw_free" },
};
#else // USE_DLL
void* FFTLibWrapper::sLibRef = reinterpret_cast<void*>( 1 );
FFTLibWrapper::LibInitRealFn FFTLibWrapper::LibInitReal
  = reinterpret_cast<FFTLibWrapper::LibInitRealFn>( fftw_plan_r2r_1d );
FFTLibWrapper::LibInitComplexFn FFTLibWrapper::LibInitComplex
  = reinterpret_cast<FFTLibWrapper::LibInitComplexFn>( fftw_plan_dft_1d );
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
  mpInputData( NULL ),
  mpOutputData( NULL ),
  mLibPrivateData( NULL )
{
  ++sNumInstances;
#if USE_DLL
  if( sLibRef && !LibInitReal )
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
  if( --sNumInstances < 1 )
    LibCleanup();

}

void
FFTLibWrapper::Cleanup()
{
  if( mLibPrivateData )
  {
    LibDestroy( mLibPrivateData );
    mLibPrivateData = NULL;
  }
  if( mpInputData )
  {
    LibFree( mpInputData );
    mpInputData = NULL;
  }
  if( mpOutputData )
  {
    LibFree( mpOutputData );
    mpOutputData = NULL;
  }
  mFFTSize = 0;
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

bool
RealFFT::Initialize( int inFFTSize, FFTDirection inDirection, FFTOptimization inOptimization )
{
  Cleanup();
  mFFTSize = inFFTSize;
  mpInputData = LibMalloc( mFFTSize * sizeof( Real ) );
  mpOutputData = LibMalloc( mFFTSize * sizeof( Real ) );
  mLibPrivateData = LibInitReal( mFFTSize, mpInputData, mpOutputData, inDirection, inOptimization );
  return mpInputData && mpOutputData && mLibPrivateData;
}

bool
ComplexFFT::Initialize( int inFFTSize, FFTDirection inDirection, FFTOptimization inOptimization )
{
  Cleanup();
  mFFTSize = inFFTSize;
  mpInputData = LibMalloc( mFFTSize * sizeof( Complex ) );
  mpOutputData = LibMalloc( mFFTSize * sizeof( Complex ) );
  mLibPrivateData = LibInitComplex( mFFTSize, mpInputData, mpOutputData, inDirection, inOptimization );
  return mpInputData && mpOutputData && mLibPrivateData;
}


