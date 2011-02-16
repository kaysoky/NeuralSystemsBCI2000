////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: Adam Wilson
// Description: An ADC class for testing purposes.
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

#include "FilePlaybackADC.h"
#include "BCIError.h"
#include "GenericSignal.h"
#include "MeasurementUnits.h"

#include <cmath>
#ifdef _WIN32
# include <windows.h>
#else
# include <sys/socket.h>
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif // M_PI

using namespace std;

// Register the source class with the framework.
RegisterFilter( FilePlaybackADC, 1 );

FilePlaybackADC::FilePlaybackADC()
: mSamplingRate( 1 ),
mLasttime( 0 ),
mBlockSize(1),
mFileName(""),
mSpeedup(1),
mDataFile(NULL)
{
	BEGIN_PARAMETER_DEFINITIONS
		"Source:Signal%20Properties int SourceCh= 16 "
		"16 1 % // number of digitized and stored channels",
		"Source:Signal%20Properties int SampleBlockSize= 32 "
		"32 1 % // number of samples transmitted at a time",
		"Source:Signal%20Properties int SamplingRate= 256Hz "
		"256Hz 1 % // sample rate",
		"Source:Signal%20Properties intlist SourceChList= 0 0 1 256 "
		"// a list of channels to acquire (empty for all)",
		"Source:Signal%20Properties int Speedup= 1 1 0 100"
		"// a value indicating the amount that the acquisition should be sped up",
		"Source:Signal%20Properties string FileName= % "
		"// the path to the existing BCI2000 data file (inputfile)",
		"Source:Signal%20Properties int SuspendAtEnd= 0 0 0 1 "
		"// suspend execution when the end of the data file is reached (boolean)",
		"Source:Signal%20Properties int StartTime= 0s 0s % % "
			"// the start time of the file",
		END_PARAMETER_DEFINITIONS
		mChList.clear();
}


FilePlaybackADC::~FilePlaybackADC()
{
}


void
FilePlaybackADC::Preflight( const SignalProperties&,
						   SignalProperties& Output ) const
{
	PreflightCondition( Parameter( "SamplingRate" ) > 0 );
	PreflightCondition( Parameter("Speedup") >= 0);
	PreflightCondition(Parameter("SampleBlockSize") > 0);
	Parameter("FileName");
	Parameter("SuspendAtEnd");
	Parameter("StartTime");
	State("Running");
	std::string fname = Parameter("FileName");
	if (fname.length() == 0){
		bcierr << "The length of the FileName must be > 0" << endl;
		return;
	}
	BCI2000FileReader dataFile(fname.c_str());
	switch (dataFile.ErrorState()){
		case (BCI2000FileReader::FileOpenError):
			bcierr << "Error opening file " << fname << endl;
			break;
		case (BCI2000FileReader::MalformedHeader):
			bcierr << "Invalid BCI2000 Data File: " << fname << endl;
			break;
		case (BCI2000FileReader::NumErrors):
			bcierr << "Unknown file error... " << fname << endl;
			break;
	}

	if (int(dataFile.Parameter("SamplingRate")) != int(Parameter("SamplingRate")))
		bcierr << "The SamplingRate in the data file ("<<dataFile.Parameter("SamplingRate")<<") should equal the configured SamplingRate."<<endl;

	//if (int(dataFile.Parameter("SampleBlockSize")) != int(Parameter("SampleBlockSize")))
	//	bcierr << "The SampleBlockSize in the data file ("<<dataFile.Parameter("SampleBlockSize")<<") should equal the configured SampleBlockSize."<<endl;

	if (Parameter("SourceCh") > dataFile.Parameter("SourceCh"))
		bcierr << "The SourceCh value must be less than or equal to the SourceCh value in the data file ("<<dataFile.Parameter("SourceCh")<<")"<<endl;

	if (Parameter("SourceChList")->NumValues() > 0){
		PreflightCondition(Parameter("SourceChList")->NumValues() == Parameter("SourceCh"));
		for (int i = 0; i < Parameter("SourceChList")->NumValues(); i++){
			if (Parameter("SourceChList")(i) > dataFile.SignalProperties().Channels())
				bcierr << "The values in SourceChList must be smaller than the number of channels in the data file (" <<dataFile.SignalProperties().Channels()<<")"<<endl;
		}
	}

	Output = SignalProperties(
		Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), dataFile.SignalProperties().Type() );


}


void
FilePlaybackADC::Initialize( const SignalProperties&, const SignalProperties& )
{
	if (mDataFile != NULL){
		delete mDataFile;
		mDataFile = NULL;
	}
	mSamplingRate = Parameter( "SamplingRate" );
	mBlockSize = Parameter("SampleBlockSize");
	mSpeedup = Parameter("Speedup");
	mFileName = string(Parameter("FileName"));

	mChList.clear();
	if (Parameter("SourceChList")->NumValues() > 0){
		for (int i = 0; i < Parameter("SourceChList")->NumValues(); i++)
			mChList.push_back(Parameter("SourceChList")(i));
	}
	else{
		for (int i = 0; i < Parameter("SourceCh"); i++)
			mChList.push_back(i);
	}
	//use a buffer size of 10s
	int bufsize = 10*mSamplingRate*mChList.size();

	mDataFile = new BCI2000FileReader();
	mDataFile->Open(mFileName.c_str(), bufsize);

	switch (mDataFile->ErrorState()){
		case (BCI2000FileReader::FileOpenError):
			bcierr << "Error opening file " << mFileName << endl;
			return;
			break;
		case (BCI2000FileReader::MalformedHeader):
			bcierr << "Invalid BCI2000 Data File: " << mFileName << endl;
			return;
			break;
		case (BCI2000FileReader::NumErrors):
			bcierr << "Unknown file error... " << mFileName << endl;
			return;
			break;
	}


	mUpdatePeriod = 0;
	if (mSpeedup > 0)
		mUpdatePeriod = 1000.0/float(mSpeedup)*mBlockSize/mSamplingRate;

	mNumSamples = mDataFile->NumSamples();
	mMaxBlock = floor(double(mNumSamples)/double(mBlockSize));
	mStartTime = MeasurementUnits::ReadAsTime(Parameter("StartTime"));
	mCurBlock = mStartTime;
	mSuspendAtEnd = (Parameter("SuspendAtEnd") == 1);
}


void
FilePlaybackADC::StartRun()
{
	mCurBlock = 0;
}


void
FilePlaybackADC::Process( const GenericSignal&, GenericSignal& Output )
{
	int curSample, curCh, nSamples = mDataFile->NumSamples();
	for (int s = 0; s < mBlockSize; s++){
		curSample = mBlockSize*mCurBlock+s;
		if (curSample >= mDataFile->NumSamples())
			break;
		for (int ch = 0; ch < mChList.size(); ch++){
			curCh = mChList[ch];
			Output(ch, s) = mDataFile->RawValue(curCh, curSample);
		}
	}
	mCurBlock++;
	if (mCurBlock >= mMaxBlock){
		if (State("Running") == 1 && mSuspendAtEnd)
			State("Running") = 0;
		else{
			bciout <<"Wrapping data around"<<endl;
			mCurBlock = 0;
		}
	}


	if (mSpeedup == 0)
		return;
	// Wait for the amount of time that corresponds to the length of a data block.
	PrecisionTime now = PrecisionTime::Now();
	//float blockDuration = 1e3 * Output.Elements() / mSamplingRate,
	float time2wait = mUpdatePeriod - ( now - mLasttime );
	if( time2wait < 0 )
		time2wait = 0;
#ifdef _WIN32
	const float timeJitter = 5;
	::Sleep( ::floor( time2wait / timeJitter ) * timeJitter );
	while( PrecisionTime::Now() - mLasttime < mUpdatePeriod - 1 )
		::Sleep( 0 );
#else
	struct timeval tv = { 0, 1e3 * time2wait };
	::select( 0, NULL, NULL, NULL, &tv );
#endif
	mLasttime = PrecisionTime::Now();

}


void
FilePlaybackADC::Halt()
{
	delete mDataFile;
	mDataFile = NULL;
}



