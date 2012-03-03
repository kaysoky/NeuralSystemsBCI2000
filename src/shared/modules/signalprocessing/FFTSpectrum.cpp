////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de,
//          Adam Wilson
// Description: The ARSpectrum filter fits a Maximum Entropy AR model to
//   its input data. Its output can be configured to be
//   - raw AR coefficients,
//   - the model's amplitude spectrum,
//   - the model's intensity spectrum.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "FFTSpectrum.h"
#include <limits>

using namespace std;

void
FFTThread::OnPublish() const
{
  SpectrumThread::OnPublish();
}

void
FFTThread::OnPreflight( const SignalProperties& Input,
                              SignalProperties& Output ) const
{
  if( !mFFT.LibAvailable() )
    bcierr << "Could not find the " << mFFT.LibName() << " library."
           << endl;
  SpectrumThread::OnPreflight( Input, Output );
  if( Parameter( "OutputType" ) == Coefficients )
  {
    Output.SetElements( 2 * Output.Elements() );
    Output.ElementUnit() = PhysicalUnit();
    Output.ElementUnit().SetRawMin( 0 )
                        .SetRawMax( Output.Elements() - 1 );
  }
  Output.SetName( "FFT " + Output.Name() );
}

void
FFTThread::OnInitialize( const SignalProperties& Input, const SignalProperties& Output )
{
  mOutputType = Parameter( "OutputType" );
  double binWidth = Parameter( "BinWidth" ).InHertz(),
         firstBinCenter = Parameter( "FirstBinCenter" ).InHertz(),
         lastBinCenter = Parameter( "LastBinCenter" ).InHertz();
  int numBins = Output.Elements();
  mSpectrum.resize( numBins );
  lastBinCenter = numBins * binWidth + firstBinCenter;
  // To determine an optimum FFT size, we proceed along the following path:
  // * To allow for downsampling to user-specified spectral resolution, a single bin should have
  //   a resolution of binWidth/n with integer n.
  // * Input data is resampled to a new sampling rate fftRate = binWidth/n*fftSize with integer
  //   fftSize.
  // * To cover the range from 0 to at least 3/2*lastBinCenter, we need fftRate >= 3*lastBinCenter.
  // * The downsampled input data must fit into fftSize:
  //   Input.Elements() * fftRate / Input.SamplingRate() <= fftSize.
  //
  // Thus, we have
  // (1) n >= Input.Elements() * binWidth / Input.SamplingRate(),
  // (2) fftSize >= 3*lastBinCenter / binWidth * n.
  int n = static_cast<int>( ::ceil( binWidth * Input.Elements() / Input.SamplingRate() ) ),
      fftSize = static_cast<int>( ::ceil( 3 * lastBinCenter / binWidth * n ) );
  mFFT.Initialize( fftSize );
  double fftRate = binWidth * fftSize / n;
  mInputResampling = fftRate / Input.SamplingRate();
  mSpectrumOversampling = n;
  bcidbg << "FFT size: " << fftSize
         << ", Input resampling: " << mInputResampling
         << ", Spectrum oversampling: " << mSpectrumOversampling
         << endl;
  // To match the first bin's center, we shift the spectrum to the left by the 
  // amount given by firstBinCenter.
  // This is achieved by multiplication with a carrier that has a negative 
  // frequency, and zero phase.
  mShiftCarrier.resize( fftSize );
  Real carrierOmega = -firstBinCenter * mInputResampling / 2 / M_PI,
       carrierPhase = -fftSize * carrierOmega / 2;
  for( int i = 0; i < fftSize; ++i )
    mShiftCarrier[i] = polar<Real>( 1, i * carrierOmega + carrierPhase );
  // Normalization is applied to energy, and chosen such that input energy/s matches output energy in all bins.
  // A factor of 2 to account for both positive and negative frequencies.
  mNormalizationFactor = 2;
  // Output is per length of input data in seconds.
  mNormalizationFactor /= ( 1.0 * Input.Elements() / Input.SamplingRate() );
  // Output is per bin width.
  mNormalizationFactor /= binWidth;
  // Resampling multiplies signal energy by the resampling factor.
  mNormalizationFactor /= mInputResampling;
  // An extra factor of 1/fftSize to account for proper FFT normalization.
  mNormalizationFactor /= fftSize;
}

void
FFTThread::OnProcess( const GenericSignal& Input, GenericSignal& Output )
{
  const Real eps = numeric_limits<Real>::epsilon();
  Real indexOffset = ( mFFT.Size() - mInputResampling * Input.Elements() ) / 2;
  for( size_t ch = 0; ch < Channels().size(); ++ch )
  {
    for( int i = 0; i < mFFT.Size(); ++i )
      mFFT.Input( i ) = 0;
    for( int i = 0; i < Input.Elements(); ++i )
    { // Resample input using linear interpolation.
      // First, project left and right boundary of the current input sample onto buffer samples.
      Real left = i * mInputResampling + indexOffset,
           right = left + mInputResampling,
           value = Input( Channels()[ch], i );
      // Integrate linearly interpolated samples between left and right boundary.
      Real intPart, fracPart;
      fracPart = ::modf( left, &intPart );
      int idxLeft = static_cast<int>( intPart );
      mFFT.Input( idxLeft ) -= value * fracPart;
      fracPart = ::modf( right, &intPart );
      int idxRight = static_cast<int>( intPart );
      for( int j = idxLeft; j < idxRight; ++j )
        mFFT.Input( j ) += value;
      mFFT.Input( idxRight ) += value * fracPart;
    }
    for( int i = 0; i < mFFT.Size(); ++i )
      mFFT.Input( i ) *= mShiftCarrier[i];
    mFFT.Compute();
    // Resample the computed spectrum into the bins specified by the user.
    int k = 0;
    switch( mOutputType )
    {
      case SpectralAmplitude:
      case SpectralPower:
        for( size_t i = 0; i < mSpectrum.size(); ++i )
        {
          Real sqMag = 0;
          for( int j = 0; j < mSpectrumOversampling; ++j )
            sqMag += norm( mFFT.Output( k++ ) );
          mSpectrum[i] = sqMag * mNormalizationFactor;
        }
        break;

      case Coefficients:
        for( size_t i = 0; i < mSpectrum.size(); ++i )
        {
          Complex value = 0;
          Real sqMag = 0;
          for( int j = 0; j < mSpectrumOversampling; ++j )
          {
            value += mFFT.Output( k );
            sqMag += norm( mFFT.Output( k++ ) );
          }
          if( ::fabs( value.real() ) > eps || ::fabs( value.imag() ) > eps )
            value /= ::sqrt( norm( value ) );
          value *= ::sqrt( sqMag * mNormalizationFactor );
          mSpectrum[i] = value;
        }
        break;

      default:
        throw bciexception_( "Unhandled output type: " << mOutputType );
    }

    switch( mOutputType )
    {
      case SpectralAmplitude:
        for( size_t i = 0; i < mSpectrum.size(); ++i )
          Output( Channels()[ch], i ) = ::sqrt( mSpectrum[i].real() );
        break;

      case SpectralPower:
        for( size_t i = 0; i < mSpectrum.size(); ++i )
          Output( Channels()[ch], i ) = mSpectrum[i].real();
        break;

      case Coefficients:
        for( size_t i = 0; i < mSpectrum.size(); ++i )
          Output( Channels()[ch], i ) = mSpectrum[i].real();
        for( size_t i = 0; i < mSpectrum.size(); ++i )
          Output( Channels()[ch], i + mSpectrum.size() ) = mSpectrum[i].imag();
        break;

      default:
        throw bciexception_( "Unhandled output type: " << mOutputType );
    }
  }
}

