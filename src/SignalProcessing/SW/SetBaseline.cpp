////////////////////////////////////////////////////////////////////////////////
//
// File: SetBaseline.cpp
//
// Description: Slow Wave Class Definition
//           written by Dr. Thilo Hinterberger 2000-2001
//           Copyright University of Tuebingen, Germany
//
// Changes:  2003, juergen.mellinger@uni-tuebingen.de: some bugs fixed.
//           Feb 8, 2004, jm: Adaptations to changes in BCI2000 framework,
//           minor reformulations, reformatting.
//           Feb 24, 2004, jm: Moved the TSetBaseline class into separate files.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SetBaseline.h"

#include "UBCIError.h"

#define SECTION "Filtering"

using namespace std;

const char* baselineStateName = "Baseline";

RegisterFilter( TSetBaseline, 2.D1 );

TSetBaseline::TSetBaseline()
: mVisualize( 0 ),
  mVis( SOURCEID::Baseline, VISTYPE::GRAPH ),
  mLastBaselineState( 0 )
{
  BEGIN_PARAMETER_DEFINITIONS
    SECTION " intlist BaseChList= 2 1 1"
      " 0 0 1 // 1 to mark that BL is subtracted",
    "Visualize int VisualizeBaselineFiltering= 0"
      " 0 0 1 // visualize baseline filtered signals (0=no 1=yes)",
  END_PARAMETER_DEFINITIONS
}

void
TSetBaseline::Preflight( const SignalProperties& inSignalProperties,
                               SignalProperties& outSignalProperties ) const
{
  PreflightCondition(
    Parameter( "BaseChList" )->GetNumValues() <= inSignalProperties.Channels() );

  State( baselineStateName );
  outSignalProperties = inSignalProperties;
}

void
TSetBaseline::Initialize()
{
  mVisualize = Parameter( "VisualizeBaselineFiltering" );

  // allocating BL variables
  int numChan = Parameter( "BaseChList" )->GetNumValues();
  mBLSamples.resize( numChan, 0 );
  mBaseChList.resize( numChan, false );
  for( int i = 0; i < numChan; ++i )
    mBaseChList[ i ] = ( Parameter( "BaseChList", i ) > 0 );
  mBLSignal = GenericSignal( numChan, 1 );

  if( mVisualize )
  {
    mVis.Send( CFGID::WINDOWTITLE, "BaselineFiltered" );
    mVis.Send( CFGID::MINVALUE, 5 );
    mVis.Send( CFGID::MAXVALUE, -5 );
    mVis.Send( CFGID::NUMSAMPLES, 256 );
    mVis.Send( CFGID::showBaselines, true );
  }
  mLastBaselineState = 0;
}

void
TSetBaseline::Process( const GenericSignal* inSignal, GenericSignal* outSignal )
{
  *outSignal = *inSignal;
  GenericSignal& ioSignal = *outSignal;

  if( State( baselineStateName ) - mLastBaselineState == 1 )
  {
    for( size_t i = 0; i < mBLSignal.Channels(); ++i )
      mBLSignal( i, 0 ) = 0;
    for( size_t i = 0; i < mBLSamples.size(); ++i )
      mBLSamples[ i ] = 0;
  }

  if( State( baselineStateName ) )
    for( size_t i = 0; i < ioSignal.Channels(); ++i )
    {
      float sum = 0;
      for( size_t j = 0; j < ioSignal.GetNumElements( i ); ++j )
        sum += ioSignal( i, j );
      mBLSignal( i, 0 ) = mBLSignal( i, 0 ) * mBLSamples[ i ] + sum;
      mBLSamples[ i ] += ioSignal.GetNumElements( i );
      mBLSignal( i, 0 ) /= mBLSamples[ i ];
    }

  for( size_t i = 0; i < mBaseChList.size(); ++i )
    if( mBaseChList[ i ] )
      for( size_t j = 0; j < ioSignal.GetNumElements( i ); ++j )
        ioSignal( i, j ) -= mBLSignal( i, 0 );

  if( mVisualize )
    mVis.Send( &ioSignal );

  mLastBaselineState = State( baselineStateName );
}
