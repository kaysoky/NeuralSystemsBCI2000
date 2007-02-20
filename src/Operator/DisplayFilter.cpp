////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        DisplayFilter.cpp
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: An IIR filter class containing a HighPass, LowPass, and Notch
//              filter.
//
// $Log$
// Revision 1.1  2006/10/26 17:01:01  mellinger
// Initial version.
//
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "DisplayFilter.h"
#include "FilterDesign.h"
#include <numeric>
#include <limits>
#include <cassert>

using namespace std;

DisplayFilter::DisplayFilter()
: mHPCorner( 0 ),
  mLPCorner( 0 ),
  mNotchCenter( 0 )
{
}

DisplayFilter::~DisplayFilter()
{
}

DisplayFilter&
DisplayFilter::HPCorner( num_type inCorner )
{
  if( inCorner != mHPCorner )
  {
    mHPCorner = inCorner;
    DesignFilter();
  }
  return *this;
}

DisplayFilter::num_type
DisplayFilter::HPCorner() const
{
  return mHPCorner;
}

DisplayFilter&
DisplayFilter::LPCorner( num_type inCorner )
{
  if( inCorner != mLPCorner )
  {
    mLPCorner = inCorner;
    DesignFilter();
  }
  return *this;
}

DisplayFilter::num_type
DisplayFilter::LPCorner() const
{
  return mLPCorner;
}

DisplayFilter&
DisplayFilter::NotchCenter( num_type inCenter )
{
  if( inCenter != mNotchCenter )
  {
    mNotchCenter = inCenter;
    DesignFilter();
  }
  return *this;
}

DisplayFilter::num_type
DisplayFilter::NotchCenter() const
{
  return mNotchCenter;
}


void
DisplayFilter::DesignFilter()
{
  typedef Ratpoly<FilterDesign::Complex> TransferFunction;
  TransferFunction tf( 1.0 );
  num_type gain = 1.0;

  // Design a HP.
  if( mHPCorner > 0 && mHPCorner < 0.5 )
  {
    TransferFunction hp =
      FilterDesign::Butterworth()
      .Order( 1 )
      .Highpass( mHPCorner )
      .TransferFunction();
    tf *= hp;
    gain *= abs( hp.Evaluate( -1.0 ) ); // HF gain
  }
  // Add a LP.
  if( mLPCorner > 0 && mLPCorner < 0.5 )
  {
    TransferFunction lp =
      FilterDesign::Butterworth()
      .Order( 2 )
      .Lowpass( mLPCorner )
      .TransferFunction();
    tf *= lp;
    gain *= abs( lp.Evaluate( 1.0 ) ); // LF gain
  }
  // Add a notch filter.
  num_type corner1 = 0.9 * mNotchCenter,
           corner2 = 1.1 * mNotchCenter;
  if( corner1 > 0 && corner2 < 0.5 )
  {
    TransferFunction notch =
      FilterDesign::Chebyshev()
      .Ripple_dB( -0.1 )
      .Order( 4 )
      .Bandstop( corner1, corner2 )
      .TransferFunction();
    tf *= notch;
    gain *= abs( notch.Evaluate( 1.0 ) ); // LF gain
  }

  assert( gain > 0 );
  mGain = 1 / gain;
  mZeros = tf.Numerator().Roots();
  mPoles = tf.Denominator().Roots();
  Reset();
}

void
DisplayFilter::Reset()
{
  Initialize( 0 );
}

void
DisplayFilter::Initialize( size_t inChannels )
{
  mDelays.clear();
  mDelays.resize( inChannels, complex_vector( mZeros.size() + 1, 0 ) );
}

void
DisplayFilter::Process( const GenericSignal* input, GenericSignal* output )
{
  if( input->Channels() != mDelays.size() )
    Initialize( input->Channels() );

  assert( mZeros.size() == mPoles.size() );
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
        FilterDesign::Complex stageOutput = ( *input )( ch, sample ) * mGain;
        for( size_t stage = 0; stage < numStages; ++stage )
        {
          FilterDesign::Complex stageInput = stageOutput;
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


