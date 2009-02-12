////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A generator of block randomized sequences.
//   Subsequent elements are obtained by calling NextElement().
//   The overall sequence is a concatenation of independent random permutations
//   of the numbers [1..<block size>], unless the block size is 0.
//   In the latter case, NextElement() will always return 0.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BlockRandSeq.h"
#include <algorithm>
#include <cstdlib>

using namespace std;

BlockRandSeq&
BlockRandSeq::SetBlockSize( size_t inSize )
{
  mSequence.resize( inSize );
  for( size_t i = 0; i < mSequence.size(); ++i )
    mSequence[ i ] = i + 1;
  mCurElement = mSequence.end();
  return *this;
}

BlockRandSeq::ValueType
BlockRandSeq::NextElement()
{
  ValueType result = 0;
  if( mCurElement == mSequence.end() )
  {
    random_shuffle( mSequence.begin(), mSequence.end(), mrGenerator );
    mCurElement = mSequence.begin();
  }
  if( mCurElement != mSequence.end() )
    result = *mCurElement++;
  return result;
}

