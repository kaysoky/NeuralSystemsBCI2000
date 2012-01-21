////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de, Adam Wilson
// Description:
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
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "SpatialFilterGroup.h"
#include "BCIAssert.h"

using namespace std;

SpatialFilterThread&
SpatialFilterThread::Start( const GenericSignal& Input, GenericSignal& Output )
{
  mpInput = &Input;
  mpOutput = &Output;
  if( !ReusableThread::Run( *this ) )
    throw bciexception( "Could not start execution: thread busy" );
  return *this;
}

void
SpatialFilterThread::OnRun()
{
  bciassert( mpInput != NULL && mpOutput != NULL );
  for( size_t i = 0; i < mSamples.size(); ++i )
  {
    int sample = mSamples[i];
    for( int outCh = 0; outCh < mpOutput->Channels(); ++outCh )
    {
      GenericSignal::ValueType value = 0;
      for( int inCh = 0; inCh < mpInput->Channels(); ++inCh )
        value += ( *mpInput )( inCh, sample ) * mrMatrix( outCh, inCh );
      ( *mpOutput )( outCh, sample ) = value;
    }
  }
}

void
SpatialFilterGroup::Clear()
{
  for( size_t i = 0; i < size(); ++i )
    delete ( *this )[i];
  clear();
}

void
SpatialFilterGroup::Initialize( const SignalProperties& inSignal, const GenericSignal& inMatrix )
{
  Clear();
  resize( min( inSignal.Elements(), OSThread::NumberOfProcessors() ) );
  for( size_t i = 0; i < size(); ++i )
    ( *this )[i] = new SpatialFilterThread( inMatrix );
  for( int i = 0; i < inSignal.Elements(); ++i )
    ( *this )[i % size()]->AddSample( i );
}

void
SpatialFilterGroup::Process( const GenericSignal& Input, GenericSignal& Output )
{
  for( size_t i = 0; i < size(); ++i )
    ( *this )[i]->Start( Input, Output );
  for( size_t i = 0; i < size(); ++i )
    ( *this )[i]->Wait();
}
