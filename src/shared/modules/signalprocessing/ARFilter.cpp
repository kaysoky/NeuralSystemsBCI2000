////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de,
//          Adam Wilson
// Description: The ARFilter fits a Maximum Entropy AR model to a window
//   of past input data.
//   Its output can be configured to be
//   - raw AR coefficients,
//   - the model's amplitude spectrum,
//   - the model's intensity spectrum.
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

#include "ARFilter.h"
#include "Detrend.h"
#include "MeasurementUnits.h"
#include <limits>

using namespace std;

const float eps = numeric_limits<float>::epsilon();

RegisterFilter( ARFilter, 2.C );

ARFilter::ARFilter()
: mpAR(NULL)
{
 BEGIN_PARAMETER_DEFINITIONS
  "Filtering int WindowLength= 0.5s 0.5s % % "
      "// Time window for spectrum computation",
  "Filtering int Detrend= 0 0 0 2 "
      "// Detrend data? 0: no, 1: mean, 2: linear (enumeration)",
  "Filtering int ModelOrder= 16 16 0 % "
      "// AR model order",
  "Filtering int OutputType= 0 0 0 2 "
      "// 0: Spectral Amplitude,"
        " 1: Spectral Power,"
        " 2: AR Coefficients"
        " (enumeration)",

  "Filtering float FirstBinCenter= 0Hz 0Hz % % "
      "// Center of first frequency bin (as a fraction of sampling rate or in Hz)",
  "Filtering float LastBinCenter= 30Hz 30Hz % % "
      "// Center of last frequency bin (as a fraction of sampling rate or in Hz)",
  "Filtering float BinWidth= 3Hz 3Hz % % "
      "// Width of spectral bins (as a fraction of sampling rate or in Hz)",
  "Filtering int EvaluationsPerBin= 15 15 1 % "
      "// Number of uniformly spaced evaluation points entering into a single bin value",
 END_PARAMETER_DEFINITIONS
 }


ARFilter::~ARFilter()
{
  Halt();
}


void
ARFilter::Preflight( const SignalProperties& Input,
                           SignalProperties& Output ) const
{
  // Parameter consistency checks.
  double windowLength = Parameter( "WindowLength" ).InBlocks(),
         samplesInWindow = windowLength * Input.Elements();
  if( samplesInWindow < Parameter( "ModelOrder" ) )
    bcierr << "WindowLength parameter must be large enough"
           << " for the number of samples to exceed the model order"
           << endl;

  Output = Input;
  switch( int( Parameter( "OutputType" ) ) )
  {
    case ARparms::SpectralAmplitude:
    case ARparms::SpectralPower:
    {
      double firstBinCenter = Parameter( "FirstBinCenter" ).AsRelativeFreq( Input ),
             lastBinCenter = Parameter( "LastBinCenter" ).AsRelativeFreq( Input ),
             binWidth = Parameter( "BinWidth" ).AsRelativeFreq( Input );

      if( firstBinCenter > 0.5 || firstBinCenter < 0
         || lastBinCenter > 0.5 || lastBinCenter < 0 )
        bcierr << "FirstBinCenter and LastBinCenter must be greater zero and"
               << " less than half the sampling rate"
               << endl;
      if( firstBinCenter > lastBinCenter )
        bcierr << "FirstBinCenter cannot be greater than LastBinCenter" << endl;
      if( binWidth <= 0 )
        bcierr << "BinWidth must be greater zero" << endl;
      else
      {
        int numBins = static_cast<int>( ::floor( ( lastBinCenter - firstBinCenter + eps ) / binWidth + 1 ) );
        Output.SetElements( numBins );
      }
      Output.ElementUnit().SetOffset( -firstBinCenter / binWidth )
                          .SetGain( Parameter( "BinWidth" ).InHertz() )
                          .SetSymbol( "Hz" );
      Output.ValueUnit().SetRawMin( 0 );
      double inputAmplitude = Input.ValueUnit().RawMax() - Input.ValueUnit().RawMin(),
             whiteNoisePowerPerBin = inputAmplitude * inputAmplitude / binWidth / 10;
      switch( int( Parameter( "OutputType" ) ) )
      {
        case ARparms::SpectralAmplitude:
          Output.SetName( "AR Amplitude Spectrum" );
          Output.ValueUnit().SetOffset( 0 )
                            .SetGain( 1e-6 )
                            .SetSymbol( "V/sqrt(Hz)" )
                            .SetRawMax( ::sqrt( whiteNoisePowerPerBin ) );
          break;

        case ARparms::SpectralPower:
          Output.SetName( "AR Power Spectrum" );
          Output.ValueUnit().SetOffset( 0 )
                            .SetGain( 1 )
                            .SetSymbol( "(muV)^2/Hz" )
                            .SetRawMax( whiteNoisePowerPerBin );
          break;
      }
    } break;

    case ARparms::ARCoefficients:
      Output.SetName( "AR Coefficients" );
      Output.SetElements( Parameter( "ModelOrder" ) );
      Output.ElementUnit().SetOffset( 0 )
                          .SetGain( 1 )
                          .SetSymbol( "" );
      Output.ValueUnit().SetOffset( 0 )
                        .SetGain( 1 )
                        .SetSymbol( "" )
                        .SetRawMin( -1 )
                        .SetRawMax( 1 );
      break;

    default:
      bcierr << "Unknown OutputType" << endl;
  }
  Output.ElementUnit().SetRawMin( 0 )
                      .SetRawMax( Output.Elements() - 1 );

  Parameter( "FirstBinCenter" );
  Parameter( "LastBinCenter" );
  Parameter( "BinWidth" );
}


void
ARFilter::Initialize( const SignalProperties& Input,
                      const SignalProperties& /*Output*/ )
{
  ARparms parms;
  parms.binWidth = Parameter( "BinWidth" ).AsRelativeFreq( Input );
  parms.detrend = Parameter( "Detrend" );
  parms.evalsPerBin = Parameter( "EvaluationsPerBin" );
  parms.firstBinCenter = Parameter( "FirstBinCenter" ).AsRelativeFreq( Input );
  parms.lastBinCenter = Parameter( "LastBinCenter" ).AsRelativeFreq( Input );
  parms.modelOrder = Parameter( "ModelOrder" );
  parms.SBS = Input.Elements();
  parms.outputType = Parameter( "OutputType" );
  parms.numWindows = Parameter( "WindowLength" ).InBlocks();

  delete mpAR;
  mpAR = new ARGroup;
  mpAR->Init(Input.Channels(), parms);
  mpAR->setDoThreaded(true);
}


void
ARFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  mpAR->Calculate(&Input, &Output);
}


void
ARFilter::Halt()
{
  delete mpAR;
  mpAR = NULL;
}

