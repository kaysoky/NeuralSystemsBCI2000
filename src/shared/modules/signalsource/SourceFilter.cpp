////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: A notch filter for removing power line noise, and a high pass
//   collected into a single filter.
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

#include "SourceFilter.h"
#include "FilterDesign.h"

using namespace std;

RegisterFilter( SourceFilter, 1.01 );

SourceFilter::SourceFilter()
{
  BEGIN_PARAMETER_DEFINITIONS
    "Source:Source%20Filter int NotchFilter= 0 0 0 2 "
      "// Power line notch filter: 0: disabled, 1: at 50Hz, 2: at 60Hz (enumeration)",
    "Source:Source%20Filter int HighPassFilter= 0 0 0 2 "
      "// Source high pass filter: 0: disabled, 1: at 0.1Hz, 2: at 1Hz (enumeration)",
    "Source:Source%20Filter int LowPassFilter= 0 0 0 4 "
      "// Source low pass filter: 0: disabled, 1: at 9Hz, 2: at 30Hz, 3: at 40Hz, 4: at 70Hz (enumeration)",
  END_PARAMETER_DEFINITIONS
}

void
SourceFilter::DesignFilter( const SignalProperties& inSignalProperties,
                            Real& outGain,
                            ComplexVector& outZeros,
                            ComplexVector& outPoles ) const
{
  Real samplingRate = inSignalProperties.SamplingRate();

  outGain = 1.0;
  outZeros.clear();
  outPoles.clear();

  typedef Ratpoly<FilterDesign::Complex> TransferFunction;
  TransferFunction tf( 1.0 );

  { // Configure notch filter
    enum
    {
      disabled = 0,
      at50Hz = 1,
      at60Hz = 2,
    };
    int notchFilter = Parameter( "NotchFilter" );
    Real corner1 = 0.0,
         corner2 = 0.0;

    switch( notchFilter )
    {
      case disabled:
        break;

      case at50Hz:
        corner1 = 45.0;
        corner2 = 55.0;
        break;

      case at60Hz:
        corner1 = 55.0;
        corner2 = 65.0;
        break;

      default:
        bcierr << "Unknown value of NotchFilter parameter" << endl;
    }

    if( notchFilter != disabled )
    {
      corner1 /= samplingRate;
      corner2 /= samplingRate;
      if( corner1 >= 0.5 || corner2 >= 0.5 )
      {
        bciout << "Power line frequency is outside sampling bandwidth. "
               << "No filtering will be performed"
               << endl;
      }
      else
      {
       TransferFunction notch =
          FilterDesign::Chebyshev()
          .Ripple_dB( -0.1 )
          .Order( 3 )
          .Bandstop( corner1, corner2 )
          .TransferFunction();
        tf *= notch;
        outGain /= abs( notch.Evaluate( 1.0 ) ); // LF gain
      }
    }
  }

  { // Configure HP filter
    enum
    {
      disabled = 0,
      at01Hz,
      at1Hz,
    };
    int highPassFilter = Parameter( "HighPassFilter" );
    Real corner = 0.0;
    switch( highPassFilter )
    {
      case disabled:
        break;

      case at01Hz:
        corner = 0.1;
        break;

      case at1Hz:
        corner = 1.0;
        break;

      default:
        bcierr << "Unknown value of HighPassFilter parameter" << endl;
    }
    if( highPassFilter != disabled )
    {
      corner /= samplingRate;
      if( corner >= 0.5 )
      {
        bciout << "High pass corner frequency is outside sampling bandwidth. "
               << "No filtering will be performed"
               << endl;
      }
      else
      {
        TransferFunction hp =
          FilterDesign::Butterworth()
          .Order( 1 )
          .Highpass( corner )
          .TransferFunction();
        tf *= hp;
        outGain /= abs( hp.Evaluate( -1.0 ) ); // HF gain
      }
    }
  }

  { // Configure LP filter
    enum
    {
      disabled = 0,
      at9Hz,
      at30Hz,
      at40Hz,
      at70Hz,
    };
    int lowPassFilter = Parameter( "LowPassFilter" );
    Real corner = 0.0;
    switch( lowPassFilter )
    {
      case disabled:
        break;

      case at9Hz:
        corner = 9.0;
        break;

      case at30Hz:
        corner = 30.0;
        break;

      case at40Hz:
        corner = 40.0;
        break;

      case at70Hz:
        corner = 70.0;
        break;

      default:
        bcierr << "Unknown value of LowPassFilter parameter" << endl;
    }
    if( lowPassFilter != disabled )
    {
      corner /= samplingRate;
      if( corner >= 0.5 )
      {
        bciout << "Low pass corner frequency is outside sampling bandwidth. "
               << "No filtering will be performed"
               << endl;
      }
      else
      {
        TransferFunction lp =
          FilterDesign::Butterworth()
          .Order( 2 )
          .Lowpass( corner )
          .TransferFunction();
        tf *= lp;
        outGain /= abs( lp.Evaluate( 1.0 ) ); // LF gain
      }
    }
  }
  outZeros = tf.Numerator().Roots();
  outPoles = tf.Denominator().Roots();
}

