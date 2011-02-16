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
#ifndef SPATIAL_FILTER_CELL_H
#define SPATIAL_FILTER_CELL_H

#include <vector>
#include <valarray>
#include <map>
#ifndef __BORLANDC__
#include <QtCore>
#include <QRunnable>
#endif // __BORLANDC__
#include "GenericSignal.h"

#ifdef __BORLANDC__
class SpatialFilterCell
#else // __BORLANDC__
class SpatialFilterCell : public QRunnable
#endif // __BORLANDC__
{
protected:
  void run()
  {
    Process();
  }
public:
  SpatialFilterCell();
  ~SpatialFilterCell();

  void Clear();
  void Init(int nInChannels, int nOutChannels, int samples, int TID, int nThreads, double *);
  void Update(const GenericSignal * inSignal, GenericSignal * outSignal);
  void Process();

private:
  int mBlockSize;
  int mStart, mEnd;
  int mInChannels, mOutChannels;
  int mLength;
  float *mFilterMatrix;
  float *mBuffer;
  std::vector<int> mSampleMap;
  int mSamplesIterate;
  int mTID;
  GenericSignal *mOut;
};

#endif // SPATIAL_FILTER_CELL_H
