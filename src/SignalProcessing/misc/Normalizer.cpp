////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        Normalizer.cpp
// Date:        Dec 30, 2005
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that outputs a normalized version of its input signal.
//              The filter groups its input signal according to a set of
//              conditions given as boolean expressions.
//              A pre-defined number of previous segments for each condition is then
//              used to normalize the signal such that its user-controlled
//              variation corresponds to a unit range, i.e. a zero mean
//              signal will be normalized to the range [-0.5, 0.5].
// $Log$
// Revision 1.4  2006/11/28 18:36:48  gschalk
// *** empty log message ***
//
// Revision 1.3  2006/02/03 13:40:53  mellinger
// Compatibility with gcc and BCB 2006.
//
// Revision 1.2  2006/01/17 17:39:44  mellinger
// Fixed list of project files.
//
// Revision 1.1  2006/01/13 15:04:46  mellinger
// Initial version.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "Normalizer.h"
#include <algorithm>
#include <numeric>
#include <set>

using namespace std;

RegisterFilter( Normalizer, 2.E1 );

Normalizer::Normalizer()
: mNumSegments( 0 ),
  mDoAdapt( false ),
  mVisualize( false ),
  mVis( SOURCEID::NORMALIZER )
{
 BEGIN_PARAMETER_DEFINITIONS

  // NormalizerOffsets corresponds to X/YMean.
  "Normalizer floatlist NormalizerOffsets= 2 0 -1 "
    "0 0 0 // Normalizer offsets",

  // NormalizerGains corresponds to X/YGain.
  "Normalizer floatlist NormalizerGains= 2 0 1 "
    "0 0 0 // Normalizer gain values",

  // SegmentingConditions corresponds to BaselineCfg but contains general boolean
  // expressions.
  "Normalizer matrix SegmentingConditions= 2 1 "
    "(TargetCode==1)&&(Feedback!=0) "
    "(TargetCode==2)&&(Feedback!=0) "
    "% % % // Conditional expressions for segmenting  "
    "(rows correspond to groups, multiple columns will be ANDed within rows)",

  // OffsetAdaptation and GainAdaptation correspond to X/YTrendControl.
  "Normalizer intlist OffsetAdaptation= 2 2 0 "
    "0 0 1 // 0: no offset adaptation, "
             "1: remove mean, "
             "2: remove median",

  "Normalizer intlist GainAdaptation= 2 1 0 "
    "0 0 2 // 0: no gain adaptation, "
             "1: normalize by variance, "
             "2: normalize by mean difference, "
             "3: normalize by median difference",

  "Normalizer int NormalizerPreprocessing= 1 "
    "0 0 1 // data preprocessing prior to computation of statistic "
             "0: none, "
             "1: segment means "
             "(enumeration)",

  // NumSegments corresponds to SignalWinLth.
  "Normalizer int NumSegments= 3 "
    "0 0 0 // number of past segments for each group that enters into adaptation",

   "Visualize int VisualizeNormalFiltering= 1 "
     "0 0 1 // visualize normalized signals (0=no 1=yes) (boolean)",

 END_PARAMETER_DEFINITIONS
}


Normalizer::~Normalizer()
{
}


void
Normalizer::Preflight( const SignalProperties& input,
                             SignalProperties& output ) const
{
  PreflightCondition(
    Parameter( "NormalizerOffsets" )->GetNumValues() >= input.Channels() );
  PreflightCondition(
    Parameter( "NormalizerGains" )->GetNumValues() >= input.Channels() );

  ParamRef OffsetAdaptation = Parameter( "OffsetAdaptation" ),
           GainAdaptation = Parameter( "GainAdaptation" );
  PreflightCondition( OffsetAdaptation->GetNumValues() >= input.Channels() );
  PreflightCondition( GainAdaptation->GetNumValues() >= input.Channels() );
  bool adaptation = false;
  for( size_t channel = 0; channel < input.Channels(); ++channel )
  {
    adaptation |= ( OffsetAdaptation( channel ) != none );
    adaptation |= ( GainAdaptation( channel ) != none );
  }
  if( adaptation )
  {
    // Evaluate all expressions to check for errors.
    ParamRef SegmentingConditions = Parameter( "SegmentingConditions" );
    for( size_t row = 0; row < SegmentingConditions->GetNumRows(); ++row )
      for( size_t col = 0; col < SegmentingConditions->GetNumColumns(); ++col )
        Expression( SegmentingConditions( row, col ) ).Evaluate();

    int NumSegments = Parameter( "NumSegments" );
    PreflightCondition( NumSegments > 0 );
  }
  // Request output signal properties:
  output = input;
}


void
Normalizer::Initialize2( const SignalProperties& input,
                         const SignalProperties& output )
{
  mSegmentingConditions.clear();
  mGroupData.Clear();
  mPreviousConditionValues.clear();

  mOffsets.clear();
  mGains.clear();

  mNumSegments = Parameter( "NumSegments" );

  ParamRef OffsetAdaptation = Parameter( "OffsetAdaptation" ),
           GainAdaptation = Parameter( "GainAdaptation" ),
           NormalizerOffsets = Parameter( "NormalizerOffsets" ),
           NormalizerGains = Parameter( "NormalizerGains" );

  mDoAdapt = false;
  for( size_t channel = 0; channel < input.Channels(); ++channel )
  {
    mOffsets.push_back( NormalizerOffsets( channel ) );
    mGains.push_back( NormalizerGains( channel ) );

    mDoAdapt |= ( OffsetAdaptation( channel ) != none );
    mDoAdapt |= ( GainAdaptation( channel ) != none );
  }

  if( mDoAdapt )
  {
    ParamRef SegmentingConditions = Parameter( "SegmentingConditions" );
    for( size_t row = 0; row < SegmentingConditions->GetNumRows(); ++row )
    {
      ostringstream oss;
      for( size_t col = 0; col < SegmentingConditions->GetNumColumns(); ++col )
        oss << "&&(" << SegmentingConditions( row, col ) << ")";
      mSegmentingConditions.push_back( Expression( oss.str().substr( 2 ).c_str() ) );
    }
    mPreviousConditionValues.resize( mSegmentingConditions.size() );
    mGroupData.NumLabels( mSegmentingConditions.size() )
              .MaxBufferedSegments( mNumSegments );
  }

  mVisualize = ( Parameter( "VisualizeNormalFiltering" ) > 0 );
  if( mVisualize )
  {
    mVis.Send( CFGID::WINDOWTITLE, "Normalizer" );
    mVis.Send( CFGID::MINVALUE, -2 );
    mVis.Send( CFGID::MAXVALUE, 2 );
    mVis.Send( CFGID::NUMSAMPLES, 256 );
  }
}


void
Normalizer::Process( const GenericSignal* input, GenericSignal* output )
{
  if( mDoAdapt )
  {
    for( size_t label = 0; label < mSegmentingConditions.size(); ++label )
    {
      bool currentCondition = mSegmentingConditions[ label ].Evaluate(),
           previousCondition = mPreviousConditionValues[ label ];
      mPreviousConditionValues[ label ] = currentCondition;
      if( currentCondition != previousCondition )
      {
        if( currentCondition )
          mGroupData.NewSegment( label );
        else
        {
          mGroupData.CloseSegment( label, Parameter( "NormalizerPreprocessing" ) );
          Adapt();
        }
      }
      if( currentCondition )
        mGroupData.AddData( label, *input );
    }
  }
  for( size_t channel = 0; channel < input->Channels(); ++channel )
    for( size_t sample = 0; sample < input->Elements(); ++sample )
      ( *output )( channel, sample )
        = ( ( *input )( channel, sample ) - mOffsets[ channel ] ) * mGains[ channel ];

  if( mVisualize )
    mVis.Send( output );
}


void
Normalizer::StopRun()
{
  if( mDoAdapt )
  {
    for( size_t label = 0; label < mSegmentingConditions.size(); ++label )
      mGroupData.CloseSegment( label, Parameter( "NormalizerPreprocessing" ) );
    Adapt();
    for( size_t channel = 0; channel < mOffsets.size(); ++channel )
    {
      Parameter( "NormalizerOffsets" )( channel ) = mOffsets[ channel ];
      Parameter( "NormalizerGains" )( channel ) = mGains[ channel ];
    }
  }
}

void
Normalizer::Adapt()
{
  if( mGroupData.MinBufferedSegments() > 0 )
  {
    for( size_t channel = 0; channel < mOffsets.size(); ++channel )
    {
      switch( int( Parameter( "OffsetAdaptation" )( channel ) ) )
      {
        case none:
          break;

        case removeMean:
          mOffsets[ channel ] = mGroupData.Mean( channel );
          break;

        case removeMedian:
          mOffsets[ channel ] = mGroupData.Median( channel );
          break;

        default:
          bcierr << "Unknown offset adaptation method for channel " << channel << endl;
      }
      switch( int( Parameter( "GainAdaptation" )( channel ) ) )
      {
        case none:
          break;

        case normalizeByVariance:
          { // Find the two labels corresponding to maximum and minimum mean.
            vector<float> means;
            for( size_t label = 0; label < mGroupData.NumLabels(); ++label )
              means.push_back( mGroupData.Mean( channel, &label, ( &label ) + 1 ) );
            // Compute the variance for the data with those two labels.
            size_t var_labels[] =
            {
              max_element( means.begin(), means.end() ) - means.begin(),
              min_element( means.begin(), means.end() ) - means.begin(),
            };
            float variance = mGroupData.CentralMoment<2>(
              channel,
              var_labels,
              var_labels + sizeof( var_labels ) / sizeof( *var_labels )
            );
            // Normalize to a standard deviation of 1/2.
            if( variance < numeric_limits<float>::epsilon() )
              variance = 0.25;
            mGains[ channel ] = 0.5 / ::sqrt( variance );
          }
          break;

        case normalizeByMeanDifference:
          { // Compute the difference between maximum and minimum of per-label
            // means.
            vector<float> means;
            for( size_t label = 0; label < mGroupData.NumLabels(); ++label )
              means.push_back( mGroupData.Mean( channel, &label, ( &label ) + 1 ) );
            float meanDiff = *max_element( means.begin(), means.end() )
                           - *min_element( means.begin(), means.end() );
            // Normalize to a mean difference of 1.
            if( meanDiff < numeric_limits<float>::epsilon() )
              meanDiff = 1.0;
            mGains[ channel ] = 1.0 / meanDiff;
          }
          break;

        case normalizeByMedianDifference:
          { // Compute the difference between maximum and minimum of per-label
            // medians.
            vector<float> medians;
            for( size_t label = 0; label < mGroupData.NumLabels(); ++label )
              medians.push_back( mGroupData.Median( channel, &label, ( &label ) + 1 ) );
            float medianDiff = *max_element( medians.begin(), medians.end() )
                             - *min_element( medians.begin(), medians.end() );
            // Normalize to a median difference of 1.
            if( medianDiff < numeric_limits<float>::epsilon() )
              medianDiff = 1.0;
            mGains[ channel ] = 1.0 / medianDiff;
          }
          break;

        default:
          bcierr << "Unknown gain adaptation method for channel " << channel << endl;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Normalizer::GroupData definitions                                          //
////////////////////////////////////////////////////////////////////////////////
Normalizer::GroupData&
Normalizer::GroupData::NumLabels( size_t inNumLabels )
{
  this->resize( inNumLabels );
  mAllLabels.clear();
  for( size_t i = 0; i < inNumLabels; ++i )
    mAllLabels.push_back( i );
  return *this;
}

Normalizer::GroupData&
Normalizer::GroupData::MaxBufferedSegments( size_t inMaxBufferedSegments )
{
  mMaxBufferedSegments = inMaxBufferedSegments;
  for( size_t label = 0; label < this->size(); ++label )
    while( ( *this )[ label ].size() > ( mMaxBufferedSegments + 1 ) )
      ( *this )[ label ].pop_front();
  return *this;
}

size_t
Normalizer::GroupData::MinBufferedSegments() const
{
  if( empty() )
    return 0;

  size_t result = ( *this )[ 0 ].size();
  for( size_t label = 1; label < this->size(); ++label )
    if( ( *this )[ label ].size() < result )
      result = ( *this )[ label ].size();

  return result;
}

Normalizer::GroupData&
Normalizer::GroupData::CloseSegment( size_t inLabel, int inPreprocessing )
{
  switch( inPreprocessing )
  {
    case none:
      break;

    case segmentMeans:
      { // Replace segment data by its mean over samples.
        typedef vector<GenericSignal> Segment;
        if( !this->at( inLabel ).empty() )
        {
          Segment& segmentData = this->at( inLabel ).back();
          if( !segmentData.empty() )
          {
            GenericSignal meanData( segmentData.front().Channels(), 1, SignalType::float32 );
            for( size_t channel = 0; channel < segmentData.front().Channels(); ++channel )
            {
              double sum = 0.0;
              for( size_t sample = 0; sample < segmentData.front().Elements(); ++sample )
                for( Segment::iterator i = segmentData.begin(); i != segmentData.end(); ++i )
                  sum += ( *i )( channel, sample );
              meanData( channel, 0 ) = sum / ( segmentData.size() * segmentData.front().Elements() );
            }
            segmentData.clear();
            segmentData.push_back( meanData );
          }
        }
      }
      break;

    default:
      bcierr << "Unknown preprocessing type" << endl;
  }
  while( this->at( inLabel ).size() > mMaxBufferedSegments )
    this->at( inLabel ).pop_front();
  return *this;
}

Normalizer::GroupData&
Normalizer::GroupData::NewSegment( size_t inLabel )
{
  this->at( inLabel ).resize( this->at( inLabel ).size() + 1 );
  return *this;
}

Normalizer::GroupData&
Normalizer::GroupData::AddData( size_t inLabel, const GenericSignal& inSignal )
{
  this->at( inLabel ).rbegin()->push_back( inSignal );
  return *this;
}

float
Normalizer::GroupData::Mean( size_t inChannel ) const
{
  return Mean( inChannel, mAllLabels.begin(), mAllLabels.end() );
}

template<class Iter>
float
Normalizer::GroupData::Mean( size_t inChannel,
                             const Iter inLabelsBegin,
                             const Iter inLabelsEnd ) const
{
  return PowAccumulate<1>( inChannel, inLabelsBegin, inLabelsEnd )
         / PowAccumulate<0>( inChannel, inLabelsBegin, inLabelsEnd );
}

template<unsigned int Order>
float
Normalizer::GroupData::CentralMoment( size_t inChannel ) const
{
  return CentralMoment<Order>( inChannel, mAllLabels.begin(), mAllLabels.end() );
}

template<unsigned int Order, class Iter>
float
Normalizer::GroupData::CentralMoment( size_t inChannel,
                                      const Iter inLabelsBegin,
                                      const Iter inLabelsEnd ) const
{
  float result = 0.0;
  switch( Order )
  {
    case 0:
      bcierr << "Zeroth order moment requested" << endl;
      break;
    case 1:
      result = 0.0;
      break;
    default:
      {
        double N = PowAccumulate<0>( inChannel, inLabelsBegin, inLabelsEnd );
        result = PowAccumulate<Order>( inChannel, inLabelsBegin, inLabelsEnd );
        if( N > 1 )
          result /= ( N - 1 );
      }
  }
  return result;
}

float
Normalizer::GroupData::Median( size_t inChannel ) const
{
  return Median( inChannel, mAllLabels.begin(), mAllLabels.end() );
}

template<class Iter>
float
Normalizer::GroupData::Median( size_t inChannel,
                               const Iter inLabelsBegin,
                               const Iter inLabelsEnd ) const
{
  float median = 0.0;
  vector<float> values;
  CollectValues( inChannel, inLabelsBegin, inLabelsEnd, values );
  if( values.empty() )
    bcierr << "Empty data range" << endl;
  else
  {
    size_t numValues = values.size();
    if( IsEven( numValues ) )
    {
      // Partial sort until the two centermost values have their correct positions.
      nth_element( values.begin(), values.begin() + numValues / 2 - 1, values.end() );
      nth_element( values.begin(), values.begin() + numValues / 2, values.end() );
      median = ( values[ numValues / 2 - 1 ] + values[ numValues / 2 ] ) / 2;
    }
    else
    {
      // Partial sort until the median value has its correct position.
      nth_element( values.begin(), values.begin() + ( numValues - 1 ) / 2, values.end() );
      median = values[ ( numValues - 1 ) / 2 ];
    }
  }
  return median;
}

// Accumulate a power sum of channel data for a given range of iterators over labels.
template<int Power, class Iter>
double
Normalizer::GroupData::PowAccumulate( size_t inChannel,
                                      const Iter inLabelsBegin,
                                      const Iter inLabelsEnd ) const
{
  double result = 0.0;
  vector<double> values;
  CollectValues( inChannel, inLabelsBegin, inLabelsEnd, values );
  if( !values.empty() )
  {
    switch( Power )
    {
      case 0:
        result = values.size();
        break;
      case 1:
        result = accumulate( values.begin(), values.end(), double( 0.0 ) );
        break;
      default:
        {
          double mean = accumulate( values.begin(), values.end(), double( 0.0 ) )
                        / values.size();
          for( size_t i = 0; i < values.size(); ++i )
          {
            double value = values[ i ] - mean,
                   summand = value * value;
            for( int j = 2; j < Power; ++j )
              summand *= value;
            result += summand;
          }
        }
    }
  }
  return result;
}

template<typename T, class Iter>
void
Normalizer::GroupData::CollectValues( size_t inChannel,
                                      const Iter inLabelsBegin,
                                      const Iter inLabelsEnd,
                                      vector<T>& ioValues ) const
{
  for( Iter label = inLabelsBegin; label != inLabelsEnd; ++label )
  {
    size_t segmentCount = 0;
    for( SegmentBuffer::const_iterator segment = this->at( *label ).begin();
         segmentCount < mMaxBufferedSegments && segment != this->at( *label ).end();
         ++segmentCount, ++segment )
      for( vector<GenericSignal>::const_iterator signalBlock = segment->begin();
           signalBlock != segment->end();
           ++signalBlock )
        for( size_t sample = 0; sample < signalBlock->Elements(); ++sample )
          ioValues.push_back( ( *signalBlock )( inChannel, sample ) );
  }
}

