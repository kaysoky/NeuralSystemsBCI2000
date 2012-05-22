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
#ifndef FFT_LIB_WRAP_H
#define FFT_LIB_WRAP_H

#include <complex>
#include "BCIAssert.h"

class FFTLibWrapper
{
  public:
    typedef double Real;
    typedef std::complex<Real> Complex;

    enum FFTDirection
    {
      FFTForward = -1,
      FFTBackward = 1,
    };

    enum FFTOptimization
    {
      Measure = 0,
      Estimate = 1U << 6,
    };

    int Size() const { return mFFTSize; }
    void Compute()   { LibExecute.Fn( mLibPrivateData ); }

    static const char* LibName() { return sLibName; }
    static bool LibAvailable()   { return sLibRef != 0; }

  protected:
    FFTLibWrapper();
    virtual ~FFTLibWrapper();
    void Cleanup();

    int     mFFTSize;
    void*   mpInputData,
        *   mpOutputData;
    void*   mLibPrivateData;

    typedef void* ( *LibInitReal_ )( int, void*, void*, int, unsigned );
    typedef void* ( *LibInitComplex_ )( int, void*, void*, int, unsigned );
    typedef void  ( *LibExecute_ )( void* );
    typedef void  ( *LibDestroy_ )( void* );
    typedef void  ( *LibCleanup_ )();
    typedef void* ( *LibMalloc_ )( unsigned long );
    typedef void  ( *LibFree_ )( void* );

    typedef union { LibInitReal_ Fn; void* Ptr; } LibInitRealFn;
    typedef union { LibInitComplex_ Fn; void* Ptr; } LibInitComplexFn;
    typedef union { LibExecute_ Fn; void* Ptr; } LibExecuteFn;
    typedef union { LibDestroy_ Fn; void* Ptr; } LibDestroyFn;
    typedef union { LibCleanup_ Fn; void* Ptr; } LibCleanupFn;
    typedef union { LibMalloc_ Fn; void* Ptr; } LibMallocFn;
    typedef union { LibFree_ Fn; void* Ptr; } LibFreeFn;

    static LibInitRealFn LibInitReal;
    static LibInitComplexFn LibInitComplex;
    static LibExecuteFn LibExecute;
    static LibDestroyFn LibDestroy;
    static LibCleanupFn LibCleanup;
    static LibMallocFn LibMalloc;
    static LibFreeFn LibFree;

  private:
    static int sNumInstances;
    static const char* sLibName;
    static void*  sLibRef;

    typedef struct { void** mProc; const char* mName; } ProcNameEntry;
    static ProcNameEntry sProcNames[];
};

class RealFFT : public FFTLibWrapper
{
  public:
    bool Initialize( int FFTSize, FFTDirection = FFTForward, FFTOptimization = Estimate );
    Real& Input( int index );
    const Real& Output( int index ) const;
};

class ComplexFFT : public FFTLibWrapper
{
  public:
    bool Initialize( int FFTSize, FFTDirection = FFTForward, FFTOptimization = Estimate );
    Complex& Input( int index );
    const Complex& Output( int index ) const;
};

inline
FFTLibWrapper::Real&
RealFFT::Input( int index )
{
#if BCIDEBUG
  bciassert( mpInputData != 0 && index < mFFTSize );
#endif
  return static_cast<Real*>( mpInputData )[index];
}

inline
const FFTLibWrapper::Real&
RealFFT::Output( int index ) const
{
#if BCIDEBUG
  bciassert( mpOutputData != 0 && index < mFFTSize );
#endif
  return static_cast<Real*>( mpOutputData )[index];
}

inline
FFTLibWrapper::Complex&
ComplexFFT::Input( int index )
{
#if BCIDEBUG
  bciassert( mpInputData != 0 && index < mFFTSize );
#endif
  return static_cast<Complex*>( mpInputData )[index];
}

inline
const FFTLibWrapper::Complex&
ComplexFFT::Output( int index ) const
{
#if BCIDEBUG
  bciassert( mpOutputData != 0 && index < mFFTSize );
#endif
  return static_cast<Complex*>( mpOutputData )[index];
}

#endif // FFT_LIB_WRAP_H
