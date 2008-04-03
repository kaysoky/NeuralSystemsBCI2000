////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The ARFilter fits a Maximum Entropy AR model to a window
//   of past input data.
//   Its output can be configured to be
//   - raw AR coefficients,
//   - the model's amplitude spectrum,
//   - the model's intensity spectrum.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
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
}


void
ARFilter::Preflight( const SignalProperties& Input,
                           SignalProperties& Output ) const
{
  // Parameter consistency checks.
  float windowLength = MeasurementUnits::ReadAsTime( Parameter( "WindowLength" ) );
  int samplesInWindow = windowLength * Parameter( "SampleBlockSize" );
  if( samplesInWindow < Parameter( "ModelOrder" ) )
    bcierr << "WindowLength parameter must be large enough"
           << " for the number of samples to exceed the model order"
           << endl;

  Output = Input;
  switch( int( Parameter( "OutputType" ) ) )
  {
    case SpectralAmplitude:
    case SpectralPower:
    {
      float firstBinCenter = MeasurementUnits::ReadAsFreq( Parameter( "FirstBinCenter" ) ),
            lastBinCenter = MeasurementUnits::ReadAsFreq( Parameter( "LastBinCenter" ) ),
            binWidth = MeasurementUnits::ReadAsFreq( Parameter( "BinWidth" ) );

      if( firstBinCenter > 0.5 || firstBinCenter < 0
          || lastBinCenter > 0.5 || lastBinCenter < 0 )
        bcierr << "FirstBinCenter and LastBinCenter must be greater zero and"
               << " less than half the sampling rate"
               << endl;
      if( firstBinCenter >= lastBinCenter )
        bcierr << "FirstBinCenter must be less than LastBinCenter" << endl;
      if( binWidth <= 0 )
        bcierr << "BinWidth must be greater zero" << endl;
      else
      {
        int numBins = ::floor( ( lastBinCenter - firstBinCenter + eps ) / binWidth + 1 );
        Output.SetElements( numBins );
      }
      Output.ElementUnit().SetOffset( firstBinCenter / binWidth )
                          .SetGain( binWidth * Parameter( "SamplingRate" ) )
                          .SetSymbol( "Hz" );
      Output.ValueUnit().SetRawMin( 0 );
      float inputAmplitude = Input.ValueUnit().RawMax() - Input.ValueUnit().RawMin(),
            whiteNoisePowerPerBin = inputAmplitude * inputAmplitude / binWidth / 10;
      switch( int( Parameter( "OutputType" ) ) )
      {
        case SpectralAmplitude:
          Output.SetName( "AR Amplitude Spectrum" );
          Output.ValueUnit().SetOffset( 0 )
                            .SetGain( 1e-6 )
                            .SetSymbol( "V/sqrt(Hz)" )
                            .SetRawMax( ::sqrt( whiteNoisePowerPerBin ) );
          break;

        case SpectralPower:
        {
          Output.SetName( "AR Power Spectrum" );
          Output.ValueUnit().SetOffset( 0 )
                            .SetGain( 1 )
                            .SetSymbol( "(muV)^2/Hz" )
                            .SetRawMax( whiteNoisePowerPerBin );
        } break;
      }
    } break;

    case ARCoefficients:
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
}


void
ARFilter::Initialize( const SignalProperties& Input,
                      const SignalProperties& /*Output*/ )
{
  int windowLength = MeasurementUnits::ReadAsTime( Parameter( "WindowLength" ) )
                     * Parameter( "SampleBlockSize" );
  mBuffer.clear();
  mBuffer.resize( Input.Channels(), DataVector( 0.0, windowLength ) );

  mOutputType = Parameter( "OutputType" );
  mDetrend = Parameter( "Detrend" );
  mMEMPredictor.SetModelOrder( Parameter( "ModelOrder" ) );

  switch( mOutputType )
  {
    case SpectralAmplitude:
    case SpectralPower:
    {
      float firstBinCenter = MeasurementUnits::ReadAsFreq( Parameter( "FirstBinCenter" ) ),
            lastBinCenter = MeasurementUnits::ReadAsFreq( Parameter( "LastBinCenter" ) ),
            binWidth = MeasurementUnits::ReadAsFreq( Parameter( "BinWidth" ) );
      int outputElements = ::floor( ( lastBinCenter - firstBinCenter + eps ) / binWidth + 1 );
      mTransferSpectrum
        .SetFirstBinCenter( firstBinCenter )
        .SetBinWidth( binWidth )
        .SetNumBins( outputElements )
        .SetEvaluationsPerBin( Parameter( "EvaluationsPerBin" ) );

    } break;

    case ARCoefficients:
      break;
  }
}


void
ARFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  for( int channel = 0; channel < Input.Channels(); ++channel )
  {
    DataVector& buf = mBuffer[ channel ];
    // Shift buffer contents left:
    size_t i = 0, j = Input.Elements();
    while( j < buf.size() )
      buf[ i++ ] = buf[ j++ ];
    // Fill the rightmost part of the buffer with new input:
    j = Input.Elements() - ( buf.size() - i );
    while( i < buf.size() )
      buf[ i++ ] = Input( channel, j++ );

    const DataVector* inputData = &buf;
    switch( mDetrend )
    {
      case none:
        break;

      case mean:
        inputData = &Detrend::MeanDetrend( buf );
        break;

      case linear:
        inputData = &Detrend::LinearDetrend( buf );
        break;

      default:
        bcierr << "Unknown detrend option" << endl;
    }

    const Ratpoly<Complex>&
      transferFunction = mMEMPredictor.TransferFunction( *inputData );

    switch( mOutputType )
    {
      case SpectralAmplitude:
      {
        const std::valarray<float>& spectrum = mTransferSpectrum.Evaluate( transferFunction );
        for( size_t bin = 0; bin < spectrum.size(); ++bin )
          Output( channel, bin ) = sqrt( spectrum[ bin ] );
      } break;

      case SpectralPower:
      {
        const std::valarray<float>& spectrum = mTransferSpectrum.Evaluate( transferFunction );
        for( size_t bin = 0; bin < spectrum.size(); ++bin )
          Output( channel, bin ) = spectrum[ bin ];
      } break;

      case ARCoefficients:
      {
        const Polynomial<Complex>::Vector& coeff = transferFunction.Denominator().Coefficients();
        for( size_t i = 1; i < coeff.size(); ++i )
          Output( channel, i - 1 ) = coeff[ i ].real();
      } break;

      default:
        bcierr << "Unknown output type" << endl;
    }
  }
}


