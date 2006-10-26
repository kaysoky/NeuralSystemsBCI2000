////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        IIRFilter.cpp
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: An abstract base class that implements a direct form II
//              transposed IIR filter.
//              Subclasses will provide individual implementations for the
//              DesignFilter() member which translates parameter settings into
//              filter coefficients.
//
// $Log$
// Revision 1.2  2006/10/26 17:05:00  mellinger
// Rewrote IIR filter as a sequence of complex-valued first-order filters to improve numerical stability.
//
// Revision 1.1  2006/05/04 17:06:43  mellinger
// Initial revision.
//
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "IIRFilter.h"
#include <numeric>
#include <limits>
#include <cassert>

using namespace std;

IIRFilter::IIRFilter()
{
}

IIRFilter::~IIRFilter()
{
}

void
IIRFilter::Preflight( const SignalProperties& input, SignalProperties& output ) const
{
  real_type      preflightGain;
  complex_vector preflightZeros,
                 preflightPoles;
  DesignFilter( preflightGain, preflightZeros, preflightPoles );
  if( preflightZeros.size() != preflightPoles.size() )
    bcierr << "The numbers of zeros and poles must agree" << endl;
  output = input;
}

void
IIRFilter::Initialize2( const SignalProperties& input, const SignalProperties& output )
{
  DesignFilter( mGain, mZeros, mPoles );
  mDelays.clear();
  mDelays.resize( input.Channels(), complex_vector( mZeros.size() + 1, 0 ) );
}

void
IIRFilter::StartRun()
{
  size_t numChannels = mDelays.size();
  mDelays.clear();
  mDelays.resize( numChannels, complex_vector( mZeros.size() + 1, 0 ) );
}

void
IIRFilter::Process( const GenericSignal* input, GenericSignal* output )
{
  size_t numStages = mZeros.size();
  if( numStages == 0 )
  {
    *output = *input;
  }
  else
  {
    for( size_t ch = 0; ch < input->Channels(); ++ch )
    {
      assert( mDelays[ch].size() == numStages + 1 );
      for( size_t sample = 0; sample < input->Elements(); ++sample )
      {
        // Implementing the filter as a sequence of complex-valued order 1
        // stages in DF I form will give us higher numerical stability and
        // lower code complexity than a sequence of real-valued order 2 stages.
        // - Numerical stability: Greatest for lowest order stages.
        // - Code complexity: Poles and zeros immediately translate into complex
        //    coefficients, and need not be grouped into complex conjugate pairs
        //    as would be the case for real-valued order 2 stages.
        complex_type stageOutput = ( *input )( ch, sample ) * mGain;
        for( size_t stage = 0; stage < numStages; ++stage )
        {
          complex_type stageInput = stageOutput;
          stageOutput = stageInput
            - mZeros[stage] * mDelays[ch][stage]
            + mPoles[stage] * mDelays[ch][stage+1];
          mDelays[ch][stage] = stageInput;
        }
        mDelays[ch][numStages] = stageOutput;
        ( *output )( ch, sample ) = real( stageOutput );
      }
    }
  }
}


