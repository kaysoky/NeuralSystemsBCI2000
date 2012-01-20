////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de,
//          Adam Wilson
// Description: A class that encapsulates a single thread that computes
//   spectra for a subset of input channels, and writes them into an output
//   signal. Multiple threads may operate on the same input and output signal
//   concurrently, provided that their channel sets do not overlap.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "ARThread.h"
#include "Detrend.h"
#include "BCIException.h"
#include "BCIAssert.h"

using namespace std;

ARThread::ARThread()
: mpInput( NULL ),
  mpOutput( NULL ),
  mWindowLength( 0 ),
  mDetrend( 0 ),
  mOutputType( 0 )
{
}

ARThread&
ARThread::Start( const GenericSignal& Input, GenericSignal& Output )
{
  mpInput = &Input;
  mpOutput = &Output;
  if( !ReusableThread::Run( *this ) )
    throw bciexception( "Could not start execution: thread busy" );
  return *this;
}

ARThread&
ARThread::Modified()
{
  mInputBuffers.clear();
  mInputBuffers.resize( mChannels.size(), DataVector( mWindowLength ) );
  return *this;
}

void
ARThread::OnRun()
{
  bciassert( mpInput != 0 && mpOutput != 0 );
  for( size_t ch = 0; ch < mChannels.size(); ++ch )
  {
    size_t i = 0,
           j = mpInput->Elements();
    while( j < mInputBuffers[ch].size() )
      mInputBuffers[ch][i++] = mInputBuffers[ch][j++];
    // Fill the rightmost part of the buffer with new input:
    j = mpInput->Elements() - ( mInputBuffers[ch].size() - i );
    while( i < mInputBuffers[ch].size() )
      mInputBuffers[ch][i++] = ( *mpInput )( mChannels[ch], j++ );

    DataVector* pMEMData = NULL;
    switch( mDetrend )
    {
      case none:
        pMEMData = &mInputBuffers[ch];
        break;

      case mean:
        Detrend::MeanDetrend( mInputBuffers[ch], mDetrendBuffer );
        pMEMData = &mDetrendBuffer;
        break;

      case linear:
        Detrend::LinearDetrend( mInputBuffers[ch], mDetrendBuffer );
        pMEMData = &mDetrendBuffer;
        break;

      default:
        throw bciexception( "Unknown detrend option" );
    }

    mMEMPredictor.TransferFunction( *pMEMData, mTransferFunction );
    switch( mOutputType )
    {
      case SpectralAmplitude:
      case SpectralPower:
        mTransferSpectrum.Evaluate( mTransferFunction, mSpectrum );
        for( size_t bin = 0; bin < mSpectrum.size(); ++bin )
          ( *mpOutput )( mChannels[ch], bin ) =
            ( mOutputType == SpectralAmplitude ) ? ::sqrt( mSpectrum[bin] ) : mSpectrum[bin];
        break;

      case ARCoefficients:
      {
        const Polynomial<Real>::Vector& coeff = mTransferFunction.Denominator().Coefficients();
        for( size_t i = 1; i < coeff.size(); ++i )
          ( *mpOutput )( mChannels[ch], i - 1 ) = coeff[i];
      } break;

      default:
        throw bciexception( "Unknown output type" );
    }
  }
}


