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
#ifndef ARGROUP_H
#define ARGROUP_H

#include <iostream>
#include <vector>
#include "ARChannel.h"

class ARGroup
{
public:
	ARGroup();
	~ARGroup();
	void Init(int numChannels, ARparms parms);

	void Calculate(const GenericSignal *, GenericSignal *);
	void Calculate(double *, double *);

	int GetOutputElements(){return mOutputElements;}
	void setDoThreaded(bool doThreaded){mDoThreaded = doThreaded;}
private:
	void Clear();
	std::vector<ARthread *> ARarray;
	int mNumChannels, mOutChannels;
	int mLength;
	int mDataLength, mSFdataLength;
	int mOutputElements;
	ARparms mParms;
	bool mDoThreaded;
#if QT_CORE_LIB
	QThreadPool mThreadPool;
#endif // QT_CORE_LIB
};

#endif // ARGROUP_H
