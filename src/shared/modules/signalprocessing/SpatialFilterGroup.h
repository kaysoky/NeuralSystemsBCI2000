////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de, Adam Wilson
//   SpatialFilterThread: A class that encapsulates a single thread that
//   computes the spatial filter result for a subset of samples, and writes
//   them into an output signal. Multiple threads may operate on the same input
//   and output signal concurrently, provided that their sample sets do not
//   overlap.
//   SpatialFilterGroup: A set of SpatialFilterThreads that computes the full
//   result of applying a spatial filter matrix.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef SPATIAL_FILTER_GROUP_H
#define SPATIAL_FILTER_GROUP_H

#include "ReusableThread.h"
#include "Runnable.h"
#include "GenericSignal.h"
#include "Environment.h"
#include <vector>

// These types correspond to configurations that a SpatialFilterThread can handle.
typedef GenericSignal FullMatrix;
struct SparseMatrixEntry
{
  int input,
      output;
  GenericSignal::ValueType weight;
};
typedef std::vector<SparseMatrixEntry> SparseMatrix;
typedef std::vector<int> CAROutputList;

class SpatialFilterThread : public ReusableThread, private Runnable
{
 public:
  SpatialFilterThread( const FullMatrix& inFullMatrix )
    : mpFullMatrix( &inFullMatrix ), mpSparseMatrix( NULL ), mpCAROutputList( NULL ) {}
  SpatialFilterThread( const SparseMatrix& inSparseMatrix )
    : mpFullMatrix( NULL ), mpSparseMatrix( &inSparseMatrix ), mpCAROutputList( NULL ) {}
  SpatialFilterThread( const CAROutputList& inCAROutputList )
    : mpFullMatrix( NULL ), mpSparseMatrix( NULL ), mpCAROutputList( &inCAROutputList ) {}

  SpatialFilterThread& AddSample( int inSample )
    { mSamples.push_back( inSample ); return *this; }

  SpatialFilterThread& Start( const GenericSignal& Input, GenericSignal& Output );

 private:
  void OnRun();

 private:
  const GenericSignal* mpInput;
  GenericSignal* mpOutput;
  const FullMatrix* mpFullMatrix;
  const SparseMatrix* mpSparseMatrix;
  const CAROutputList* mpCAROutputList;
  std::vector<int> mSamples;
};

class SpatialFilterGroup : private std::vector<SpatialFilterThread*>, private Environment
{
 public:
  ~SpatialFilterGroup() { Clear(); }
  void Clear();
  void Preflight() const;
  template<class T> void Initialize( const SignalProperties&, const T& );
  void Process( const GenericSignal&, GenericSignal& );
};

template<class T>
void
SpatialFilterGroup::Initialize( const SignalProperties& inSignal, const T& inConfig )
{
  Clear();
  int numberOfThreads = OptionalParameter( "NumberOfThreads", -1 );
  if( numberOfThreads <= 0 )
    numberOfThreads = OSThread::NumberOfProcessors();
  resize( std::min( inSignal.Elements(), numberOfThreads ) );
  for( size_t i = 0; i < size(); ++i )
    ( *this )[i] = new SpatialFilterThread( inConfig );
  for( int i = 0; i < inSignal.Elements(); ++i )
    ( *this )[i % size()]->AddSample( i );
}

#endif // SPATIAL_FILTER_GROUP_H
