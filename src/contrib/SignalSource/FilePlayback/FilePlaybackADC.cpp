////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: Adam Wilson, Jeremy Hill
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
#include "OSThread.h"

#include <cmath>

using namespace std;

// Register the source class with the framework.
RegisterFilter( FilePlaybackADC, 1 );

FilePlaybackADC::FilePlaybackADC()
: mSamplingRate( 1 ),
mLasttime( 0 ),
mBlockSize(1),
mFileName(""),
mTemplateFileName(""),
mSpeedup(1),
mDataFile(NULL)
{
	mTemplateFileName = OptionalParameter("PlaybackFileName", "");
	if( mTemplateFileName.size() ) // --PlaybackFileName was given as a command-line option. Use this opportunity to declare all the parameters and states that are in the file, giving the file's parameter values as defaults (with a few exceptions, below).
	{
		OSThread::Sleep( 2000 );  // If we connect to the operator after the other modules, Parameter declarations from the other modules will be overwritten by ours, which is what we want.
		                          // (TODO:  is there any way of making this more certain? i.e. detecting whether other modules have connected yet rather than just hoping 2 sec will be enough?)
		                          // NB by contrast, parameters already declared by *this* module will not be overwritten by a second BEGIN_PARAMETER_DEFINITIONS:  we will actually need to detect this case and update the parameter
		BCI2000FileReader dataFile( mTemplateFileName.c_str() );
		CheckFile( mTemplateFileName, dataFile );
		ParamList *already = this->Parameters;
		const ParamList * pl = dataFile.Parameters();
		for(int i = 0; i < pl->Size(); i++)
		{
			const Param p = (*pl)[i];

			bool skipParam = ( p.Section() == "System" ); // skip anything in the "System" tab
			const HierarchicalLabel& hl = p.Sections();
			if( hl.size() >= 3 )
			{
				const string name = p.Name();
				const string blame = hl[2];
				if( blame.size() > 6 && blame.substr( blame.size()-6 ) == "Logger" )
					if( name.size() > 3 && name.substr( 0, 3 ) == "Log" )
						skipParam = true; // skip anything whose name begins with "Log" if it was declared by a class whose name ends in "Logger".
				                          // Thus LogEyetracker from the EyetrackerLogger is skipped, LogMouse from the KeyLogger is skipped, etc.
				                          // Thus if the original had --LogMouse=1,     but playback does not,         and PlaybackStates is 1, the original logged MousePosX and MousePosY are simply played back (very nice)
				                          //      if the original had --LogMouse=1,     and playback has --LogMouse=1, and PlaybackStates is 0, the new mouse movements (during playback) are logged (also potentially useful)
				                          //      if the original had --LogMouse=1,     and playback has --LogMouse=1, and PlaybackStates is 1, there will be havoc: played-back and logged states will conflict.
				                          //      if the original had no mouse logging, and playback has --LogMouse=1, the new mouse movements (during playback) are logged (and it's OK even if PlaybackStates is 1, because the original had no MousePosX and MousePosY states to play back)
			}

			if( skipParam )
			{
				//bciout << "Skipping " << p.Name() << endl;
			}
			else if( already->Exists( p.Name() ) )
			{
				//bciout << "Updating " << p << endl;
				(*already)[p.Name()] = p;
			}
			else
			{
				//bciout << "Setting " << p << endl;
				stringstream ss;
				ss << p;
				string s = ss.str();
				BEGIN_PARAMETER_DEFINITIONS
					s.c_str(),
				END_PARAMETER_DEFINITIONS
			}
		}
		const StateList *sl = dataFile.States();
		for( int i = 0; i < sl->Size(); i++ )
		{
			stringstream ss;
			ss << (*sl)[i];
			string s = ss.str();
			BEGIN_STATE_DEFINITIONS
				s.c_str(),
			END_STATE_DEFINITIONS
		}
	}
	else // declare the bare minimum of source module parameters
	{
		BEGIN_PARAMETER_DEFINITIONS
			"Source:Signal%20Properties int SourceCh= 16 "
			" 16 1 % // number of digitized and stored channels",
			
			"Source:Signal%20Properties int SampleBlockSize= 32 "
			" 32 1 % // number of samples transmitted at a time",
			
			"Source:Signal%20Properties int SamplingRate= 256Hz "
			" 256Hz 1 % // sample rate",

		END_PARAMETER_DEFINITIONS
	}

	// declare additional playback-specific parameters
	BEGIN_PARAMETER_DEFINITIONS
		"Source:Playback string PlaybackFileName= % " // puts the OptionalParameter in context within the Config dialog (and allows it to be used from here)
		" % % % // the path to the existing BCI2000 data file (inputfile)",
			
		"Source:Playback int PlaybackStartTime= 0s "
		" 0s % % // the start time of the file",

		"Source:Playback list PlaybackChList= 0 "
		" % % % // a list of channels to acquire (empty for all). Use indices, or labels from the ChannelNames as they were recorded in the file.",
		
		"Source:Playback float PlaybackSpeed= 1 "
		" 1 0 100 // a value indicating the factor by which the acquisition should be sped up",
				
		"Source:Playback int PlaybackStates= 0 "
		" 0 0 1 // play back state variable values (except timestamps)? (boolean)",

		"Source:Playback int PlaybackLooped= 0 "
		" 0 0 1 // loop playback at the end of the data file instead of suspending execution (boolean)",

	END_PARAMETER_DEFINITIONS
	mChList.clear();
}


FilePlaybackADC::~FilePlaybackADC()
{
	Halt();
}


void
FilePlaybackADC::CheckFile( string& fname, BCI2000FileReader& dataFile ) const
{
	switch( dataFile.ErrorState() ){
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
}

void
FilePlaybackADC::Preflight( const SignalProperties&,
						   SignalProperties& Output ) const
{
	PreflightCondition( Parameter( "SamplingRate" ) > 0 );
	PreflightCondition( Parameter("PlaybackSpeed") >= 0.0f );
	PreflightCondition(Parameter("SampleBlockSize") > 0);
	Parameter("PlaybackLooped");
	Parameter("PlaybackStartTime");
	State("Running");
	std::string fname = Parameter("PlaybackFileName");
	if (fname.length() == 0){
		bcierr << "PlaybackFileName cannot be empty" << endl;
		return;
	}
	if( mTemplateFileName.length() && fname != mTemplateFileName )
		bciout << "file " << fname << " is now being played back. Be aware that the playback module was launched with a different file, " << mTemplateFileName << " as the template for parameter and state definitions." << endl;

	BCI2000FileReader dataFile(fname.c_str());
	CheckFile( fname, dataFile );

	if (int(dataFile.Parameter("SamplingRate")) != int(Parameter("SamplingRate")))
		bcierr << "The SamplingRate in the data file ("<<dataFile.Parameter("SamplingRate")<<") should equal the configured SamplingRate."<<endl;

	//if (int(dataFile.Parameter("SampleBlockSize")) != int(Parameter("SampleBlockSize")))
	//	bcierr << "The SampleBlockSize in the data file ("<<dataFile.Parameter("SampleBlockSize")<<") should equal the configured SampleBlockSize."<<endl;
	if (Parameter("SourceCh") > dataFile.Parameter("SourceCh"))
		bcierr << "The SourceCh value must be less than or equal to the SourceCh value in the data file ("<<dataFile.Parameter("SourceCh")<<")"<<endl;

	vector<int> dummyChList;
	MatchChannels( dataFile, dummyChList );

	if( (int)Parameter( "PlaybackStates" ) )
	{
		const StateList* sl = dataFile.States();
		for( int i = 0; i < sl->Size(); i++ )
			State( (*sl)[i].Name() );
		
	}
	Output = SignalProperties(
		Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), dataFile.SignalProperties().Type() );


}


void
FilePlaybackADC::MatchChannels( const BCI2000FileReader& inDataFile, vector<int>& inChList ) const
{
	inChList.clear();
	ParamRef playbackChList = Parameter("PlaybackChList");
	const int sourceCh = Parameter("SourceCh");
	if( playbackChList->NumValues() == 0 )
	{
		for( int i = 0; i < sourceCh; i++ )
			inChList.push_back( i );
		return;
	}
	if( playbackChList->NumValues() != sourceCh )
	{
		bcierr << "number of elements of PlaybackChList must match the value of SourceCh" << endl;
		return;
	}
	// From here on, we definitely have a PlaybackChList parameter of the correct length, which we need to deal with
	vector<string> fileChannelNames, numericStrings;
	const ParamRef fileChannelNamesParam = inDataFile.Parameter("ChannelNames");
	const int numberOfChannelsInFile = inDataFile.SignalProperties().Channels();
	if( fileChannelNamesParam->NumValues() == numberOfChannelsInFile )
		for( int i = 0; i < numberOfChannelsInFile; i++ )
			fileChannelNames.push_back( fileChannelNamesParam( i ) );
	for( int i = 0; i < numberOfChannelsInFile; i++ )
	{
		stringstream ss;
		ss << (i+1);
		numericStrings.push_back( ss.str() );
	}

	for( int i = 0; i < sourceCh; i++ )
	{
		string chName = playbackChList(i);
		unsigned int channelIndex;
		bool found = false;
		
		if( !found )
		{
			for( channelIndex = 0; channelIndex < fileChannelNames.size(); channelIndex++ )
				if( fileChannelNames[channelIndex] == chName ) { found = true; break; }
			//if( found ) bciout << "matched " << chName << " to " << channelIndex+1 << " via file's ChannelNames" << endl;
		}
		if( !found )
		{
			for( channelIndex = 0; channelIndex < numericStrings.size(); channelIndex++ )
				if( numericStrings[channelIndex] == chName )  { found = true; break; }
			//if( found ) bciout << "matched " << chName << " to " << channelIndex+1 << " as a number" << endl;
		}
		if( found )
			inChList.push_back( channelIndex );
		else
			bcierr << "could not match entry \"" << chName << "\" of PlaybackChList as a legal channel in the file (file has " << numberOfChannelsInFile << " data channels, and " << fileChannelNamesParam->NumValues() << " entries in its ChannelNames parameter)" << endl;
	}
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
	mSpeedup = Parameter("PlaybackSpeed");
	mFileName = string(Parameter("PlaybackFileName"));

	//use a buffer size of 10s
	int bufsize = (int)::ceil( 10*mSamplingRate*Parameter("SourceCh") );

	mDataFile = new BCI2000FileReader();
	mDataFile->Open(mFileName.c_str(), bufsize);
	CheckFile( mFileName, *mDataFile );
	MatchChannels( *mDataFile, mChList );
	mUpdatePeriod = 0;
	if (mSpeedup > 0)
		mUpdatePeriod = 1000.0f/float(mSpeedup)*mBlockSize/mSamplingRate;

	mNumSamples = mDataFile->NumSamples();
	mMaxBlock = (int)floor( double(mNumSamples)/double(mBlockSize) );
	double startTime = Parameter("PlaybackStartTime").InSampleBlocks();
	mStartTime = (int)floor( startTime );
	if( startTime != (float)mStartTime ) bciout << "PlaybackStartTime " << (string)Parameter( "PlaybackStartTime" ) << " has been rounded down to a whole number of SampleBlocks (" << mStartTime << ")" << endl;
	mCurBlock = mStartTime;
	mSuspendAtEnd = (Parameter("PlaybackLooped") == 0);
	
	mStateMappings.clear();
	if( Parameter("PlaybackStates") != 0 )
	{
		StateList* statesHere = States;
		const StateList* statesFile = mDataFile->States();
		for( int i = 0; i < statesHere->Size(); i++ )
		{
			class State& srh = (*statesHere)[i];
			const string statename = srh.Name();
			if( statename != "SourceTime" && statename != "StimulusTime" && statename != "Running" && statename != "Recording" && statesFile->Exists(statename) )
			{
				unsigned int indf = statesFile->Index(statename);
				const class State& srf = (*statesFile)[indf];
				mStateMappings.push_back(StateMapping( srf.Location(), srf.Length(), srh.Location(), srh.Length() ));
				//bciout << "Will play back " << statename << " from (" << indf << "," << srf.Location() << "," << srf.Length() << ")" << " to (" << i << "," << srh.Location() << "," << srh.Length() << ")" << endl;
			}
		}
	}
}


void
FilePlaybackADC::StartRun()
{
	mCurBlock = 0;
	for( unsigned int i = 0; i < mStateMappings.size(); i++ ) mStateMappings[i].Reset();
}


void
FilePlaybackADC::Process( const GenericSignal&, GenericSignal& Output )
{
	int curCh;
	long long curSample;
	long long nSamples = mDataFile->NumSamples();
	bool playStates = ( mStateMappings.size() != 0 && State("Running") != 0 );
	for (int el = 0; el < mBlockSize; el++){
		curSample = mBlockSize * mCurBlock + el;
		if (curSample >= mDataFile->NumSamples())
			break;
		for (unsigned int ch = 0; ch < mChList.size(); ch++){
			curCh = mChList[ch];
			Output(ch, el) = mDataFile->RawValue(curCh, curSample);
		}
		if( playStates )
		{
			mDataFile->ReadStateVector( curSample );
			const StateVector* sv = mDataFile->StateVector();
			for( unsigned int i = 0; i < mStateMappings.size(); i++ )
				mStateMappings[i].Copy( sv, 0, Statevector, el );			
		}
	}
	mCurBlock++;
	if (mSuspendAtEnd && mCurBlock >= mMaxBlock-1 && State("Running")==1)
		State("Running") = 0;
	else if( mCurBlock >= mMaxBlock )
	{
		bciout <<"Wrapping data around"<<endl;
		mCurBlock = 0;
	}


	if (mSpeedup == 0)
		return;
	// Wait for the amount of time that corresponds to the length of a data block.
	PrecisionTime now = PrecisionTime::Now();
	//float blockDuration = 1e3 * Output.Elements() / mSamplingRate,
	float time2wait = mUpdatePeriod - ( now - mLasttime );
	if( time2wait < 0 )
		time2wait = 0;

	const float timeJitter = 5;
	OSThread::Sleep( (int)( ::floor( time2wait / timeJitter ) * timeJitter ) );
	while( PrecisionTime::Now() - mLasttime < mUpdatePeriod - 1 )
		OSThread::Sleep( 0 );
	mLasttime = PrecisionTime::Now();

}


void
FilePlaybackADC::Halt()
{
	delete mDataFile;
	mDataFile = NULL;
}



