////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An interface layer to a FFT DLL that allows for user
//        installation of the GPL protected FFTW library. Its sole purpose
//        is to allow using FFTW from a non-GPLed project by requiring
//        user interaction and avoiding the use of copyrighted FFTW code.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef FFT_LIB_WRAP_H
#define FFT_LIB_WRAP_H

#ifndef __GNUC__
# define USE_DLL 1
#endif

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
  if( mInputData == 0 || index > mFFTSize )
    throw "FFTLibWrapper::Input: Index out of bounds";
#endif
  return mInputData[ index ];
}

inline
const FFTLibWrapper::number&
FFTLibWrapper::Output( int index ) const
{
#if !defined( _NDEBUG ) || defined( _DEBUG )
  if( mOutputData == 0 || index > mFFTSize )
    throw "FFTLibWrapper::Output: Index out of bounds";
#endif
  return mOutputData[ index ];
}

#endif // FFT_LIB_WRAP_H
