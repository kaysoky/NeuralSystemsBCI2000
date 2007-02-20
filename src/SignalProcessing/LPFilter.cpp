////////////////////////////////////////////////////////////////////////////////
//
//  File:        LPFilter.cpp
//
//  Description: A (working) tutorial low pass filter demonstrating
//               parameter access, visualization, and unit conversion.
//
//  Date:        May 27, 2005
//  Author:      juergen.mellinger@uni-tuebingen.de
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "LPFilter.h"

#include "MeasurementUnits.h"
#include "UBCIError.h"
#include "defines.h"
#include <vector>
#include <cmath>
#include <limits>
#include <sstream>

using namespace std;

RegisterFilter( LPFilter, 2.B1 );

LPFilter::LPFilter()
: mDecayFactor( 0 ),
  mPreviousOutput( 0 ),
  mSignalVis( SOURCEID::LowPass )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Filtering float LPTimeConstant= 0"
      " 16s 0 0 // time constant for the high pass filter in blocks or seconds",
    "Visualize int VisualizeLowPass= 0"
      " 1 0 1 // visualize low pass output signal (boolean)",
    "Visualize int LPVisMin= -40 0 0 0 "
      "// low pass visualization min value",
    "Visualize int LPVisMax= 40 0 0 0 "
      "// low pass visualization max value",
  END_PARAMETER_DEFINITIONS
}


LPFilter::~LPFilter()
{
}


void
LPFilter::Preflight( const SignalProperties& inputProperties,
                           SignalProperties& outputProperties ) const
{
  float LPTimeConstant = MeasurementUnits::ReadAsTime( Parameter( "LPTimeConstant" ) );
  LPTimeConstant *= inputProperties.Elements();
  // The PreflightCondition macro will automatically generate an error
  // message if its argument evaluates to false.
  // However, we need to make sure that its argument is user-readable
  // -- this is why we chose a variable name that matches the parameter
  // name.
  PreflightCondition( LPTimeConstant >= 0 );
  // Alternatively, we might write:
  if( LPTimeConstant < 0 )
    bcierr << "The LPTimeConstant parameter must be 0 or greater" << endl;

  // Request output signal properties:
  outputProperties = inputProperties;
}


void
LPFilter::Initialize2( const SignalProperties& inputProperties,
                       const SignalProperties& outputProperties )
{
  // Get the time constant in units of a sample block's duration:
  float timeConstant = MeasurementUnits::ReadAsTime( Parameter( "LPTimeConstant" ) );
  // Convert it into units of a sample's duration:
  timeConstant *= inputProperties.Elements();

  if( timeConstant < numeric_limits<float>::epsilon() )
    mDecayFactor = 0.0;
  else
    mDecayFactor = ::exp( -1.0 / timeConstant );

  // This will initialize elements with 0,
  // implementing the first line of the filter prescription:
  mPreviousOutput.clear();
  mPreviousOutput.resize( inputProperties.Channels(), 0 );

  // Determine a sample's duration in seconds:
  float sampleDuration = Parameter( "SampleBlockSize" ) /
                       Parameter( "SamplingRate" ) / inputProperties.Elements();

  mSignalVis.Send( CFGID::WINDOWTITLE, "Low Pass" );
  mSignalVis.Send( CFGID::graphType, CFGID::polyline );
  mSignalVis.Send( CFGID::MINVALUE, Parameter( "LPVisMin" ) );
  mSignalVis.Send( CFGID::MAXVALUE, Parameter( "LPVisMax" ) );
  mSignalVis.Send( CFGID::NUMSAMPLES, 5 / sampleDuration );
  ostringstream oss;
  oss << sampleDuration << "s";
  mSignalVis.Send( CFGID::sampleUnit, oss.str() );
}


void
LPFilter::Process( const GenericSignal* input, GenericSignal* output )
{
  // This implements the second line for all channels:
  for( size_t channel = 0; channel < input->Channels(); ++channel )
  {
    for( size_t sample = 0; sample < input->Elements(); ++sample )
    {
      mPreviousOutput[ channel ] *= mDecayFactor;
      mPreviousOutput[ channel ] += ( *input )( channel, sample ) * ( 1.0 - mDecayFactor );
      ( *output )( channel, sample ) = mPreviousOutput[ channel ];
    }
  }
  // Do the visualization if requested.
  if( Parameter( "VisualizeLowPass" ) == 1 )
    mSignalVis.Send( output );
}

