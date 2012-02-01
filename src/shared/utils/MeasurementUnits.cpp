/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Utilities for handling measurement units and conversions.
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
//////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "MeasurementUnits.h"
#include "BCIError.h"
#include <limits>

using namespace std;

double MeasurementUnits::sSamplingRate = 1.0;
double MeasurementUnits::sSampleBlockSize = 1.0;
PhysicalUnit MeasurementUnits::sTimeUnit;
PhysicalUnit MeasurementUnits::sFreqUnit;
PhysicalUnit MeasurementUnits::sVoltageUnit;

void
MeasurementUnits::Initialize( const ParamList& inParams )
{
  if( inParams.Exists( "SamplingRate" ) )
  {
    sSamplingRate = PhysicalUnit()
                   .SetGain( 1.0 )
                   .SetOffset( 0.0 )
                   .SetSymbol( "Hz" )
                   .PhysicalToRaw( inParams[ "SamplingRate" ].Value() );
    if( sSamplingRate <= 0.0 )
      bcierr << "Parameter SamplingRate needs to be greater zero" << endl;
  }

  if( inParams.Exists( "SampleBlockSize" ) )
  {
    sSampleBlockSize = PhysicalUnit()
                      .SetGain( 1.0 )
                      .SetOffset( 0.0 )
                      .SetSymbol( "" )
                      .PhysicalToRaw( inParams[ "SampleBlockSize" ].Value().c_str() );
    if( sSampleBlockSize < 1 )
      bcierr << "Parameter SampleBlockSize needs to be >= 1" << endl;
  }
  // Set the unit for raw numbers representing time to multiples of sample block duration.
  sTimeUnit.SetOffset( 0 ).SetGain( sSampleBlockSize / sSamplingRate ).SetSymbol( "s" );
  // Set the unit for raw numbers representing frequencies to 1Hz.
  sFreqUnit.SetOffset( 0 ).SetGain( 1.0 ).SetSymbol( "Hz" );
  // Set the unit for raw numbers representing voltages to Microvolts.
  sVoltageUnit.SetOffset( 0 ).SetGain( 1e-6 ).SetSymbol( "V" );
}

#if MEASUREMENT_UNITS_BACK_COMPAT
// These functions are deprecated, as their names are ambiguous:
double 
MeasurementUnits::ReadAsTime( const std::string& value )
{
  bciout << "MeasurementUnits::ReadAsTime() is deprecated. Please call MeasurementUnits::TimeInSampleBlocks() instead."
         << endl;
  return TimeInSampleBlocks( value );
}

double 
MeasurementUnits::ReadAsFreq( const std::string& value )
{
  bciout << "MeasurementUnits::ReadAsFreq() is deprecated. Please use MeasurementUnits::FreqInHertz() / Input.SamplingRate() instead."
         << endl;
  return FreqInHertz( value ) / SamplingRate();
}
#endif // MEASUREMENT_UNITS_BACK_COMPAT
