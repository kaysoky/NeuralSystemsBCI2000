////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: A notch filter for removing power line noise, and a high pass
//   collected into a single filter.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SourceFilter.h"
#include "FilterDesign.h"
#include "MeasurementUnits.h"

using namespace std;

RegisterFilter( SourceFilter, 1.01 );

SourceFilter::SourceFilter()
{
  BEGIN_PARAMETER_DEFINITIONS
    "Source:Source%20Filter int NotchFilter= 0 0 0 2 "
      "// Power line notch filter: 0: disabled, 1: at 50Hz, 2: at 60Hz (enumeration)",
    "Source:Source%20Filter int HighPassFilter= 0 0 0 1 "
      "// Source high pass filter: 0: disabled, 1: at 0.1Hz (enumeration)",
  END_PARAMETER_DEFINITIONS
}

void
SourceFilter::DesignFilter( const SignalProperties& inSignalProperties,
                            Real& outGain,
                            ComplexVector& outZeros,
                            ComplexVector& outPoles ) const
{
  Real samplingRate = MeasurementUnits::ReadAsTime( "1s" ) * inSignalProperties.Elements();

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
  outZeros = tf.Numerator().Roots();
  outPoles = tf.Denominator().Roots();
}

