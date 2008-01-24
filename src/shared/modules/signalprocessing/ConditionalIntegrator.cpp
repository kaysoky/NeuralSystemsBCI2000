////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that integrates its input signal while a given
//              expression evaluates to true.
//              This filter is intended for offline simulations of application
//              module behavior.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "ConditionalIntegrator.h"

using namespace std;

RegisterFilter( ConditionalIntegrator, 2.F ); // Place it last in signal processing.


ConditionalIntegrator::ConditionalIntegrator()
: mPreviousConditionValue( false )
{
 BEGIN_PARAMETER_DEFINITIONS
  "ConditionalIntegrator float IntegrationCondition= Feedback % % % "
    "// Perform integration while this condition is true",
 END_PARAMETER_DEFINITIONS
}


ConditionalIntegrator::~ConditionalIntegrator()
{
}


void
ConditionalIntegrator::Preflight( const SignalProperties& Input,
                                        SignalProperties& Output ) const
{
  GenericSignal preflightSignal( Input );
  Expression( Parameter( "IntegrationCondition" ) ).Evaluate( &preflightSignal );
  // Request output signal properties:
  Output = Input;
}


void
ConditionalIntegrator::Initialize( const SignalProperties& Input,
                                   const SignalProperties& Output )
{
  mSignal = GenericSignal( Input );
  mCondition = Expression( Parameter( "IntegrationCondition" ) );
  mPreviousConditionValue = false;
}


void
ConditionalIntegrator::Process( const GenericSignal& Input,
                                      GenericSignal& Output )
{
  bool currentConditionValue = mCondition.Evaluate( &Input );
  if( currentConditionValue )
    for( int channel = 0; channel < Input.Channels(); ++channel )
      for( int sample = 0; sample < Input.Elements(); ++sample )
        mSignal( channel, sample ) += Input( channel, sample );
  if( currentConditionValue && !mPreviousConditionValue )
    for( int channel = 0; channel < Input.Channels(); ++channel )
      for( int sample = 0; sample < Input.Elements(); ++sample )
        mSignal( channel, sample ) = 0;
  mPreviousConditionValue = currentConditionValue;

  Output = mSignal;
}

