////////////////////////////////////////////////////////////////////////////////
//
// File: SWFilter.cpp
//
// Description: Slow Wave Class Definition
//              written by Dr. Thilo Hinterberger 2000-2001
//              Copyright University of Tuebingen, Germany
//
// Changes:     2003, juergen.mellinger@uni-tuebingen.de: some bugs fixed.
//              Feb 8, 2004, jm: Adaptations to changes in BCI2000 framework,
//              minor reformulations, reformatting.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SWFilter.h"

#include "UBCIError.h"
#include "MeasurementUnits.h"
#include <cmath>

#define SECTION "Filtering"

using namespace std;

const float infinity = 1e10;

const char* itiStateName = "IntertrialInterval";

RegisterFilter( TSWFilter, 2.C );

TSWFilter::TSWFilter()
: mVisualize( 0 ),
  mVis( SOURCEID::SW, VISTYPE::GRAPH ),
  mLastItiState( 0 ),
  mBufferOffset( 0 ),
  mPosInBuffer( 0 ),
  mBlocksInTrial( 0 ),
  mBlockSize( 0 ),
  mAvgSpan( 0 ),
  mSWCh( 0 ),
  mAvgBufferSize( 0 ),
  mTcFactor( 0.0 )
{
  BEGIN_PARAMETER_DEFINITIONS
    SECTION " float SWAvgSpan= 0.5s"
      " 0.5s 0 0 // Duration of averaging window",
    SECTION " intlist SWInChList= 2 1 2"
      " 0 0 0 // Channel number of input signal (include artifact channel!)",
    SECTION " intlist SWOutChList= 2 1 2"
      " 0 0 0 // Channel number of output signal (include artifact channel!)",
    SECTION " floatlist ThresholdAmp= 2 200 800"
      " 200 -2000 2000 // Threshold for invalid Trial in uV",
    SECTION " float Tc= 0"
      " 0 0 0 // Time constant, 0 for no correction",
    "Visualize int VisualizeSWFiltering= 1"
      " 0 0 1 // visualize SW filtered signals (0=no 1=yes) (boolean)",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "Artifact 1 0 0 0",
  END_STATE_DEFINITIONS
}

void
TSWFilter::Preflight( const SignalProperties& Input,
                      SignalProperties& Output ) const
{
  if( Parameter( "YMean" ) != 0 )
    bciout << "YMean should be set to zero for Slow Waves" << endl;
  if( Parameter( "YGain" ) < 300 || Parameter( "YGain" ) > 400 )
    bciout << "YGain should be set to 327.68 for Slow Waves" << endl;

  PreflightCondition( Parameter( "SampleBlockSize" ) > 0 );
  Parameter( "FeedbackEnd" );
  State( itiStateName );

  if( Parameter( "SWOutChList" )->GetNumValues() != Parameter( "SWInChList" )->GetNumValues() )
    bcierr << "The number of entries in SWOutChList must match that in SWInChList"
           << endl;
  for( size_t i = 0; i < Parameter( "SWInChList" )->GetNumValues(); ++i )
    PreflightCondition( Parameter( "SWInChList", i ) > 0 && Parameter( "SWInChList", i ) <= Input.Channels() );
  for( size_t i = 0; i < Parameter( "SWOutChList" )->GetNumValues(); ++i )
    PreflightCondition( Parameter( "SWOutChList", i ) > 0 && Parameter( "SWOutChList", i ) <= Input.Channels() );
  PreflightCondition( MeasurementUnits::ReadAsTime( Parameter( "Tc" ) ) >= 0.0 );
  PreflightCondition( Parameter( "SpatialFilteredChannels" ) == Input.Channels() );
  Output = Input;
}

void
TSWFilter::Initialize()
{
  mBlockSize = Parameter( "SampleBlockSize" );
  mBlocksInTrial = MeasurementUnits::ReadAsTime( Parameter( "FeedbackEnd" ) );
  mBufferOffset = mBlocksInTrial;
  mAvgBufferSize = mBufferOffset + mBlocksInTrial + 1;
  mAvgSpan = MeasurementUnits::ReadAsTime( Parameter( "SWAvgSpan" ) );

  mSWCh = Parameter( "SWInChList" )->GetNumValues();
  mSWInChList.resize( mSWCh );
  mSWOutChList.resize( mSWCh );
  mThresholdAmp.resize( mSWCh );
  for( int i = 0; i < mSWCh; ++i )
  {
    mSWInChList[ i ] = Parameter( "SWInChList", i ) - 1;
    mSWOutChList[ i ] = Parameter( "SWOutChList", i ) - 1;
    mThresholdAmp[ i ] = Parameter( "ThresholdAmp", i );
  }

  mAvgBlockBuffer = GenericSignal( mSWCh, mAvgBufferSize );
  for( int n = 0; n < mAvgBufferSize; n++ )
    for( int m = 0; m < mSWCh; m++ )
      mAvgBlockBuffer( m, n ) = 0;
  mMaxValue.clear();
  mMaxValue.resize( mSWCh, -infinity );
  mMinValue.clear();
  mMinValue.resize( mSWCh, infinity );

  mPosInBuffer = mBufferOffset - 1;

  // Tc-correction variables:
  float timeConstant = MeasurementUnits::ReadAsTime( Parameter( "Tc" ) );
  if( timeConstant == 0 )
    mTcFactor = 0;
  else
    mTcFactor = 1.0 - ::exp( -timeConstant );
  mTcAk.clear();
  mTcAk.resize( mSWCh, 0 );

  mVisualize = Parameter( "VisualizeSWFiltering" );
  if( mVisualize )
  {
    mVis.Send( CFGID::WINDOWTITLE, "SWFiltered" );
    mVis.Send( CFGID::MINVALUE, -100 );
    mVis.Send( CFGID::MAXVALUE, 100 );
    mVis.Send( CFGID::NUMSAMPLES, 256 );
  }
  mLastItiState = 0;
}

void
TSWFilter::Process( const GenericSignal* InputSignal, GenericSignal* OutputSignal )
{
  *OutputSignal = *InputSignal;
  if( mPosInBuffer < mAvgBufferSize - 1 )
    ++mPosInBuffer;
  if( State( itiStateName ) - mLastItiState == -1 )
    NewTrial();
  AvgToBuffer( *InputSignal );
  CorrectTc();
  AvgToSW( *OutputSignal );
  CheckArtefacts( *OutputSignal );

  if( mVisualize )
    mVis.Send( OutputSignal );

  mLastItiState = State( itiStateName );
}

// SW calculation: arithmetic functions.
void
TSWFilter::NewTrial()
{
  for( int n = 0; n < mPosInBuffer - mBufferOffset; n++ )
    for( short m = 0; m < mSWCh; m++ )
      mAvgBlockBuffer( m, n + 2 * mBufferOffset - mPosInBuffer ) =
                                mAvgBlockBuffer( m, n + mBufferOffset );
  for( short m = 0; m < mSWCh; m++ )
    mTcAk[ m ] = 0;
  for( short m = 0; m < mSWCh; m++ )
    mMaxValue[ m ] = -infinity;
  for( short m = 0; m < mSWCh; m++ )
    mMinValue[ m ] = infinity;

  mPosInBuffer = mBufferOffset;
  State( "Artifact" ) = 0;
}

void
TSWFilter::AvgToBuffer( const GenericSignal& InputSignal )
{
  for( short m = 0; m < mSWCh; m++ )
  {
    float zsum = 0;
    for( unsigned int n = 0; n < mBlockSize; n++ )
      zsum += InputSignal( mSWInChList[ m ], n );
    mAvgBlockBuffer( m, mPosInBuffer ) = zsum / mBlockSize;
  }
}

void
TSWFilter::CorrectTc()
{
  for( short m = 0; m < mSWCh; m++ )
  {
    double aux = ( mTcFactor * ( mAvgBlockBuffer( m, mPosInBuffer )
                 + mAvgBlockBuffer( m, mPosInBuffer - 1 ) - mTcAk[ m ] ) ) / 2;
    mTcAk[ m ] += aux;
    mAvgBlockBuffer( m, mPosInBuffer ) += mTcAk[ m ];
  }
}

void
TSWFilter::AvgToSW( GenericSignal& OutputSignal )
{
  for( short m = 0; m < mSWCh; m++ )
  {
    float zsum = 0;
    for( unsigned int n = 0; n < mAvgSpan; n++ )
      zsum += mAvgBlockBuffer( m, mPosInBuffer - n );
    OutputSignal( mSWOutChList[ m ], 0 ) = zsum / mAvgSpan;
  }
}

void
TSWFilter::CheckArtefacts( const GenericSignal& InputSignal )
{
  for( short m = 0; m < mSWCh; m++ )
  {
    if( mThresholdAmp[ m ] > 0 )
    {
      if( InputSignal( mSWOutChList[ m ], 0 ) < mMinValue[ m ] )
        mMinValue[ m ] = InputSignal( mSWOutChList[ m ], 0 );
      if( InputSignal( mSWOutChList[ m ], 0 ) > mMaxValue[ m ] )
        mMaxValue[ m ] = InputSignal( mSWOutChList[ m ], 0 );
      if( mMaxValue[ m ] - mMinValue[ m ] > mThresholdAmp[ m ] )
        State( "Artifact" ) = 1;
    }
  }
}


