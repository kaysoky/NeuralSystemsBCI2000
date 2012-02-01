////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: Adam Wilson
//
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
////////////////////////////////////////////////////////////////////////////////
#include <algorithm>
#include "analysisClass.h"
//---------------------------------------------------------------------------

typedef signed short int16;
typedef signed int   int32;
typedef float        float32;

#define signal(ch,s) (dFile->CalibratedValue(ch, s))
/*---------------------------
analysis: class constructor
*/
analysis::analysis()
{
    //set pointers to NULL values, and set a few variables to reasonable values
    dFile = NULL;
    //signal = NULL;
    nStates = 0;
    thresh = 0.25;

    //add some default ignoreStates, which may or may not be used eventually...
    ignoreStates.push_back("CursorPosX");
    ignoreStates.push_back("CursorPosY");
    ignoreStates.push_back("Running");
}


analysis::~analysis()
{
    //run the clear function
    clear();
}

/*
open
description: open the dat file, checking if it exists and is valid,
    and determine the type of task
input: string file - the file name to open
        vector<TaskType> - an array of the possible task types from the ini file
output: TaskType - the task type that this function determines the file belongs to
*/
bool analysis::open(string file, Tasks &taskTypes)
{
    //first try to open the file using the BCI2000 file reader function
    fName = file;
	int nTasks = 0;
	thisTask.taskName = "Unknown";

    droppedSamples = 0;
    checkedSamples = 0;

	dFile = new BCI2000FileReader;
	dFile->Open(fName.c_str());

    if (!dFile->IsOpen())
    {
        cout <<"Could not open \""<<fName<<"\" as a BCI2000 data file."<<endl;
        mIsOpen = false;
		thisTask.taskName = "";
		return false;
    }
    mIsOpen = true;

    //get some task parameters
    nSamples = dFile->NumSamples();
    nChannels = dFile->SignalProperties().Channels();
    sampleRate = dFile->SamplingRate();
    SignalType dataType = dFile->SignalProperties().Type();

    //get states -----------------------------
    nStates = dFile->States()->Size();
    for (int i = 0; i < nStates; i++)
    {
        string tmpStr = (*dFile->States())[i].Name();
        states[tmpStr] = new double[nSamples];
        stateNames.push_back(tmpStr);
    }

    const StateVector* stateVector = dFile->StateVector();
    //const StateList& curStateList = stateVector->StateList();

    for (int s = 0; s < nSamples; s++)
    {
        dFile->ReadStateVector(s);
        for (int i = 0; i < nStates; i++)
        {
            states[stateNames[i]][s] = stateVector->StateValue(stateNames[i]);
        }
    }

    //get useful parameters
    //get the data location, whos parameter may be different depending on the version
    //i.e., FileInitials is v1 and DataDirectory is v2
	blockSize = dFile->Parameter("SampleBlockSize");
	if (dFile->Parameters()->Exists("FileInitials"))
		FileInitials = (string)dFile->Parameter("FileInitials");
	else if (dFile->Parameters()->Exists("DataDirectory"))
		FileInitials = (string)dFile->Parameter("DataDirectory");

	SubjectSession = (string)dFile->Parameter("SubjectSession");
	SubjectName = (string)dFile->Parameter("SubjectName");
	SubjectRun = (string)dFile->Parameter("SubjectRun");

    //cleanup
    //delete stateVector; //handled by delete dFile;
    bool statesFound = false;
	bool parmsFound = false;
	bool taskFound = false;
	bool nameFound = false;

    //go through every possible task type, and compare configurations
	for (size_t i = 0; i < taskTypes.size() && !taskFound; i++)
    {
        //search through the task types until one is found
        statesFound = true;
		parmsFound = true;
		nameFound = false;

        //first, check to see if the task folder and subject name are the same
        //if not, we should not continue at all
		nameFound = (taskTypes[i].taskName == SubjectName);

		if (!nameFound)
			continue;

        //if states are specified in the ini file, check to see that those
        //states exist; otherwise, they have no effect on the task classification
        for (size_t s = 0; s < taskTypes[i].states.size(); s++)
        {
            statesFound = (isMember(stateNames, taskTypes[i].states[s]) && statesFound);
		}

        //at some point, we can also add the parm check,
        //but for now it probably is not necessary

        //if the name is found, the states are found (true by def),
        //and the parms are found (true by def), then we have successfuly classified
        //this file as the current task type
        if (statesFound && parmsFound && nameFound)
		{
            //set thisTask as the current taskType, and set the parameterss
			thisTask = (taskTypes[i]);
			thisTask.sampleRate = sampleRate;
			thisTask.blockSize = blockSize;
			taskFound = true;
            nTasks++;
        }
	}
    //if the task is not classified, say so
	if (!taskFound)
    {
		thisTask.taskName = "Unknown";
        return false;
    }

	return true;
}

//clear the file name
bool analysis::close()
{
	fName = "";
	if (dFile != NULL)
		delete dFile;
	dFile = NULL;
    //clear states, which is a map<string, double*>
	for (it = states.begin(); it != states.end(); it++)
    {
        if (it->second != NULL)
        {
            delete []it->second;
            it->second = NULL;
        }
    }
	stateNames.clear();
    return true;
}

/*
clear
description: clear all of the variables/arrays, including the signal, states, and parms
*/
void analysis::clear()
{

	close();

    //clear the statss
    latencyStats.clear();

    droppedSamples = 0;
    checkedSamples = 0;
}

/*-------------------------------------
doThreshAnalysis
description: this is the main analysis function. It does some of the analysis,
    and delegates some responsibility to more specific doThreshAnalysis function
input: TaskType &taskType - the struct that tells the function which analyses to
    perform for this task
output: vector<basicStats> - an array of statistics structures for each analyses
    (e.g., video, audio, etc)
*/
bool analysis::doThreshAnalysis(double threshTmp)
{
	mIgnoreDur = 5*blockSize;

	thresh = threshTmp;
	//reset the latencyStats vector
	latencyStats.clear();

	//if for some reason the file has not been read, return
	if (!mIsOpen)
		return false;

	basicStats ampStats;
    if (thisTask.amp.flag)
    {
        //the amp analysis does not depend on any specific state, so just do the
        //analysis on the specified channel based on the latency from the block start
        ampStats = doThreshAnalysis(thisTask.amp.ch);
        ampStats.taskName = "Amp     ";
        // add the stats to the stats vector
        latencyStats.push_back(ampStats);

        //check for dropped samples on this channel
        checkDroppedSamples(thisTask.amp.ch);
	}

    basicStats dAmpStats;
    if (thisTask.dAmp.flag)
    {
        //the amp analysis does not depend on any specific state, so just do the
        //analysis on the specified channel based on the latency from the block start
        dAmpStats = doThreshAnalysis(thisTask.dAmp.ch);
        dAmpStats.taskName = "DigAmp  ";
        // add the stats to the stats vector
        latencyStats.push_back(dAmpStats);

        //check for dropped samples on this channel
        //checkDroppedSamples(thisTask.dAmp.ch);
    }

    //do the analysis on each state for the video
    basicStats vidSystemStats;
    if (thisTask.vid.flag)
    {
        //do the video analysis on the channel and state specified
        vidSystemStats = doThreshAnalysis(thisTask.vid.ch, thisTask.vid.state, thisTask.vid.stateVal);
        vidSystemStats.taskName = "VidSys  ";
        latencyStats.push_back(vidSystemStats);

        checkDroppedSamples(thisTask.vid.ch);
    }

    //do the analysis on each state for the audio
    basicStats audSystemStats;
    if (thisTask.aud.flag)
    {
        audSystemStats = doThreshAnalysis(thisTask.aud.ch, thisTask.aud.state, thisTask.aud.stateVal);
        audSystemStats.taskName = "AudSys  ";
        latencyStats.push_back(audSystemStats);

        checkDroppedSamples(thisTask.aud.ch);
    }

	//do the processing latency and metronome
    //first, get the "metronome" stats, the difference in time between each block source time
    map<string, double*>::iterator it; //an iterator for the states map

    //declare double vectors for the metronome and processing latencies
    vector<double> metronomeDiff;
    vector<double> procLat;

    //declare variables to track previous values of the stim and source times
	//double stimT, oldStimT, sourceT, oldSourceT, prevSourceT;

    //get the first values for stim/source times
	//oldStimT =  states["StimulusTime"][0];
	//oldSourceT = prevSourceT = states["SourceTime"][0];
	//float nMod1 = 0, nMod2 = 0;
    //loop through the states, starting at the 2nd block, and increase
	//by the block size
	unsigned short t2, t1, tans;
	for (int i = mIgnoreDur; i < nSamples; i += blockSize)
	{
		t2 = states["StimulusTime"][i];
		t1 = states["SourceTime"][i];
		tans = t2-t1;
		procLat.push_back(tans);

		t2 = states["SourceTime"][i];
		t1 = states["SourceTime"][i-blockSize];
		tans = t2-t1;
		metronomeDiff.push_back(tans);
    }
	for (int i = 0; i < metronomeDiff.size(); i++)
		metronomeDiff[i] = 100*((blockSize*1000/sampleRate) - metronomeDiff[i])/((blockSize*1000/sampleRate));

    //calculate the stats for the processing latency using our vector helper functions
	basicStats procStats;
	procStats.mean = vMean(&procLat);
	procStats.std = vStd(&procLat);
	procStats.min = vMin(&procLat);
	procStats.max = vMax(&procLat);
	procStats.median = vMedian(&procLat);
    if (thisTask.exportData)
        procStats.vals = procLat;

    procStats.taskName = "ProcLat ";
    procStats.desc = "StimulusTime - SourceTime";

    //...now do the same for the metronome
    basicStats metronome;
	metronome.mean = vMean(&metronomeDiff);
	metronome.median = vMedian(&metronomeDiff);
	metronome.max = vMax(&metronomeDiff);
	metronome.min = vMin(&metronomeDiff);
	metronome.std = vStd(&metronomeDiff);
    if (thisTask.exportData)
        metronome.vals = metronomeDiff;

    metronome.taskName = "Jitter  ";
    metronome.desc = "SourceTime[t+1] - SourceTime[t]";

    //add these to the task stats array
    latencyStats.push_back(metronome);
    latencyStats.push_back(procStats);

    //if we did the amp and video latencies, then we can also calculate the
    //video system latency (actually, the video latency calculate above is the
    //video system latency, so we use that to find the output latency as:
    // vidOut = vidSys - amp - procLat)
    if (thisTask.amp.flag && thisTask.vid.flag && ampStats.mean != -1 && vidSystemStats.mean != -1)
    {
		basicStats vidOutputStats;
		double spMean = vMean(&vidSystemStats.sigProc);

		vidOutputStats.vals.insert(vidOutputStats.vals.begin(),
									vidSystemStats.vals.begin(),
									vidSystemStats.vals.end());
		for (size_t i = 0; i < vidOutputStats.vals.size(); i++)
			vidOutputStats.vals[i] -= (spMean + ampStats.mean);

		vidOutputStats.mean = vMean(&vidOutputStats.vals);
		vidOutputStats.std = vStd(&vidOutputStats.vals);
		vidOutputStats.min = vMin(&vidOutputStats.vals);
		vidOutputStats.max = vMax(&vidOutputStats.vals);
		vidOutputStats.median = vMedian(&vidOutputStats.vals);
        vidOutputStats.taskName = "VidOut  ";
        vidOutputStats.desc = "Video output latency";
        latencyStats.push_back(vidOutputStats);
    }

    //same as the video stats above
    if (thisTask.amp.flag && thisTask.aud.flag && ampStats.mean != -1 && audSystemStats.mean != -1)
    {
		basicStats audOutputStats;
		double spMean = vMean(&audSystemStats.sigProc);
		audOutputStats.vals.insert(audOutputStats.vals.begin(),
									audSystemStats.vals.begin(),
									audSystemStats.vals.end());
		for (size_t i = 0; i < audOutputStats.vals.size(); i++)
			audOutputStats.vals[i] -= (spMean + ampStats.mean);

		audOutputStats.mean = vMean(&audOutputStats.vals);
		audOutputStats.std = vStd(&audOutputStats.vals);
		audOutputStats.min = vMin(&audOutputStats.vals);
		audOutputStats.max = vMax(&audOutputStats.vals);
		audOutputStats.median = vMedian(&audOutputStats.vals);

        if (thisTask.exportData)
            audOutputStats.vals.clear();
        audOutputStats.taskName = "AudOut  ";
        audOutputStats.desc = "Audio output latency";
        latencyStats.push_back(audOutputStats);
    }

	//return our stats
    return true;
}

/*------------------------------
doThreshAnalysis
description: do the threshold analysis using the specified channel, and using the
    sample blocks as the "trigger"
input: int chNum- the channel in the signal to use
output: basicStats - the stats for this channel
*/
basicStats analysis::doThreshAnalysis(int chNum)
{
	vector<double> tDiff;
	basicStats tmpStat;
	tmpStat.mean = -1;
	tmpStat.median = -1;
	tmpStat.std = -1;
	tmpStat.min = -1;
	tmpStat.max = -1;

	if (chNum >= nChannels || chNum < 0)
		return tmpStat;

	double *dSig = new double[nSamples-1];
	double dMean = 0;
	for (int s = 0; s < nSamples-1; s++){
		dSig[s] = signal(chNum, s+1) - signal(chNum, s);
		dMean += dSig[s];
	}
	dMean /= (nSamples-1);
	double std = 0;
	for (int s = 0; s < nSamples-1; s++)
		std += (dSig[s] - dMean)*(dSig[s] - dMean);
	std = sqrt(std/(nSamples-1));


	for (size_t s = mIgnoreDur; s < nSamples-1; s++){
		if (dSig[s] >= std){
			tDiff.push_back(1000*((s+1) % blockSize)/sampleRate);
			s += blockSize/3;
		}
	}
    delete [] dSig;
	if (tDiff.size() == 0)
		return tmpStat;

    //find the mean, std, etc
    if (tDiff.size() > 0)
    {
        tmpStat.mean = vMean(&tDiff);
        tmpStat.std = vStd(&tDiff);
        tmpStat.min = vMin(&tDiff);
		tmpStat.max = vMax(&tDiff);
		tmpStat.median = vMedian(&tDiff);
        tmpStat.vals = tDiff;
    }

    stringstream str;
    str << "Ch " << chNum <<" v. blocks";
    tmpStat.desc = str.str();

    return tmpStat;
}

/*------------------------------
doThreshAnalysis
description: this version of the analysis uses a specified recorded channel,
    and triggers off of a state when that state changes to a given value
input: int chNum - the channel to use in the analysis
        string stateName - the state name to use for triggering
        int stateVal - the value that the state must change to to trigger
            (0 triggers ANY positive state change)
output: basicStats - the results of the analysis
*/
basicStats analysis::doThreshAnalysis(int chNum, string stateName, vector<int> stateVal)
{
	vector<double> tDiff;
	basicStats tmpStat;
	tmpStat.mean = -1;
	tmpStat.median = -1;
	tmpStat.std = -1;
	tmpStat.min = -1;
	tmpStat.max = -1;

    //normalize the data by subtracting the minimum value, and dividing by the max
    //giving us values between 0 and 1
	if (chNum >= nChannels || chNum < 0)
		return tmpStat;

	map<string, double*>::iterator itr = states.find(stateName);
	if (itr == states.end())
		return tmpStat;

	// get the 1st deriv of the signal and its std dev
	vector<int> signalChangePos;
	double *dSig = new double[nSamples-1];
	double dMean = 0;
	for (int s = 0; s < nSamples-1; s++){
		dSig[s] = signal(chNum, s+1) - signal(chNum, s);
		dMean += dSig[s];
	}
	dMean /= (nSamples-1);
	double std = 0;
	for (int s = 0; s < nSamples-1; s++)
		std += (dSig[s] - dMean)*(dSig[s] - dMean);
	std = sqrt(std/(nSamples-1));

	for (size_t s = 0; s < nSamples-1; s++){
		if (dSig[s] > std)
			signalChangePos.push_back(s);
	}

	// find all state changes for all specified state values
	vector<int> stateChangePos;
	for (int s = mIgnoreDur; s < nSamples; s++){
		double dState = states[stateName][s] - states[stateName][s-1];
		if (dState == 0) continue;
		// check if the state change is a value we are looking for
		if (dState > 0 && stateVal[0] == 0){
			stateChangePos.push_back(s+1);
            continue;
		}
		for (size_t p = 0; p < stateVal.size(); p++){
			if (stateVal[p] == dState){
				stateChangePos.push_back(s);
				continue;
			}
		}
	}
    delete [] dSig;
	// now find all values
	if (stateChangePos.size() == 0 || signalChangePos.size() == 0){
		return tmpStat;
	}

	size_t lastPos = 0;
	for (size_t i = 0; i < stateChangePos.size()-1; i++){
		for (size_t j = lastPos; j < signalChangePos.size(); j++){
			if (signalChangePos[j] >= stateChangePos[i] && signalChangePos[j] < stateChangePos[i+1]){
				lastPos = j+1;
				double sigProc = states["StimulusTime"][stateChangePos[i]] - states["SourceTime"][stateChangePos[i]];
				tmpStat.sigProc.push_back(sigProc);
				tDiff.push_back(1000*(signalChangePos[j] - stateChangePos[i])/sampleRate);
				unsigned short t1 = states["SourceTime"][stateChangePos[i]];
				unsigned short t2 = states["SourceTime"][stateChangePos[i]-blockSize];
				tmpStat.jitter.push_back(t1-t2 );
			}
			if (signalChangePos[j] > stateChangePos[i+i]) j = signalChangePos.size();
		}
	}

	if (tDiff.size() == 0)
		return tmpStat;

    //find the mean, std, etc
    if (tDiff.size() > 0)
	{
		tmpStat.mean = vMean(&tDiff);
		tmpStat.std = vStd(&tDiff);
		tmpStat.min = vMin(&tDiff);
		tmpStat.max = vMax(&tDiff);
		tmpStat.median = vMedian(&tDiff);
		tmpStat.vals = tDiff;
	}

	stringstream str;
    str << "Ch " << chNum <<" v. " << stateName;
	if (stateVal[0] > 0){
		str <<"(";
		for (size_t k = 0; k < stateVal.size(); k++){
			if (k < stateVal.size() -1)
				str << stateVal[k] << " || ";
			else
				str << stateVal[k];
			str << ")" <<endl;
		}
	}

    tmpStat.desc = str.str();

    return tmpStat;
}


/*
checks for dropped samples, by either checking for consecutive zeros, or
values outside a reasonable range
*/
void analysis::checkDroppedSamples(int ch)
{
    if (ch >= nChannels || ch < 0)
        return;

    checkedSamples += nSamples;
    double prevVal = signal(ch, 0);
    for (int s = 1; s < nSamples; s++)
    {
        if (signal(ch, s) == 0 && prevVal == 0)
            droppedSamples++;

        if (abs(signal(ch, s)) >= 1e7) // 10 million uV, or 10 V
            droppedSamples++;

        prevVal = signal(ch, s);
  }
}

void analysis::checkDroppedSamples()
{
    double prevVal;
    droppedSamples = 0;
    for (int ch = 0; ch < nChannels; ch++)
    {
        prevVal = signal(ch, 0);
        for (int s = 1; s < nSamples; s++)
        {
            if (signal(ch, s) == 0 && prevVal == 0)
                droppedSamples++;

            if (abs(signal(ch, s)) >= 1e7) // 10 million uV, or 10 V
                droppedSamples++;

            prevVal = signal(ch, s);
        }
    }
}

/*
From here, these are all helper functions to find the mean, min, max etc of
the vector<double> arrays
*/



bool analysis::print(FILE * out, vector<basicStats*> minReqs, int num)
{
	fprintf(out, "[%d] %s\tDuration=%1.2fs\n",num, shortFname(fName).c_str(), float(this->nSamples)/this->sampleRate);
    //resOut << std::endl<<fName<<":"<<std::endl;
    bool ok = true;
	for (size_t t = 0; t < latencyStats.size(); t++)
    {
        basicStats tmpTask = latencyStats[t];
        fprintf(out, "\t%s: ",(tmpTask.taskName).c_str());
        //resOut<<tmpTask.taskName<<": ";
        int tfound = 0;

        //compare this against the min requirements by searching through the minreqs array
        //and checking if the names match; if they do, see if the mean and stddev are within the bounds

		for (size_t a = 0; a < minReqs.size(); a++)
        {
			if (tolower(strtrim(tmpTask.taskName)) == tolower(strtrim(minReqs[a]->taskName)))
            {
                tfound = 1;
				if (tmpTask.mean <= (minReqs[a]->mean)){
					if (tolower(strtrim(tmpTask.taskName)) == "jitter" ||
							tmpTask.mean >= 0){
						fprintf(out, "%s\t\t","pass");
					}

				}
                else if (tmpTask.mean == -1)
                {
                    tfound = 2;
                    fprintf(out, "%s\t","no stimuli");
                    continue;
                }
                else
                {
                    fprintf(out, "%s\t\t","fail");
                    ok = false;
                }
            }
        }

        if (tfound == 2){
            fprintf(out, "%\n");
            continue;
        }
        //if the min requirement for this analysis was not defined, say so, otherwise output the results
        if (tfound == 0)
            fprintf(out, "%s\n","task requirements undefined\n");
        else
        {
            fprintf(out, "%1.2f\t%1.2f\t%1.2f\t%1.2f\t%1.2f\n",tmpTask.mean, tmpTask.median,tmpTask.std, tmpTask.min, tmpTask.max);
        }
    }

    //fprintf(out,"\tDropped Samples\t\t%d/%d\n",droppedSamples, checkedSamples);
    //say whether this file passed the requirements
    if (ok)
    {
        fprintf(out, "\tOverall\tPASSED\n");
        cout<<fName<< " passed the minimum BCI2000 requirements."<<endl;
    }
    else
    {
        fprintf(out, "\tOverall\tFAILED\n");
        cout<<fName<< " did not pass the minimum BCI2000 requirements."<<endl;
    }
    fprintf(out,"\n\n");
	return ok;
}


bool analysis::exportData(string expfile)
{
    ofstream o;
	o.open(expfile.c_str(), ios::trunc | ios::out);
    if (!o.is_open())
    {
        cout << "Error opening data file " <<expfile <<endl;
        return false;
    }

	for (size_t i = 0; i < latencyStats.size(); i++)
    {
        basicStats tmpTask = latencyStats[i];
        if (tmpTask.vals.size() == 0)
            continue;
        o<<tmpTask.taskName;
        o.precision(4);
        for (size_t s = 0; s < tmpTask.vals.size(); s++)
            o << ", " << tmpTask.vals[s];
        o << endl;
    }

    o.close();
    return true;
}
