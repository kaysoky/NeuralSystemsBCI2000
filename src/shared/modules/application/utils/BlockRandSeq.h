////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A generator of block randomized sequences.
//   Subsequent elements are obtained by calling NextElement().
//   The overall sequence is a concatenation of independent random permutations
//   of the numbers [1..<block size>], unless the block size is 0.
//   In the latter case, NextElement() will always return 0.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BLOCK_RAND_SEQ
#define BLOCK_RAND_SEQ

#include "State.h"
#include "RandomGenerator.h"
#include <vector>
#include <algorithm>

class BlockRandSeq
{
 public:
  typedef State::ValueType ValueType;

  BlockRandSeq( RandomGenerator& inGenerator )
    : mrGenerator( inGenerator ),
      mCurElement( mSequence.end() )
    {}
  ~BlockRandSeq()
    {}

  BlockRandSeq&  SetBlockSize( size_t );
  size_t         BlockSize() const
    { return mSequence.size(); }

  // Alternatively to specifying a block size,
  // specify a sequence of absolute frequencies for individual elements.
  template<class Container>
   BlockRandSeq& SetFrequencies( const Container& );

  ValueType      NextElement();

 private:
  typedef std::vector<State::ValueType> Sequence;
  Sequence                 mSequence;
  Sequence::const_iterator mCurElement;
  RandomGenerator&         mrGenerator;
};

////////////////////////////////////////////////////////////////////////////////
// Template definitions
////////////////////////////////////////////////////////////////////////////////
template<class Container>
BlockRandSeq&
BlockRandSeq::SetFrequencies( const Container& inFreq )
{
  mSequence.resize( std::accumulate( inFreq.begin(), inFreq.end(), 0 ) );
  size_t curValue = 0;
  Sequence::const_iterator curElement = mSequence.begin();
  for( Container::const_iterator i = inFreq.begin(); i != inFreq.end(); ++i )
  {
    ++curValue;
    for( Container::value_type j = 0; j < *i; ++j )
      *curElement++ = curValue;
  }
  mCurElement = mSequence.end();
  return *this;
}

#endif // BLOCK_RAND_SEQ