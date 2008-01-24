////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: An abstract base class that implements a direct form II
//              transposed IIR filter.
//              Subclasses will provide individual implementations for the
//              DesignFilter() member which translates parameter settings into
//              filter coefficients.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
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
IIRFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  Real          preflightGain;
  ComplexVector preflightZeros,
                preflightPoles;
  DesignFilter( preflightGain, preflightZeros, preflightPoles );
  if( preflightZeros.size() != preflightPoles.size() )
    bcierr << "The numbers of zeros and poles must agree" << endl;
  Output = Input;
}

void
IIRFilter::Initialize( const SignalProperties& Input, const SignalProperties& /*Output*/ )
{
  DesignFilter( mGain, mZeros, mPoles );
  mDelays.clear();
  mDelays.resize( Input.Channels(), ComplexVector( mZeros.size() + 1, 0 ) );
}

void
IIRFilter::StartRun()
{
  size_t numChannels = mDelays.size();
  mDelays.clear();
  mDelays.resize( numChannels, ComplexVector( mZeros.size() + 1, 0 ) );
}

void
IIRFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  int numStages = mZeros.size();
  if( numStages == 0 )
  {
    Output = Input;
  }
  else
  {
    for( int ch = 0; ch < Input.Channels(); ++ch )
    {
      assert( int( mDelays[ch].size() ) == numStages + 1 );
      for( int sample = 0; sample < Input.Elements(); ++sample )
      {
        // Implementing the filter as a sequence of complex-valued order 1
        // stages in DF I form will give us higher numerical stability and
        // lower code complexity than a sequence of real-valued order 2 stages.
        // - Numerical stability: Greatest for lowest order stages.
        // - Code complexity: Poles and zeros immediately translate into complex
        //    coefficients, and need not be grouped into complex conjugate pairs
        //    as would be the case for real-valued order 2 stages.
        Complex stageOutput = Input( ch, sample ) * mGain;
        for( int stage = 0; stage < numStages; ++stage )
        {
          Complex stageInput = stageOutput;
          stageOutput = stageInput
            - mZeros[stage] * mDelays[ch][stage]
            + mPoles[stage] * mDelays[ch][stage+1];
          mDelays[ch][stage] = stageInput;
        }
        mDelays[ch][numStages] = stageOutput;
        Output( ch, sample ) = real( stageOutput );
      }
    }
  }
}


