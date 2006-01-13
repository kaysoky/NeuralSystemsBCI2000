////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        ConditionalIntegrator.cpp
// Date:        Jan 2, 2006
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that integrates its input signal while a given
//              expression evaluates to true.
//              This filter is intended for offline simulations of application
//              module behavior.
// $Log$
// Revision 1.1  2006/01/13 15:04:46  mellinger
// Initial version.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "ConditionalIntegrator.h"

using namespace std;

RegisterFilter( ConditionalIntegrator, 2.F ); // Place it last in signal processing.

ConditionalIntegrator::ConditionalIntegrator()
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
ConditionalIntegrator::Preflight( const SignalProperties& input,
                                        SignalProperties& output ) const
{
  GenericSignal preflightSignal( input );
  Expression( Parameter( "IntegrationCondition" ) ).Evaluate( &preflightSignal );
  // Request output signal properties:
  output = input;
}


void
ConditionalIntegrator::Initialize2( const SignalProperties& input,
                                    const SignalProperties& output )
{
  mSignal = GenericSignal( input );
  mCondition = Expression( Parameter( "IntegrationCondition" ) );
}


void
ConditionalIntegrator::Process( const GenericSignal* input, GenericSignal* output )
{
  if( mCondition.Evaluate( input ) )
    for( size_t channel = 0; channel < input->Channels(); ++channel )
      for( size_t sample = 0; sample < input->Elements(); ++sample )
        mSignal( channel, sample ) += ( *input )( channel, sample );
  else
    for( size_t channel = 0; channel < input->Channels(); ++channel )
      for( size_t sample = 0; sample < input->Elements(); ++sample )
        mSignal( channel, sample ) = 0;

  *output = mSignal;
}


