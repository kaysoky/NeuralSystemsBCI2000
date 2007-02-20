////////////////////////////////////////////////////////////////////////////////
//  $Id$
//  File:        RandomFilter.cpp
//  Date:        Oct 13, 2005
//  Author:      juergen.mellinger@uni-tuebingen.de
//  Description: A filter that returns zero-mean white noise multiplied by the
//               input signal's value.
//  $Log$
//  Revision 1.1  2006/01/13 15:04:46  mellinger
//  Initial version.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "RandomFilter.h"

#include "UGenericVisualization.h"
#include "UBCIError.h"
#include <cmath>
#include <cstdlib>

using namespace std;

RegisterFilter( RandomFilter, 2.C1 );

RandomFilter::RandomFilter()
: mpGetRandom( 0 ),
  mpVis( 0 )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Filtering int RandomNoise= 0 0 0 1 // 0: none, 1: uniform (enumeration)",
  END_PARAMETER_DEFINITIONS
}


RandomFilter::~RandomFilter()
{
  delete mpVis;
}


void
RandomFilter::Preflight( const SignalProperties& inputProperties,
                               SignalProperties& outputProperties ) const
{
  int outputChannels = inputProperties.Channels();
  if( Parameter( "RandomNoise" ) > 0 )
    outputChannels *= 2;
  // Request output signal properties:
  outputProperties = SignalProperties(
    outputChannels,
    inputProperties.Elements(),
    inputProperties.Type()
  );
}


void
RandomFilter::Initialize2( const SignalProperties& inputProperties,
                           const SignalProperties& outputProperties )
{
  switch( int( Parameter( "RandomNoise" ) ) )
  {
    case none:
      break;

    case uniform:
      mpGetRandom = GetRandomUniform;
      break;
  }
  if( mpGetRandom )
    ::randomize();

#if 0
  delete mpVis;
  if( mpGetRandom )
  {
    mpVis = new GenericVisualization( SOURCEID::Debug );
    mpVis->Send( CFGID::MINVALUE, -20 );
    mpVis->Send( CFGID::MAXVALUE, 20 );
  }
  else
  {
    mpVis = 0;
  }
#endif
}


void
RandomFilter::Process( const GenericSignal* input, GenericSignal* output )
{
  for( size_t channel = 0; channel < input->Channels(); ++channel )
    for( size_t sample = 0; sample < input->Elements(); ++sample )
    {
      ( *output )( channel, sample ) = ( *input )( channel, sample );
      if( mpGetRandom )
        ( *output )( channel + input->Channels(), sample ) =
          ( *input )( channel, sample ) * mpGetRandom();
    }
  if( mpVis )
    mpVis->Send( output );
}


float
RandomFilter::GetRandomUniform()
{
  return ( ::rand() * 2.0 / RAND_MAX ) - 1.0;
}

