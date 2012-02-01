////////////////////////////////////////////////////////////////////////////////////
// $Id: NIDAQFilter.h 2708 2010-07-14 16:34:14Z mellinger $                       //
// Author: justin.renga@gmail.com                                                 //
// Description: A filter that performs output for National Instruments DAQ boards //
//                                                                                //
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
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
////////////////////////////////////////////////////////////////////////////////////
#ifndef NIDAQ_FILTER_H
#define NIDAQ_FILTER_H

#include "GenericFilter.h"
#include "OSThread.h"
#include "NIDAQmx.h"
#include "Expression.h"
#include <vector>
#include <string>
#include <map>
#include <fstream>

#define MAX_RANGES 32

class NIDAQFilter : public GenericFilter
{
	public:
		NIDAQFilter();				// Equivalent of Pulish() in a logger
		virtual ~NIDAQFilter();		// Desctructor
		// Standard Functions (with incoming signals) //
		virtual void Preflight(const SignalProperties&, SignalProperties&) const;
		virtual void Initialize(const SignalProperties&, const SignalProperties&);
		virtual void Process(const GenericSignal&, GenericSignal&);
		// Standard Functions (don't require incoming signals) //
		virtual void StartRun();
		virtual void StopRun();
		virtual void Halt();
		// Custom Member Functions //
		int		GetNumDigitalLines(std::string device);				//	acquires the number of digital lines
		int		GetNumAnalogOutputLines(std::string device);		//	acquries the number of analog output lines
	private:
		// Member Functions [Private] //
		int		ReportError(int errCode) const;						//	reports any NIDAQ error that may be called
		bool	AcquireAOVRanges();									//	gathers the available acceptable voltages
		static std::string IntToString(int n);						//	Returns a string version of integer argument
		static std::string FloatToString(float n);					//	Returns a string version of float argument
		std::vector<std::string> 	CollectDeviceNames();			//	collects the device names
		static bool find(std::string, std::vector<std::string>);	// determines if the specified device is connected to the computer
		// Member Variables [Private] //
		int							mFound[2];		//	array of number of channels found
		int							mCounter[2];	//	array of number of channels actually used
		bool						mRan;			//	has the filter been run at least once?
		bool						mUsed;			//	is there going to be logging?
		float						mSampleRate;	//	the speed of the sampling rate of the logger
		TaskHandle					mAnalog;		//	analog task handle
		TaskHandle					mDigital;		//	digital task handle
		std::string					mDevs[2];		//	array of the device names
		std::string					mActive[2];		//	array of active channel names
		std::vector<bool>			mLines;			//	the port usage distribution
		std::vector<float>			mRanges;		//	the voltage ranges for analog input
		std::vector<std::string>	mLNames;		//	the names of the physical channels on the device(s)
		std::vector<Expression>		mExpressions;	//	the Expressions being used by the filter
};
#endif		// NIDAQ_FILTER_H
