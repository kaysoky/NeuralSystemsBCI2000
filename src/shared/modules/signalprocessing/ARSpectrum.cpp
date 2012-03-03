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

#include "ARSpectrum.h"
#include "BCIException.h"

using namespace std;

void
ARThread::OnPublish() const
{
  SpectrumThread::OnPublish();
  BEGIN_PARAMETER_DEFINITIONS
   "Filtering:AR%20Spectral%20Estimator int ModelOrder= 16 16 0 % "
      "// AR model order",
   "Filtering:AR%20Spectral%20Estimator int EvaluationsPerBin= 15 15 1 % "
      "// Number of uniformly spaced evaluation points entering into a single bin value",
  END_PARAMETER_DEFINITIONS
 }

void
ARThread::OnPreflight( const SignalProperties& Input,
                             SignalProperties& Output ) const
{
  if( Input.Elements() < Parameter( "ModelOrder" ) )
    bcierr << "WindowLength parameter must be large enough"
           << " for the number of samples to exceed the model order"
           << endl;

  if( Parameter( "OutputType" ) == Coefficients )
  {
    Output = Input;
    Output.SetName( "AR Coefficients" );
    Output.SetElements( Parameter( "ModelOrder" ) );
    Output.ElementUnit().SetSymbol( "" )
                        .SetOffset( 0 )
                        .SetGain( 1 )
                        .SetRawMin( 0 )
                        .SetRawMax( Output.Elements() - 1 );
  }
  else
  {
    SpectrumThread::OnPreflight( Input, Output );
    Output.SetName( "AR " + Output.Name() );
  }
}

void
ARThread::OnInitialize( const SignalProperties& Input, const SignalProperties& Output )
{
  mMEMPredictor.SetModelOrder( Parameter( "ModelOrder" ) );
  mOutputType = Parameter( "OutputType" );
  if( mOutputType != Coefficients )
  {
    mTransferSpectrum.SetFirstBinCenter( Parameter( "FirstBinCenter" ).InHertz() / Input.SamplingRate() );
    mTransferSpectrum.SetBinWidth( Parameter( "BinWidth" ).InHertz() / Input.SamplingRate() );
    mTransferSpectrum.SetNumBins( Output.Elements() );
    mTransferSpectrum.SetEvaluationsPerBin( Parameter( "EvaluationsPerBin" ) );
    mSpectrum.resize( Output.Elements() );
  }
  mInput.resize( Input.Elements() );
}

void
ARThread::OnProcess( const GenericSignal& Input, GenericSignal& Output )
{
  for( size_t ch = 0; ch < Channels().size(); ++ch )
  {
    for( size_t i = 0; i < mInput.size(); ++i )
      mInput[i] = Input( Channels()[ch], i );

    mMEMPredictor.TransferFunction( mInput, mTransferFunction );
    mTransferFunction *= ::sqrt( 2.0 ); // Multiply power by a factor of 2 to account for positive and negative frequencies.
    switch( mOutputType )
    {
      case SpectralAmplitude:
      case SpectralPower:
        mTransferSpectrum.Evaluate( mTransferFunction, mSpectrum );
        for( size_t bin = 0; bin < mSpectrum.size(); ++bin )
          Output( Channels()[ch], bin ) =
            ( mOutputType == SpectralAmplitude ) ? ::sqrt( mSpectrum[bin] ) : mSpectrum[bin];
        break;

      case ARCoefficients:
      {
        const Polynomial<Real>::Vector& coeff = mTransferFunction.Denominator().Coefficients();
        for( size_t i = 1; i < coeff.size(); ++i )
          Output( Channels()[ch], i - 1 ) = coeff[i];
      } break;

      default:
        throw bciexception( "Unknown output type" );
    }
  }
}

