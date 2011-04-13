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

#include "ARGroup.h"
#include <algorithm>

ARGroup::ARGroup()
{
}

void ARGroup::Init(int numChannels, const ARparms& parms)
{
	Clear();
	mParms = parms;

	int threadCount = 1;

#ifdef USE_QT
	threadCount = QThreadPool::globalInstance()->maxThreadCount();
#endif // USE_QT

	ARarray.resize(std::min(numChannels, threadCount));
	for (size_t t = 0; t < ARarray.size(); t++){
		ARarray[t] = new ARthread();
		ARarray[t]->Init(t, numChannels, threadCount, parms);
	}
	mOutputElements = ARarray[0]->GetOutputElements();
}

void ARGroup::Clear()
{
	for (size_t ch = 0; ch < ARarray.size(); ch++)
		delete ARarray[ch];
	ARarray.clear();
}

ARGroup::~ARGroup()
{
	Clear();
}
