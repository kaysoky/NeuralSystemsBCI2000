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
#include "progressClass.h"
#include <iostream>

progressClass::progressClass(bool useGUI, QLabel *label, QProgressBar *progress)
{
	mUseGUI = useGUI;

	mLabel = label;
	mProgress = progress;
}

void progressClass::init(int max)
{
	mMax = max;
	if (mUseGUI){
		mProgress->setRange(0, max);
		mProgress->reset();
		connect(this, SIGNAL(incrementBar(int)), mProgress, SLOT(setValue(int)));
	}
	mCurVal = 0;
}


void progressClass::increment(QString txt)
{
	mCurVal++;
	if (mUseGUI){

		mLabel->setText(txt);
		emit incrementBar(mCurVal);
	}
	else{
		std::cout << mCurVal << "/" << mMax << ": " << txt.toStdString() << std::endl;
	}
}

