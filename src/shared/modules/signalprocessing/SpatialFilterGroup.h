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
#ifndef SPATIALFILTERGROUP_H
#define SPATIALFILTERGROUP_H

#include "SpatialFilterCell.h"
#include <vector>
#include <valarray>

class SpatialFilterGroup{
public:
  SpatialFilterGroup();
  ~SpatialFilterGroup();

  void Clear();
  void Init(int nInChannels, int nOutChannels, int nSamples, std::vector<std::valarray<double> > *spatialFilter);
  void Calculate(const GenericSignal * inSignal, GenericSignal * outSignal, bool doThreaded);
private:
  std::vector<SpatialFilterCell *> mSF;
  int mThreadCount;
  double *mFilterMatrix;
  int mInChannels, mOutChannels, mSamples;
#ifdef USE_QT
  QThreadPool mThreadPool;
#endif // USE_QT
};

#endif // SPATIALFILTERGROUP_H
