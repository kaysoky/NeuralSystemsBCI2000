////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that takes a unit variance, zero mean signal and
//              scales it according to a desired minimum trial duration, and
//              system update rate.
//              The purpose of this filter is to connect old-style application
//              modules to the new normalizer filter without changing their
//              behavior.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "TaskAdapter.h"
#include "MeasurementUnits.h"

using namespace std;

RegisterFilter( TaskAdapter, 2.99999 ); // Place it immediately before the task filter.

TaskAdapter::TaskAdapter()
{
 BEGIN_PARAMETER_DEFINITIONS
  // MinFeedbackDuration is a more intuitive replacement for the X/YPixelsPerSec parameters.
  "UsrTask float MinFeedbackDuration= 3s 0 0 0 "
    "// Time for the cursor to cross the task display for a unit control signal",
 END_PARAMETER_DEFINITIONS
}


TaskAdapter::~TaskAdapter()
{
}


void
TaskAdapter::Preflight( const SignalProperties& input,
                              SignalProperties& output ) const
{
  float MinFeedbackDuration = MeasurementUnits::ReadAsTime( Parameter( "MinFeedbackDuration" ) );
  PreflightCondition( MinFeedbackDuration > 0 );
  // Request output signal properties:
  output = input;
}


void
TaskAdapter::Initialize( const SignalProperties& input,
                         const SignalProperties& output )
{
  mGains.clear();
  mGains.resize( ::max( 2, input.Channels() ), 1.0 );

  float MinFeedbackDuration = MeasurementUnits::ReadAsTime( Parameter( "MinFeedbackDuration" ) );
  mGains[ 0 ] = float( 0x7fff ) / MinFeedbackDuration;
  mGains[ 1 ] = mGains[ 0 ];
}


void
TaskAdapter::Process( const GenericSignal& input, GenericSignal& output )
{
  for( int channel = 0; channel < input.Channels(); ++channel )
    for( int sample = 0; sample < input.Elements(); ++sample )
      output( channel, sample ) = input( channel, sample ) * mGains[ channel ];
}


