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

#if _WIN32
# include <windows.h>
# define dlopen( x, y )  LoadLibrary( x )
# define dlsym( x, y )   GetProcAddress( ( HMODULE )x, y )
#else // _WIN32
# include <dlfcn.h>
#endif // _WIN32

#if _WIN32
# define LIB_EXTENSION ""
#elif __APPLE__
# define LIB_EXTENSION ".dylib"
#else
# define LIB_EXTENSION ".so"
#endif

const char* FFTLibWrapper::sLibName = "fftw3" LIB_EXTENSION;
int FFTLibWrapper::sNumInstances = 0;

void* FFTLibWrapper::sLibRef = ::dlopen( sLibName, RTLD_LAZY );
FFTLibWrapper::LibInitRealFn FFTLibWrapper::LibInitReal = { NULL };
FFTLibWrapper::LibInitComplexFn FFTLibWrapper::LibInitComplex = { NULL };
FFTLibWrapper::LibExecuteFn FFTLibWrapper::LibExecute = { NULL };
FFTLibWrapper::LibDestroyFn FFTLibWrapper::LibDestroy = { NULL };
FFTLibWrapper::LibCleanupFn FFTLibWrapper::LibCleanup = { NULL };
FFTLibWrapper::LibMallocFn FFTLibWrapper::LibMalloc = { NULL };
FFTLibWrapper::LibFreeFn FFTLibWrapper::LibFree = { NULL };

FFTLibWrapper::ProcNameEntry FFTLibWrapper::sProcNames[] =
{
  { &LibInitReal.Ptr,   "fftw_plan_r2r_1d" },
  { &LibInitComplex.Ptr,"fftw_plan_dft_1d" },
  { &LibExecute.Ptr,    "fftw_execute" },
  { &LibDestroy.Ptr,    "fftw_destroy_plan" },
  { &LibCleanup.Ptr,    "fftw_cleanup" },
  { &LibMalloc.Ptr,     "fftw_malloc" },
  { &LibFree.Ptr,       "fftw_free" },
};


FFTLibWrapper::FFTLibWrapper()
: mFFTSize( 0 ),
  mpInputData( NULL ),
  mpOutputData( NULL ),
  mLibPrivateData( NULL )
{
  ++sNumInstances;
  if( sLibRef && !LibInitReal.Fn )
  {
    bool foundAllProcs = true;
    for( size_t i = 0; foundAllProcs && i < sizeof( sProcNames ) / sizeof( *sProcNames ); ++i )
    {
      void* libProc = ( void* )::dlsym( sLibRef, sProcNames[ i ].mName );
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
  --sNumInstances;
  if( LibAvailable() )
  {
    Cleanup();
    if( sNumInstances < 1 )
      LibCleanup.Fn();

  }
}

void
FFTLibWrapper::Cleanup()
{
  if( mLibPrivateData )
  {
    LibDestroy.Fn( mLibPrivateData );
    mLibPrivateData = NULL;
  }
  if( mpInputData )
  {
    LibFree.Fn( mpInputData );
    mpInputData = NULL;
  }
  if( mpOutputData )
  {
    LibFree.Fn( mpOutputData );
    mpOutputData = NULL;
  }
  mFFTSize = 0;
}

bool
RealFFT::Initialize( int inFFTSize, FFTDirection inDirection, FFTOptimization inOptimization )
{
  Cleanup();
  mFFTSize = inFFTSize;
  mpInputData = LibMalloc.Fn( mFFTSize * sizeof( Real ) );
  mpOutputData = LibMalloc.Fn( mFFTSize * sizeof( Real ) );
  mLibPrivateData = LibInitReal.Fn( mFFTSize, mpInputData, mpOutputData, inDirection, inOptimization );
  return mpInputData && mpOutputData && mLibPrivateData;
}

bool
ComplexFFT::Initialize( int inFFTSize, FFTDirection inDirection, FFTOptimization inOptimization )
{
  Cleanup();
  mFFTSize = inFFTSize;
  mpInputData = LibMalloc.Fn( mFFTSize * sizeof( Complex ) );
  mpOutputData = LibMalloc.Fn( mFFTSize * sizeof( Complex ) );
  mLibPrivateData = LibInitComplex.Fn( mFFTSize, mpInputData, mpOutputData, inDirection, inOptimization );
  return mpInputData && mpOutputData && mLibPrivateData;
}
