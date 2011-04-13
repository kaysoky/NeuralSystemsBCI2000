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

#include "SpatialFilterGroup.h"

using namespace std;

SpatialFilterGroup::SpatialFilterGroup()
{
  mFilterMatrix = NULL;
}
SpatialFilterGroup::~SpatialFilterGroup()
{
}

void SpatialFilterGroup::Clear()
{
  for (size_t i = 0; i < mSF.size(); i++)
    delete mSF[i];
  mSF.clear();
  if (mFilterMatrix != NULL)
    delete [] mFilterMatrix;
  mFilterMatrix = NULL;
}

void SpatialFilterGroup::Init(int nInChannels, int nOutChannels, int nSamples, vector < valarray<double> > * spatialFilter)
{
  Clear();
  mInChannels = nInChannels;
  mOutChannels = nOutChannels;
  mSamples = nSamples;
  mThreadCount = 1;
#ifdef USE_QT
  mThreadCount = QThreadPool::globalInstance()->maxThreadCount();
#endif // USE_QT
  mFilterMatrix = new double[mInChannels*mOutChannels];
  for (int inCh = 0; inCh < mInChannels; inCh++)
    for (int outCh = 0; outCh < mOutChannels; outCh++)
      mFilterMatrix[outCh + inCh*mOutChannels] = (*spatialFilter)[outCh][inCh];
  for (int i = 0; i < mThreadCount; i++){
      SpatialFilterCell *tmp = new SpatialFilterCell();
    tmp->Init(mInChannels, mOutChannels, mSamples, i, mThreadCount, mFilterMatrix);
    mSF.push_back(tmp);
  }
}

void SpatialFilterGroup::Calculate(const GenericSignal * in, GenericSignal * out, bool doThreaded)
{
  for (size_t i = 0; i < mSF.size(); i++){
    mSF[i]->Update(in, out);
  }
  if (!doThreaded){
    for (size_t i = 0; i < mSF.size(); i++){
      mSF[i]->Process();
    }
  }
  else{
  #ifdef USE_QT
    for (size_t i = 0; i < mSF.size(); i++){
      //QThreadPool::globalInstance()->start(mSF[i]);
		mThreadPool.start(mSF[i]);
    }

    //QThreadPool::globalInstance()->waitForDone();
	mThreadPool.waitForDone();

  #else // USE_QT
    for (size_t i = 0; i < mSF.size(); i++){
      mSF[i]->Process();
    }
  #endif // USE_QT
  }
}
