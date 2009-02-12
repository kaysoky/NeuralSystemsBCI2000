////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that outputs a normalized version of its input signal.
//   The filter buffers its input signal according to a user-defined
//   set of conditions given as boolean expressions.
//   These data are then used to normalize the signal such that its
//   total variance (user controlled plus noise) corresponds to a
//   unit range, i.e. a zero mean signal will be normalized to the
//   range [-1,1].
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef NORMALIZER_H
#define NORMALIZER_H

#include "GenericFilter.h"
#include "Expression.h"
#include <vector>
#include <valarray>

class Normalizer : public GenericFilter
{
 public:
   Normalizer();
   ~Normalizer();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal&, GenericSignal& );
   void StartRun();
   void StopRun();

 private:
   enum AdaptationTypes
   {
     none = 0,
     zeroMean,
     zeroMeanUnitVariance,
   };

   void Update();

   class RingBuffer
   {
    public:
     typedef std::valarray<float> DataVector;

     explicit RingBuffer( size_t inSize )
       : mData( 0.0, inSize ),
         mCursor( 0 ),
         mWrapped( false )
       {}

     const size_t Fill() const
       { return mWrapped ? mData.size() : mCursor; }
     const DataVector& Data() const
       { return mData; }

     void Put( float inData )
       {
         if( ++mCursor == mData.size() )
         {
           mWrapped = true;
           mCursor = 0;
         }
         if( mData.size() > 0 )
           mData[ mCursor ] = inData;
       }

    private:
     DataVector mData;
     size_t     mCursor;
     bool       mWrapped;
   };

   std::vector< std::vector<RingBuffer> > mDataBuffers;
   std::vector< std::vector<Expression> > mBufferConditions;

   Expression*      mpUpdateTrigger;
   bool             mPreviousTrigger;
   std::vector<int> mAdaptation;

   std::vector<float>   mOffsets,
                        mGains;
   bool                 mDoAdapt;
};
#endif // NORMALIZER_H
