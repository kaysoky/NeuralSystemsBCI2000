////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: Adam Wilson
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

#include "SpatialFilterCell.h"
#include <algorithm>
#include <numeric>

//#define USEINNERPRODUCT

using namespace std;

SpatialFilterCell::SpatialFilterCell()
: mBlockSize(1),
  mStart(0),
  mEnd(0),
  mInChannels(0),
  mOutChannels(0),
  mLength(0),
  mFilterMatrix(NULL),
  mBuffer(NULL)
{
#ifdef USE_QT
  setAutoDelete(false);
#endif // USE_QT
}

SpatialFilterCell::~SpatialFilterCell()
{
  Clear();
}

void SpatialFilterCell::Clear()
{
  if (mFilterMatrix != NULL)
    delete [] mFilterMatrix;
  if (mBuffer != NULL)
    delete [] mBuffer;

  mFilterMatrix = NULL;
  mBuffer = NULL;
}

void SpatialFilterCell::Init(int nInChannels, int nOutChannels, int samples, int TID, int nThreads, double *spatialFilter)
{
  Clear();

  mInChannels = nInChannels;
  mOutChannels = nOutChannels;
  mLength = samples;
  mBlockSize = nThreads;
  mTID = TID;

  for (int i = mTID; i < mLength; i+= mBlockSize)
    mSampleMap.push_back(i);

  mBuffer = new float[mSampleMap.size()* mInChannels];

  mFilterMatrix = new float[mInChannels * mOutChannels];
  for (int i = 0; i < mInChannels*mOutChannels; i++){
      //mFilterMatrix[ch].resize((*spatialFilter)[ch].size());
    mFilterMatrix[i] = spatialFilter[i];
  }
}

void SpatialFilterCell::Update(const GenericSignal * in, GenericSignal * out)
{
  for (int inCh = 0; inCh < in->Channels(); inCh++)
  {
    for (int s = mTID, pos=0; s < in->Elements(); s+= mBlockSize, pos++)
    {
      mBuffer[pos + inCh*mSampleMap.size()]= (*in)(inCh,s);
    }
  }
  mOut = out;
}

void SpatialFilterCell::Process()
{
  float v;

  for (int outCh = 0; outCh < mOutChannels; outCh++){
    for (size_t s = 0; s < mSampleMap.size(); s++){
      v = 0;
      for (int inCh = 0; inCh < mInChannels; inCh++)
        v += mFilterMatrix[outCh + inCh*mOutChannels]*mBuffer[s + inCh*mSampleMap.size()];
      (*mOut)(outCh,mSampleMap[s]) = v;
    }
  }
}
