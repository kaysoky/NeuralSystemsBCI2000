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

#include <vector>
#include "ARChannel.h"

class ARGroup
{
public:
	ARGroup();
	~ARGroup();
	void Init(int numChannels, const ARparms& parms);

	template<typename T> void Calculate(const T*, T*);

	int GetOutputElements(){return mOutputElements;}
	void setDoThreaded(bool doThreaded){mDoThreaded = doThreaded;}
private:
	void Clear();
	std::vector<ARthread *> ARarray;
	int mOutputElements;
	ARparms mParms;
	bool mDoThreaded;
#if QT_CORE_LIB
	QThreadPool mThreadPool;
#endif // QT_CORE_LIB
};


template<typename T>
void ARGroup::Calculate(const T* inSignal, T* outSignal)
{
	if (mDoThreaded){
		for (size_t ch = 0; ch < ARarray.size(); ch++){
			ARarray[ch]->UpdateBuffer(inSignal);
#if QT_CORE_LIB
			mThreadPool.start(ARarray[ch]);
#else // QT_CORE_LIB
			ARarray[ch]->Process();
#endif // QT_CORE_LIB
		}
#if QT_CORE_LIB
		mThreadPool.waitForDone();
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
  case ARparms::SpectralAmplitude:
  case ARparms::SpectralPower:
		for (size_t ch = 0; ch < ARarray.size(); ch++){
			ARarray[ch]->UpdatePower(outSignal);
		}
		break;

  case ARparms::ARCoefficients:
		for (size_t ch = 0; ch < ARarray.size(); ch++){
			ARarray[ch]->UpdateCoeffs(outSignal);
		}
		break;
	}
}


#endif // ARGROUP_H
