#include "PCHIncludes.h"
#pragma hdrstop

#include "AverageDisplay.h"
#include "defines.h"
#include "Color.h"
#include <vector>
#include <map>

using namespace std;

RegisterFilter( AverageDisplay, 2.C1 );

const RGBColor AverageDisplay::sChannelColors[] =
{
  Red, Green, Blue, Yellow, White,
};

AverageDisplay::AverageDisplay()
: mLastTargetCode( 0 )
{
 BEGIN_PARAMETER_DEFINITIONS
   "Visualize matrix AvgDisplayCh= 2 2 "
      "1 Channel%201 "
      "2 Channel%202 "
        "0 0 0 // Channels and display names for average display",
 END_PARAMETER_DEFINITIONS
}

void
AverageDisplay::Preflight( const SignalProperties& inProperties, SignalProperties& outProperties ) const
{
  PreflightCondition( Parameter( "AvgDisplayCh" )->GetNumValuesDimension2() >= 2 );
  for( size_t i = 0; i < Parameter( "AvgDisplayCh" )->GetNumValuesDimension1(); ++i )
    PreflightCondition
    (
      Parameter( "AvgDisplayCh", i, 0 ) > 0
      && Parameter( "AvgDisplayCh", i, 0 ) <= inProperties.Channels()
    );
  State( "TargetCode" );
  outProperties = inProperties;
}

void
AverageDisplay::Initialize()
{
  mVisualizations.clear();
  mChannelIndices.clear();
  mPowerSums.clear();
  mTargetCodes.clear();
  mSignalOfCurrentRun.clear();
#ifdef SET_BASELINE
  mBaselines.clear();
  mBaselineSamples.clear();
#endif // SET_BASELINE
  size_t numChannels = Parameter( "AvgDisplayCh" )->GetNumValuesDimension1();
  mPowerSums.resize( maxPower + 1, vector<vector<vector<float> > >( numChannels ) );
  for( size_t i = 0; i < numChannels; ++i )
  {
     mVisualizations.push_back( GenericVisualization( SOURCEID::Average + i, VISTYPE::GRAPH ) );
     GenericVisualization& vis = mVisualizations[ i ];

     string windowTitle = ( const char* )Parameter( "AvgDisplayCh", i, 1 );
     if( windowTitle == "" )
       windowTitle = "unknown";
     windowTitle += " Average";
     vis.Send( CFGID::WINDOWTITLE, windowTitle.c_str() );
     // Note min and max value are interchanged to account for EEG display direction.
     vis.Send( CFGID::MINVALUE, 50 );
     vis.Send( CFGID::MAXVALUE, -50 );
     vis.Send( CFGID::NUMSAMPLES, 0 );
     vis.Send( CFGID::graphType, CFGID::polyline );

     Colorlist channelColors;
     channelColors.resize( numChannels );
     const size_t numColors = sizeof( sChannelColors ) / sizeof( *sChannelColors );
     for( size_t i = 0; i < numChannels; ++i )
       channelColors[ i ] = sChannelColors[ i % numColors ];
     vis.Send( CFGID::channelColors, channelColors );

     vis.Send( CFGID::channelGroupSize, 0 );
     vis.Send( CFGID::showBaselines, 1 );

     mChannelIndices.push_back( Parameter( "AvgDisplayCh", i, 0 ) - 1 );
  }
  
  mSignalOfCurrentRun.resize( numChannels );
#ifdef SET_BASELINE
  mBaselines.resize( numChannels );
  mBaselineSamples.resize( numChannels );
#endif // SET_BASELINE
  mLastTargetCode = 0;
}

void
AverageDisplay::Process( const GenericSignal* inputSignal, GenericSignal* outputSignal )
{
#ifdef TODO
# error Factor out the power sums into a class.
#endif // TODO
  const GenericSignal& signal = *inputSignal;
  size_t targetCode = State( "TargetCode" );
  if( targetCode == 0 && targetCode != mLastTargetCode )
  {
    size_t targetIndex = find( mTargetCodes.begin(), mTargetCodes.end(), mLastTargetCode ) - mTargetCodes.begin();
    if( targetIndex == mTargetCodes.size() )
      mTargetCodes.push_back( mLastTargetCode );
    // End of the current target code run.
    for( size_t i = 0; i < mChannelIndices.size(); ++i )
    {
      for( int power = 0; power <= maxPower; ++power )
      {
        // - If the target code occurred for the first time, adapt the power sums.
        if( mPowerSums[ power ][ i ].size() <= targetIndex )
          mPowerSums[ power ][ i ].resize( targetIndex + 1 );
        // - Update power sum sizes.
        if( mPowerSums[ power ][ i ][ targetIndex ].size() < mSignalOfCurrentRun[ i ].size() )
          mPowerSums[ power ][ i ][ targetIndex ].resize( mSignalOfCurrentRun[ i ].size(), 0 );
      }
#ifdef SET_BASELINE
      if( mBaselineSamples[ i ] > 0 )
        mBaselines[ i ] /= mBaselineSamples[ i ];
#endif // SET_BASELINE
      // - Compute the power sum entries.
      for( size_t j = 0; j < mSignalOfCurrentRun[ i ].size(); ++j )
      {
#ifdef SET_BASELINE
        mSignalOfCurrentRun[ i ][ j ] -= mBaselines[ i ];
#endif // SET_BASELINE
        float summand = 1.0;
        for( size_t power = 0; power < maxPower; ++power )
        {
          mPowerSums[ power ][ i ][ targetIndex ][ j ] += summand;
          summand *= mSignalOfCurrentRun[ i ][ j ];
        }
        mPowerSums[ maxPower ][ i ][ targetIndex ][ j ] += summand;
      }
    }
    // - Clear target run buffer.
    for( size_t i = 0; i < mSignalOfCurrentRun.size(); ++i )
      mSignalOfCurrentRun[ i ].clear();
#ifdef SET_BASELINE
    for( size_t i = 0; i < mBaselines.size(); ++i )
    {
      mBaselineSamples[ i ] = 0;
      mBaselines[ i ] = 0;
    }
#endif // SET_BASELINE
    // - Compute and display the averages.
    for( size_t channel = 0; channel < mVisualizations.size(); ++channel )
    {
      size_t numTargets = mPowerSums[ maxPower ][ channel ].size(),
             numSamples = numeric_limits<size_t>::max();
      for( size_t target = 0; target < numTargets; ++target )
        if( mPowerSums[ maxPower ][ channel ][ target ].size() < numSamples )
          numSamples = mPowerSums[ maxPower ][ channel ][ target ].size();

      // To minimize user confusion, always send target averages in ascending order
      // of target codes. This ensures that colors in the display don't depend
      // on the order of target codes in the task sequence once all target codes
      // occurred.
      // We cannot, however, avoid color changes when yet unknown target codes
      // occur.
      //
      // The map is automatically sorted by its "key", so all we need to do
      // is to put the target codes and their indices into it, using the
      // target code as "key" and the index as "value", and later iterate over
      // the map to get the indices sorted by their associated target code.
      map<int, int> targetCodesToIndex;
      for( size_t target = 0; target < numTargets; ++target )
        targetCodesToIndex[ mTargetCodes[ target ] ] = target;

      GenericSignal average( numTargets, numSamples );
      for( map<int, int>::const_iterator target = targetCodesToIndex.begin();
                                   target != targetCodesToIndex.end(); ++target )
        for( size_t sample = 0; sample < numSamples; ++sample )
          // If everything behaves as we believe it will,
          // a division by zero is impossible.
          // If it occurs nevertheless, the logic is messed up.
          average( target->second, sample ) =
            mPowerSums[ 1 ][ channel ][ target->second ][ sample ] /
                          mPowerSums[ 0 ][ channel ][ target->second ][ sample ];
      mVisualizations[ channel ].Send( &average );
    }
  }
  if( targetCode != 0 )
  {
    // Store the current signal to the end of the run buffer.
    for( size_t i = 0; i < mChannelIndices.size(); ++i )
    {
      size_t signalCursorPos = mSignalOfCurrentRun[ i ].size();
      mSignalOfCurrentRun[ i ].resize( signalCursorPos + signal.Elements() );
      for( size_t j = 0; j < signal.Elements(); ++j )
        mSignalOfCurrentRun[ i ][ signalCursorPos + j ] = signal( mChannelIndices[ i ], j );
    }
#ifdef SET_BASELINE
    if( OptionalState( "BaselineInterval" ) || OptionalState( "Baseline" ) )
      for( size_t i = 0; i < mChannelIndices.size(); ++i )
      {
        mBaselineSamples[ i ] += signal.Elements();
        for( size_t j = 0; j < signal.Elements(); ++j )
          mBaselines[ i ] += signal( mChannelIndices[ i ], j );
      }
#endif // SET_BASELINE
  }
  mLastTargetCode = targetCode;
  *outputSignal = *inputSignal;
}




