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
#ifndef FFT_LIB_WRAP_H
#define FFT_LIB_WRAP_H

#ifdef _WIN32
# define USE_DLL 1
#endif

#include "BCIAssert.h"

class FFTLibWrapper
{
  public:
    typedef double number;
    enum FFTDirection
    {
      FFTForward = 0,
      FFTBackward = 1,
    };
    FFTLibWrapper();
    ~FFTLibWrapper();

    bool Initialize( int FFTSize, FFTDirection = FFTForward );
    int Size() const;
    number& Input( int index );
    const number& Output( int index ) const;

    void Compute();

    static const char* LibName() { return sLibName; }
    static bool LibAvailable()   { return sLibRef != 0; }

  private:
    void Cleanup();

    int     mFFTSize;
    number* mInputData,
          * mOutputData;
    void*   mLibPrivateData;

    static const char* sLibName;
    static void*  sLibRef;

    typedef void* ( *LibInitFn )( int, number*, number*, int, unsigned );
    typedef void  ( *LibExecuteFn )( void* );
    typedef void  ( *LibDestroyFn )( void* );
    typedef void  ( *LibCleanupFn )();
    typedef void* ( *LibMallocFn )( unsigned long );
    typedef void  ( *LibFreeFn )( void* );

    static LibInitFn    LibInit;
    static LibExecuteFn LibExecute;
    static LibDestroyFn LibDestroy;
    static LibCleanupFn LibCleanup;
    static LibMallocFn  LibMalloc;
    static LibFreeFn    LibFree;
#if USE_DLL
    typedef struct { void** mProc; const char* mName; } ProcNameEntry;
    static ProcNameEntry sProcNames[];
#endif // USE_DLL
};

inline
FFTLibWrapper::number&
FFTLibWrapper::Input( int index )
{
#if !defined( _NDEBUG ) || defined( _DEBUG )
  bciassert( mInputData != 0 && index < mFFTSize );
#endif
  return mInputData[ index ];
}

inline
const FFTLibWrapper::number&
FFTLibWrapper::Output( int index ) const
{
#if !defined( _NDEBUG ) || defined( _DEBUG )
  bciassert( mOutputData != 0 && index < mFFTSize );
#endif
  return mOutputData[ index ];
}

#endif // FFT_LIB_WRAP_H
