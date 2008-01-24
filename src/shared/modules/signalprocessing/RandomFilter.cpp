////////////////////////////////////////////////////////////////////////////////
//  $Id$
//  Author:      juergen.mellinger@uni-tuebingen.de
//  Description: A filter that returns zero-mean white noise multiplied by the
//               input signal's value.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "RandomFilter.h"
#include "BCIError.h"
#include <cmath>
#include <cstdlib>

using namespace std;

RegisterFilter( RandomFilter, 2.C1 );

RandomFilter::RandomFilter()
: mpGetRandom( NULL )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Filtering int RandomNoise= 0 0 0 1 // 0: none, 1: uniform (enumeration)",
  END_PARAMETER_DEFINITIONS
}


RandomFilter::~RandomFilter()
{
}


void
RandomFilter::Preflight( const SignalProperties& Input,
                               SignalProperties& Output ) const
{
  int outputChannels = Input.Channels();
  if( Parameter( "RandomNoise" ) > 0 )
    outputChannels *= 2;
  // Request output signal properties:
  Output = Input;
  Output.SetChannels( outputChannels );
}


void
RandomFilter::Initialize( const SignalProperties& Input,
                          const SignalProperties& Output )
{
  switch( int( Parameter( "RandomNoise" ) ) )
  {
    case none:
      break;

    case uniform:
      mpGetRandom = &RandomFilter::GetRandomUniform;
      break;
  }
}


void
RandomFilter::Process( const GenericSignal& Input,
                             GenericSignal& Output )
{
  for( int channel = 0; channel < Input.Channels(); ++channel )
    for( int sample = 0; sample < Input.Elements(); ++sample )
    {
      Output( channel, sample ) = Input( channel, sample );
      if( mpGetRandom )
        Output( channel + Input.Channels(), sample ) =
          Input( channel, sample ) * ( this->*mpGetRandom )();
    }
}


float
RandomFilter::GetRandomUniform()
{
  return ( mRandomGenerator.Random() * 2.0 / mRandomGenerator.RandMax() ) - 1.0;
}

