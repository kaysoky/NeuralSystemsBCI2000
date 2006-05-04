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
  num_seq_type preflightB, preflightA;
  DesignFilter( preflightB, preflightA );
  if( !preflightB.empty() )
  {
    if( preflightA.empty() )
      bcierr << "Output coefficients must contain at least one element" << endl;
    else if( fabs( preflightA[ 0 ] ) < numeric_limits<num_type>::epsilon() )
      bcierr << "First output coefficient must not be zero" << endl;
  }
  output = input;
}

void
IIRFilter::Initialize2( const SignalProperties& input, const SignalProperties& output )
{
  DesignFilter( mInputCoeffs, mOutputCoeffs );
  if( !mInputCoeffs.empty() )
  {
    for( size_t i = 0; i < mInputCoeffs.size(); ++i )
      mInputCoeffs[ i ] /= mOutputCoeffs[ 0 ];
    for( size_t i = 1; i < mOutputCoeffs.size(); ++i )
      mOutputCoeffs[ i ] /= mOutputCoeffs[ 0 ];
    mOutputCoeffs.erase( mOutputCoeffs.begin() );
    mDelayedInput.resize( input.Channels() );
    mDelayedOutput.resize( input.Channels() );
  }
}

void
IIRFilter::StartRun()
{
  size_t numChannels = mDelayedInput.size();
  mDelayedInput.clear();
  mDelayedInput.resize( numChannels, num_seq_type( mInputCoeffs.size(), 0 ) );
  mDelayedOutput.clear();
  mDelayedOutput.resize( numChannels, num_seq_type( mOutputCoeffs.size(), 0 ) );
}

void
IIRFilter::Process( const GenericSignal* input, GenericSignal* output )
{
  if( mInputCoeffs.empty() )
  {
    *output = *input;
  }
  else
  {
    for( size_t ch = 0; ch < input->Channels(); ++ch )
      for( size_t sample = 0; sample < input->Elements(); ++sample )
      {
        mDelayedInput[ ch ].push_front( ( *input )( ch, sample ) );
        mDelayedInput[ ch ].pop_back();
        ( *output )( ch, sample ) =
          inner_product( mInputCoeffs.begin(), mInputCoeffs.end(), mDelayedInput[ ch ].begin(), 0 )
          - inner_product( mOutputCoeffs.begin(), mOutputCoeffs.end(), mDelayedOutput[ ch ].begin(), 0 );
        mDelayedOutput[ ch ].push_front( ( *output )( ch, sample ) );
        mDelayedOutput[ ch ].pop_back();
      }
   }
}


