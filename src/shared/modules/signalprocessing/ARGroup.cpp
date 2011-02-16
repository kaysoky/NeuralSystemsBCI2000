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
#include <math.h>

ARGroup::ARGroup()
{
}

void ARGroup::Init(int numChannels, ARparms parms)
{
	Clear();
	mNumChannels = numChannels;
	mLength = parms.length;
	mParms = parms;
	mOutputElements = 0;

	int threadCount = 1;

#if QT_CORE_LIB
	threadCount = QThreadPool::globalInstance()->maxThreadCount();
#endif // QT_CORE_LIB

	ARarray.resize(std::min(mNumChannels, threadCount));
	for (size_t t = 0; t < ARarray.size(); t++){
		ARarray[t] = new ARthread();
		ARarray[t]->Init(t, mNumChannels, threadCount, parms);
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

void ARGroup::Calculate(const GenericSignal * inSignal, GenericSignal * outSignal)
{
	if (mDoThreaded){
		for (size_t ch = 0; ch < ARarray.size(); ch++){
			ARarray[ch]->UpdateBuffer(inSignal);
#if QT_CORE_LIB
			mThreadPool.start(ARarray[ch]);
		}
		mThreadPool.waitForDone();

#else // QT_CORE_LIB
			ARarray[ch]->Process();
		}
#endif // QT_CORE_LIB
	}
	else{
		for (size_t ch = 0; ch < ARarray.size(); ch++){
			ARarray[ch]->UpdateBuffer(inSignal);
			ARarray[ch]->Process();
		}
	}

	switch (mParms.outputType)
	{
	case 0:
	case 1:
		for (size_t ch = 0; ch < ARarray.size(); ch++){
			ARarray[ch]->UpdatePower(outSignal);
		}

		break;
	case 2:
		for (size_t ch = 0; ch < ARarray.size(); ch++){
			ARarray[ch]->UpdateCoeffs(outSignal);
		}
		break;
	}
}

void ARGroup::Calculate(double * inSignal, double * outSignal)
{
	if (mDoThreaded){
		for (size_t ch = 0; ch < ARarray.size(); ch++){
			ARarray[ch]->UpdateBuffer(inSignal);
#ifdef QT_CORE_LIB
			mThreadPool.start(ARarray[ch]);
		}
		mThreadPool.waitForDone();
#else // QT_CORE_LIB
			ARarray[ch]->Process();
		}
#endif // QT_CORE_LIB
	}
	else{
		for (size_t ch = 0; ch < ARarray.size(); ch++){
			ARarray[ch]->UpdateBuffer(inSignal);
			ARarray[ch]->Process();
		}
	}

	switch (mParms.outputType)
	{
	case 0:
	case 1:
		for (size_t ch = 0; ch < ARarray.size(); ch++){
			ARarray[ch]->UpdatePower(outSignal);
		}

		break;
	case 2:
		for (size_t ch = 0; ch < ARarray.size(); ch++){
			ARarray[ch]->UpdateCoeffs(outSignal);
		}
		break;
	}
}
