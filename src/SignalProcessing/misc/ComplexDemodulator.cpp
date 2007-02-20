////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        ComplexDemodulator.cpp
// Date:        Jan 2, 2006
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that computes the squared amplitudes for a small number
//              of bands.
//              Its operation is roughly equivalent to a short-term fourier
//              transform followed by demodulation for selected frequency bins.
// $Log$
// Revision 1.1  2006/01/13 15:04:46  mellinger
// Initial version.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "ComplexDemodulator.h"

#include "MeasurementUnits.h"
#include <numeric>

using namespace std;

RegisterFilter( ComplexDemodulator, 2.F ); // Place it last in signal processing.

ComplexDemodulator::ComplexDemodulator()
{
 BEGIN_PARAMETER_DEFINITIONS
  "ComplexDemodulator list DemodulatorFrequencies= 3 625/6Hz 625/5Hz 625/4Hz % % % "
    "// frequencies at which to perform demodulation (corresponds to output signal elements)",
  "ComplexDemodulator int FrequencyResolution= 10Hz % % % "
    "// width of frequency bands",
 END_PARAMETER_DEFINITIONS
}


ComplexDemodulator::~ComplexDemodulator()
{
}


void
ComplexDemodulator::Preflight( const SignalProperties& input,
                                     SignalProperties& output ) const
{
  ParamRef DemodulatorFrequencies = Parameter( "DemodulatorFrequencies" );
  bool error = false;
  for( size_t i = 0; i < DemodulatorFrequencies->GetNumValues(); ++i )
  {
    double frequency = MeasurementUnits::ReadAsFreq( DemodulatorFrequencies( i ) );
    error |= ( frequency < 0 );
    error |= ( frequency > 0.5 );
  }
  if( error )
    bcierr << "DemodulatorFrequencies must be greater 0 and less than half the "
           << "sampling rate "
           << "(currently " << Parameter( "SamplingRate" ) << "Hz)"
           << endl;
  double FrequencyResolution
    = MeasurementUnits::ReadAsFreq( Parameter( "FrequencyResolution" ) );
  PreflightCondition( FrequencyResolution > 0 );

  // Request output signal properties:
  output = SignalProperties(
            input.Channels(),
            DemodulatorFrequencies->GetNumValues(),
            SignalType::float32
           );
}


void
ComplexDemodulator::Initialize2( const SignalProperties& input,
                                 const SignalProperties& output )
{
  float FrequencyResolution = MeasurementUnits::ReadAsFreq( Parameter( "FrequencyResolution" ) );
  size_t numCoefficients = ::floor( 1.0 / FrequencyResolution ) + 1;
  ParamRef DemodulatorFrequencies = Parameter( "DemodulatorFrequencies" );
  mCoefficients.clear();
  mCoefficients.resize(
    DemodulatorFrequencies->GetNumValues(),
    vector<complex<double> >( numCoefficients )
  );
  for( size_t i = 0; i < DemodulatorFrequencies->GetNumValues(); ++i )
  {
    double freq = MeasurementUnits::ReadAsFreq( DemodulatorFrequencies( i ) );
    for( size_t j = 0; j < numCoefficients; ++j )
      mCoefficients[ i ][ j ] = polar( 0.5 / numCoefficients, freq * j * 2.0 * M_PI );
  }
  mSignalBuffer.clear();
  mSignalBuffer.resize( input.Channels(), vector<double>( numCoefficients ) );
}


void
ComplexDemodulator::Process( const GenericSignal* input, GenericSignal* output )
{
  for( size_t channel = 0; channel < input->Channels(); ++channel )
  {
    for( size_t sample = 0; sample < input->Elements(); ++sample )
    {
      mSignalBuffer[ channel ].push_back( ( *input )( channel, sample ) );
      mSignalBuffer[ channel ].erase( mSignalBuffer[ channel ].begin() );
    }
    for( size_t band = 0; band < mCoefficients.size(); ++band )
    {
      ( *output )( channel, band )
        = norm( inner_product(
            mSignalBuffer[ channel ].begin(),
            mSignalBuffer[ channel ].end(),
            mCoefficients[ band ].begin(),
            complex<double>( 0.0, 0.0 )
          ) );
    }
  }
}


