////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that computes the squared amplitudes for a small number
//              of bands.
//              Its operation is roughly equivalent to a short-term fourier
//              transform followed by demodulation for selected frequency bins.
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
ComplexDemodulator::Preflight( const SignalProperties& Input,
                                     SignalProperties& Output ) const
{
  ParamRef DemodulatorFrequencies = Parameter( "DemodulatorFrequencies" );
  bool error = false;
  for( int i = 0; i < DemodulatorFrequencies->NumValues(); ++i )
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
  Output = SignalProperties(
            Input.Channels(),
            DemodulatorFrequencies->NumValues(),
            SignalType::float32
           );
}


void
ComplexDemodulator::Initialize( const SignalProperties& Input,
                                const SignalProperties& Output )
{
  double FrequencyResolution = MeasurementUnits::ReadAsFreq( Parameter( "FrequencyResolution" ) );
  size_t numCoefficients = static_cast<size_t>( ::floor( 1.0 / FrequencyResolution ) + 1 );
  ParamRef DemodulatorFrequencies = Parameter( "DemodulatorFrequencies" );
  mCoefficients.clear();
  mCoefficients.resize(
    DemodulatorFrequencies->NumValues(),
    vector<complex<double> >( numCoefficients )
  );
  for( int i = 0; i < DemodulatorFrequencies->NumValues(); ++i )
  {
    double freq = MeasurementUnits::ReadAsFreq( DemodulatorFrequencies( i ) );
    for( size_t j = 0; j < numCoefficients; ++j )
      mCoefficients[ i ][ j ] = polar( 0.5 / numCoefficients, freq * j * 2.0 * M_PI );
  }
  mSignalBuffer.clear();
  mSignalBuffer.resize( Input.Channels(), vector<double>( numCoefficients ) );
}


void
ComplexDemodulator::Process( const GenericSignal& Input, GenericSignal& Output )
{
  for( int channel = 0; channel < Input.Channels(); ++channel )
  {
    for( int sample = 0; sample < Input.Elements(); ++sample )
    {
      mSignalBuffer[ channel ].push_back( Input( channel, sample ) );
      mSignalBuffer[ channel ].erase( mSignalBuffer[ channel ].begin() );
    }
    for( size_t band = 0; band < mCoefficients.size(); ++band )
    {
      Output( channel, band )
        = norm( inner_product(
            mSignalBuffer[ channel ].begin(),
            mSignalBuffer[ channel ].end(),
            mCoefficients[ band ].begin(),
            complex<double>( 0.0, 0.0 )
          ) );
    }
  }
}


