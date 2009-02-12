////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: An IIR filter class containing a HighPass, LowPass, and Notch
//              filter.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "DisplayFilter.h"
#include "FilterDesign.h"
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
DisplayFilter::HPCorner( Real inCorner )
{
  if( inCorner != mHPCorner )
  {
    mHPCorner = inCorner;
    DesignFilter();
  }
  return *this;
}

DisplayFilter::Real
DisplayFilter::HPCorner() const
{
  return mHPCorner;
}

DisplayFilter&
DisplayFilter::LPCorner( Real inCorner )
{
  if( inCorner != mLPCorner )
  {
    mLPCorner = inCorner;
    DesignFilter();
  }
  return *this;
}

DisplayFilter::Real
DisplayFilter::LPCorner() const
{
  return mLPCorner;
}

DisplayFilter&
DisplayFilter::NotchCenter( Real inCenter )
{
  if( inCenter != mNotchCenter )
  {
    mNotchCenter = inCenter;
    DesignFilter();
  }
  return *this;
}

DisplayFilter::Real
DisplayFilter::NotchCenter() const
{
  return mNotchCenter;
}


void
DisplayFilter::DesignFilter()
{
  typedef Ratpoly<FilterDesign::Complex> TransferFunction;
  TransferFunction tf( 1.0 );
  Real gain = 1.0;

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
  Real corner1 = 0.9 * mNotchCenter,
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
  mFilter.SetGain( 1 / gain )
         .SetZeros( tf.Numerator().Roots() )
         .SetPoles( tf.Denominator().Roots() );
  Reset();
}

void
DisplayFilter::Reset()
{
  mFilter.Initialize();
}

void
DisplayFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  if( Input.Channels() != mFilter.Channels() )
    mFilter.Initialize( Input.Channels() );
  mFilter.Process( Input, Output );
}


