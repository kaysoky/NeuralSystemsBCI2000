////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        Normalizer.h
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
// Revision 1.2  2006/02/03 13:40:53  mellinger
// Compatibility with gcc and BCB 2006.
//
// Revision 1.1  2006/01/13 15:04:46  mellinger
// Initial version.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef NormalizerH
#define NormalizerH

#include "UGenericFilter.h"
#include "UGenericVisualization.h"
#include "Expression/Expression.h"
#include <vector>
#include <deque>

class Normalizer : public GenericFilter
{
 public:
   Normalizer();
   ~Normalizer();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize2( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal*, GenericSignal* );
   void StopRun();

   static bool IsEven( int i ) { return ( i & 1 ) == 0; }

 private:
   enum NormalizerConstants
   {
     none = 0,

     // Offset adaptation
     removeMean = 1,
     removeMedian = 2,

     // Gain adaptation
     normalizeByVariance = 1,
     normalizeByMeanDifference = 2,
     normalizeByMedianDifference = 3,
   };

   void Adapt();

   // The GroupData class encapsulates details about data buffering and statistics.
   class GroupData : private std::vector<std::deque<std::vector<GenericSignal> > >
   {
    enum
    {
     // Preprocessing
     none = 0,
     segmentMeans = 1,
    };

    public:
     GroupData() : mMaxBufferedSegments( 0 ) {}
     ~GroupData() {}

    private:
     GroupData( const GroupData& );
     GroupData& operator=( const GroupData& );

    public:
     GroupData& MaxBufferedSegments( size_t );
     size_t MinBufferedSegments() const;

     GroupData& Clear() { return NumLabels( 0 ); }
     GroupData& NumLabels( size_t );
     size_t NumLabels() const { return size(); }

     GroupData& NewSegment( size_t label );
     GroupData& CloseSegment( size_t label, int preprocessingType );
     GroupData& AddData( size_t label, const GenericSignal& );

     float Mean( size_t channel ) const;
     template<class Iter>
       float Mean( size_t channel, const Iter labelsBegin, const Iter labelsEnd ) const;

     template<unsigned int Order>
       float CentralMoment( size_t channel ) const;
     template<unsigned int Order, class Iter>
       float CentralMoment( size_t channel, const Iter labelsBegin, const Iter labelsEnd ) const;

     float Median( size_t channel ) const;
     template<class Iter>
       float Median( size_t channel, const Iter labelsBegin, const Iter labelsEnd ) const;

    private:
     template<int Power, class Iter>
       double PowAccumulate(size_t channel, const Iter labelsBegin, const Iter labelsEnd ) const;
     template<typename T, class Iter>
       void CollectValues( size_t channel, const Iter labelsBegin, const Iter labelsEnd, std::vector<T>& ) const;

     typedef std::deque<std::vector<GenericSignal> > SegmentBuffer;
     size_t               mMaxBufferedSegments;
     std::vector<size_t>  mAllLabels;

   } mGroupData;

   std::vector<Expression> mSegmentingConditions;
   std::vector<bool>       mPreviousConditionValues;
   size_t                  mNumSegments;
   std::vector<float>      mOffsets,
                           mGains;
   bool                    mDoAdapt;

   bool                    mVisualize;
   GenericVisualization    mVis;
};
#endif // NormalizerH
