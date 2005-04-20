#include "PCHIncludes.h"
#pragma hdrstop

#include "FFTFilter.h"
#include "defines.h"
#include <vector>
#include <map>
#include <assert>
#include <math.h>

#define SECTION "Filtering"

using namespace std;

RegisterFilter( FFTFilter, 2.B1 ); // We want it after the spatial filter.

FFTFilter::FFTFilter()
: mVisualizeFFT( 0 ),
  mFFTOutputSignal( eInput ),
  mFFTSize( 0 ),
  mFFTWindow( eNone )
{
 BEGIN_PARAMETER_DEFINITIONS
   SECTION " int FFTOutputSignal= 0"
     " 0 0 2 // Signal provided to the next filter: "
              "0: Input Signal, "
              "1: Power Spectra of selected Channels "
              "2: Complex Amplitudes of selected Channels "
              "(enumeration)",
   SECTION " intlist FFTInputChannels= 1 1"
     " 0 0 0 // Input Channels the FFT is performed on",
   SECTION " int FFTSize= 256"
     " 0 0 0 // Number of Samples over which the FFT is computed",
   SECTION " int FFTWindow= 3"
     " 0 0 3 // Type of Sidelobe Suppression Window "
              "1: Hamming, "
              "2: Hann, "
              "3: Blackman "
              "(enumeration)",
   "Visualize int VisualizeFFT= 0"
     " 0 0 1 // Visualize FFT Power Spectra (boolean)",
   "Visualize float FFTMaxValue= 4000"
     " 4000 0 0 // FFT visualization max value",
 END_PARAMETER_DEFINITIONS
}

FFTFilter::~FFTFilter()
{
}

void
FFTFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  for( size_t i = 0; i < Parameter( "FFTInputChannels" )->GetNumValues(); ++i )
    PreflightCondition( Parameter( "FFTInputChannels", i ) > 0
                        && Parameter( "FFTInputChannels", i ) <= Input.Channels() );

  PreflightCondition( Parameter( "FFTInputChannels" )->GetNumValues() == 0 || Parameter( "FFTSize" ) > 0 );
  if( ( int )Parameter( "VisualizeFFT" ) && ( ( int )Parameter( "FFTSize" ) / 2 + 1 > 255 ) )
    bcierr << "FFTSize must be less than 510 because signals with more than"
           << " 255 channels cannot be sent to the operator"
           << endl;

  if( ( int )Parameter( "VisualizeFFT" ) )
    PreflightCondition( Parameter( "FFTMaxValue" ) > 0 );

  bool fftRequired = ( ( int )Parameter( "FFTOutputSignal" ) != eInput
                       || ( int )Parameter( "VisualizeFFT" ) )
                     && ( Parameter( "FFTInputChannels" )->GetNumValues() > 0 );
  if( fftRequired )
  {
    if( mFFT.LibAvailable() )
    {
      FFTLibWrapper preflightFFT;
      if( !preflightFFT.Initialize( Parameter( "FFTSize" ) ) )
        bcierr << "Requested parameters are not supported by FFT library" << endl;
    }
    else
      bcierr << "The FFT Filter could not find the " << mFFT.LibName() << " library. "
             << "For legal reasons, this library is not part of the BCI2000 distribution. "
             << "For information on how to obtain it, see the file README_FFT.txt inside "
             << "the documents folder. We apologize for this inconvenience."
             << endl;
  }

  switch( ( int )Parameter( "FFTOutputSignal" ) )
  {
    case eInput:
      Output = Input;
      break;
    case ePower:
      Output = SignalProperties( Parameter( "FFTInputChannels" )->GetNumValues(),
                                    ( int )Parameter( "FFTSize" ) / 2 + 1, Input.Type() );
      break;
    case eHalfcomplex:
      Output = SignalProperties( Parameter( "FFTInputChannels" )->GetNumValues(),
                                    ( int )Parameter( "FFTSize" ), Input.Type() );
      break;
    default:
      assert( false );
  }
}

void
FFTFilter::Initialize()
{
  mVisualizeFFT = ( int )Parameter( "VisualizeFFT" );
  mFFTOutputSignal = ( eFFTOutputSignal )( int )Parameter( "FFTOutputSignal" );
  mFFTSize = Parameter( "FFTSize" );
  mFFTWindow = ( eFFTWindow )( int )Parameter( "FFTWindow" );

  mFFTInputChannels.clear();
  mSpectra.clear();
  mVisualizations.clear();
  for( size_t i = 0; i < Parameter( "FFTInputChannels" )->GetNumValues(); ++i )
  {
    mFFTInputChannels.push_back( Parameter( "FFTInputChannels", i ) + 1 );
    mSpectra.push_back( GenericSignal( mFFTSize / 2 + 1, 1 ) );
    if( mVisualizeFFT )
    {
      mVisualizations.push_back( GenericVisualization( SOURCEID::FFT + i, VISTYPE::GRAPH ) );
      ostringstream oss;
      oss << "FFT for Ch ";
      if( Parameter( "FFTInputChannels" )->Labels().IsTrivial() )
        oss << i + 1;
      else
        oss << Parameter( "FFTInputChannels" )->Labels()[ i ];
      mVisualizations.back().Send( CFGID::WINDOWTITLE, oss.str().c_str() );
      mVisualizations.back().Send( CFGID::graphType, CFGID::field2d );
      mVisualizations.back().Send( CFGID::MINVALUE, 0 );
      mVisualizations.back().Send( CFGID::MAXVALUE, ( float )Parameter( "FFTMaxValue" ) );
    }
  }

  mWindow.clear();
  mWindow.resize( mFFTSize, 1.0 );
  float phasePerSample = M_PI / float( mFFTSize );
  // Window coefficients: None Hamming Hann Blackman
  const float a1[] = {       0,   0.46, 0.5,     0.5, },
              a2[] = {       0,   0,    0,       0.08 };

  for( int i = 0; i < mFFTSize; ++i )
    mWindow[ i ] = 1.0 - a1[ mFFTWindow ] - a2[ mFFTWindow ]
                   + a1[ mFFTWindow ] * cos( float( i ) * phasePerSample )
                   + a2[ mFFTWindow ] * cos( float( i ) * 2 * phasePerSample );

  mValueBuffers.resize( mFFTInputChannels.size() );
  ResetValueBuffers( mFFTSize );

  bool fftRequired = ( mVisualizeFFT || mFFTOutputSignal != eInput)
                     && mFFTInputChannels.size() > 0;
  if( !fftRequired )
  {
    mFFTInputChannels.clear();
    mSpectra.clear();
  }
  else
    mFFT.Initialize( mFFTSize );
}

void
FFTFilter::Process( const GenericSignal* inputSignal, GenericSignal* outputSignal )
{
  for( size_t i = 0; i < mFFTInputChannels.size(); ++i )
  {
    // Copy the input signal values to the value buffer.
    vector<float>& buffer = mValueBuffers[ i ];
    int inputSize = inputSignal->Elements(),
        bufferSize = buffer.size();
    // Move old values towards the beginning of the buffer, if any.
    for( int j = 0; j < bufferSize - inputSize; ++j )
      buffer[ j ] = buffer[ j + inputSize ];
    // Copy new values to the end of the buffer;
    // the buffer size may be greater of smaller than the input size.
    for( int j = ::max( 0, bufferSize - inputSize ); j < bufferSize; ++j )
      buffer[ j ] = ( *inputSignal )( i, j + inputSize - bufferSize );
    // Prepare the buffer.
    if( mFFTWindow == eNone )
      for( int j = 0; j < bufferSize; ++j )
        mFFT.Input( j ) = buffer[ j ];
    else
      for( int j = 0; j < bufferSize; ++j )
        mFFT.Input( j ) = buffer[ j ] * mWindow[ j ];
    // Compute the power spectrum and visualize it if requested.
    mFFT.Transform();
    GenericSignal& spectrum = mSpectra[ i ];
    if( mVisualizeFFT || mFFTOutputSignal == ePower )
    {
      float normFactor = 1.0 / bufferSize;
      spectrum( 0, 0 ) = mFFT.Output( 0 ) * mFFT.Output( 0 ) * normFactor;
      for( int k = 1; k < ( bufferSize + 1 ) / 2; ++k )
        spectrum( k, 0 ) = (
          mFFT.Output( k ) * mFFT.Output( k ) +
          mFFT.Output( bufferSize - k ) * mFFT.Output( bufferSize - k ) ) * normFactor;
      if( bufferSize % 2 == 0 )
        spectrum( bufferSize / 2, 0 ) = mFFT.Output( bufferSize / 2 ) * mFFT.Output( bufferSize / 2 ) * normFactor;
    }
    else if( mFFTOutputSignal == eHalfcomplex )
    {
      float normFactor = 1.0 / ::sqrt( bufferSize );
      for( int k = 0; k < bufferSize; ++k )
        spectrum( k, 0 ) = mFFT.Output( k ) * normFactor;
    }
    if( mVisualizeFFT )
      mVisualizations[ i ].Send( &spectrum );
  }
  switch( mFFTOutputSignal )
  {
    case eInput:
      *outputSignal = *inputSignal;
      break;
    case ePower:
    case eHalfcomplex:
      for( size_t channel = 0; channel < mSpectra.size(); ++channel )
        for( size_t element = 0; element < mSpectra[ channel ].Channels(); ++element )
          ( *outputSignal )( channel, element ) = mSpectra[ channel ]( element, 0 );
      break;
    default:
      assert( false );
  }
}

void
FFTFilter::Resting()
{
  ResetValueBuffers( mFFTSize );
}


void
FFTFilter::ResetValueBuffers( size_t inSize )
{
  // Clear all the value buffers so old data won't enter into the transform
  // on the next call to Process().
  for( size_t i = 0; i < mValueBuffers.size(); ++i )
  {
    mValueBuffers[ i ].clear();
    mValueBuffers[ i ].resize( inSize, 0 );
  }
}



