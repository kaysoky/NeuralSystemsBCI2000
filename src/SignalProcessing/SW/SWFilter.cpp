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
#include <assert>

#define SECTION "Filtering"

using namespace std;

const float infinity = 1e10;

const char* baselineStateName = "Baseline",
          * itiStateName = "IntertrialInterval";

// TSetBaseline class definitions.
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

// TFBArteCorrection class definitions.
RegisterFilter( TFBArteCorrection, 2.D2 );

TFBArteCorrection::TFBArteCorrection()
: mVisualize( 0 ),
  mVis( SOURCEID::Artefact, VISTYPE::GRAPH ),
  mArteMode( off )
{
  BEGIN_PARAMETER_DEFINITIONS
    SECTION " intlist ArteChList= 2 2 0"
      " 2 0 0 // Association of artefact channels with signal channels, 0 for no artefact channel",
    SECTION " floatlist ArteFactorList= 2 0.15 0"
      " 0 0 0 // Influence of artefact channel on input channel, -1: no artifact channel",
    SECTION " int ArteMode= 0"
      " 1 0 3 // Artefact correction mode: "
        "0 off, "
        "1 linear subtraction, "
        "2 subtraction if supporting, "
        "3 subtraction w/abort",
    "Visualize int VisualizeFBArteCorFiltering= 0"
      " 0 0 1 // visualize FBArte corrected signals (0=no 1=yes)",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "Artifact 1 0 0 0",
  END_STATE_DEFINITIONS
}

void
TFBArteCorrection::Preflight( const SignalProperties& inSignalProperties,
                                    SignalProperties& outSignalProperties ) const
{
  for( size_t i = 0; i < Parameter( "ArteChList" )->GetNumValues(); ++i )
  {
    PreflightCondition( Parameter( "ArteChList", i ) >= 0 );
    PreflightCondition( Parameter( "ArteChList", i ) <= inSignalProperties.Channels() );
  }
  outSignalProperties = inSignalProperties;
}

void
TFBArteCorrection::Initialize()
{
  mArteChList.clear();
  for( size_t i = 0; i < Parameter( "ArteChList" )->GetNumValues(); ++i )
    mArteChList.push_back( Parameter( "ArteChList", i ) - 1 );

  mArteFactorList.clear();
  for( size_t i = 0; i < Parameter( "ArteFactorList" )->GetNumValues(); ++i )
    mArteFactorList.push_back( Parameter( "ArteFactorList", i ) );

  mArteMode = static_cast<ArteMode>( ( int )Parameter( "ArteMode" ) );
  mVisualize = Parameter( "VisualizeFBArteCorFiltering" );
  if( mVisualize )
  {
    mVis.Send( CFGID::WINDOWTITLE, "Artefact filtered");
    mVis.Send( CFGID::MINVALUE, -100 );
    mVis.Send( CFGID::MAXVALUE, 100 );
    mVis.Send( CFGID::NUMSAMPLES, 256 );
  }
}

void
TFBArteCorrection::Process( const GenericSignal* inSignal, GenericSignal* outSignal )
{
  *outSignal = *inSignal;
  GenericSignal& ioSignal = *outSignal;

  for( size_t i = 0; i < mArteChList.size(); ++i )
    if( mArteChList[ i ] >= 0 )
      for( size_t j = 0; j < ioSignal.GetNumElements( i ); ++j )
      {
        float controlSignal = ioSignal( i, j ),
              arteSignal = ioSignal( mArteChList[ i ], j ) * mArteFactorList[ i ];

        switch( mArteMode )
        {
          case off:
            break;

          case linearSubtraction:
            // Linear subtraction.
            ioSignal( i, j ) = controlSignal - arteSignal;
            break;

          case subtractionIfSupporting:
          case subtractionWithAbort:
            if( arteSignal * controlSignal > 0 )
            { // If they have same signs:
              if( ::fabs( arteSignal ) < ::fabs( controlSignal ) )
              // If artefact is not too big, correct the signal.
                ioSignal( i, j ) = controlSignal - arteSignal;
              else
              { // Artefact is too big.
                ioSignal( i, j ) = 0; // FB is supressed.(?)
                if( mArteMode == subtractionWithAbort )
                  State( "Artifact" ) = 1;
              }
            }
            break;

          default:
            assert( false );
        }
      }

  if( mVisualize )
    mVis.Send( &ioSignal );
}

// TSW class definitions.
RegisterFilter( TSW, 2.C );

TSW::TSW()
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
      " 0 0 1 // visualize SW filtered signals (0=no 1=yes)",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "Artifact 1 0 0 0",
  END_STATE_DEFINITIONS
}

void
TSW::Preflight( const SignalProperties& Input,
                      SignalProperties& Output ) const
{
  if( Parameter( "UD_A" ) != 0 )
    bciout << "UD_A should be set to zero for Slow Waves" << endl;
  if( Parameter( "UD_B" ) < 300 || Parameter( "UD_B" ) > 400 )
    bciout << "UD_B should be set to 327.68 for Slow Waves" << endl;

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
TSW::Initialize()
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
    mVis.Send( CFGID::NUMSAMPLES, 256 );
  }
  mLastItiState = 0;
}

void
TSW::Process( const GenericSignal* InputSignal, GenericSignal* OutputSignal )
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
TSW::NewTrial()
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
TSW::AvgToBuffer( const GenericSignal& InputSignal )
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
TSW::CorrectTc()
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
TSW::AvgToSW( GenericSignal& OutputSignal )
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
TSW::CheckArtefacts( const GenericSignal& InputSignal )
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


