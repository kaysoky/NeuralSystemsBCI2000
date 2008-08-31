/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include "analysisClass.h"
#include "PrecisionTime.h"
//---------------------------------------------------------------------------

typedef signed short int16;
typedef signed int   int32;
typedef float        float32;

//#define signal(ch,s) (dFile->CalibratedValue(ch, s))
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
	dFile->Open(fName.c_str());//, 1000000000);

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

    //get the source channel gain in mV, not uV
	/*double *sourceChGain;
	sourceChGain = new double[nChannels];
	for (int i = 0; i < nChannels; i++)
		sourceChGain[i] = dFile->Parameter("SourceChGain")(0,i)/1000;
	*/
	//read in the signal -----------------------
	//unsigned short t1 = PrecisionTime::Now();
	signal = new double *[nChannels];
	for (int ch = 0; ch < nChannels; ch++)
		signal[ch] = new double[nSamples];
	for (int s = 0; s < nSamples; s++)
	{
        for (int ch = 0; ch < nChannels; ch++)
		{
			signal[ch][s] = dFile->RawValue(ch, s);
        }
	}
    //delete [] sourceChGain;
	//cout <<"Read time= "<<PrecisionTime::TimeDiff(t1, PrecisionTime::Now()) <<" ms"<<endl;
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
		FileInitials = dFile->Parameter("FileInitials");
	else if (dFile->Parameters()->Exists("DataDirectory"))
		FileInitials = dFile->Parameter("DataDirectory");

	SubjectSession = dFile->Parameter("SubjectSession");
	SubjectName = dFile->Parameter("SubjectName");
	SubjectRun = dFile->Parameter("SubjectRun");

    //cleanup
    //delete stateVector; //handled by delete dFile;
    bool statesFound = false;
	bool parmsFound = false;
	bool taskFound = false;
	bool nameFound = false;

    //go through every possible task type, and compare configurations
	for (int i = 0; i < taskTypes.size() && !taskFound; i++)
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
        for (int s = 0; s < taskTypes[i].states.size(); s++)
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
    return true;
}

/*
clear
description: clear all of the variables/arrays, including the signal, states, and parms
*/
void analysis::clear()
{
	//free pointer memory for the signal, which is a double**
    if (signal != NULL)
        {
        for (int i = 0; i < nChannels; i++)
        {
            if (signal[i] != NULL)
            {
                delete[] signal[i];
                signal[i] = NULL;
            }
        }
        delete[] signal;
        signal = NULL;
    }
                 
    //clear states, which is a map<string, double*>
    for (it = states.begin(); it != states.end(); it++)
    {
        if (it->second != NULL)
        {
            delete []it->second;
            it->second = NULL;
        }
    }

    //delete the file pointer
    if (dFile != NULL)
        delete dFile;
    dFile = NULL;

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
    thresh = threshTmp;
    //reset the latencyStats vector
    latencyStats.clear();

    //if for some reason the file has not been read, return
    if (!mIsOpen)
        return false;

    //first, check dropped samples on all channels
    //checkDroppedSamples();
    
    //do the amp latency analysis
    //the task type struct has a flag for each possible analysis, which tells if
    //we should perform the analysis
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
	double stimT, oldStimT, sourceT, oldSourceT, prevSourceT;

    //get the first values for stim/source times
	oldStimT =  states["StimulusTime"][0];
	oldSourceT = prevSourceT = states["SourceTime"][0];
	float nMod1 = 0, nMod2 = 0;
    //loop through the states, starting at the 2nd block, and increase
    //by the block size
	for (int i = blockSize; i < nSamples; i += blockSize)
	{
		stimT = states["StimulusTime"][i];
        sourceT = states["SourceTime"][i];

        //nMod1 and nMod2 track the times, which are short (16bit) integers
        //if the new stim is less than the oldStim, we have wrapped around
        //and so we must correct for this
        if (stimT-oldStimT < 0)
            nMod1++;
        if (sourceT-oldSourceT < 0)
            nMod2++;

        //the processing latency is the difference between the stimulus time and
        //source time; if we have wrapped the times, the nModX variable corrects this
		double procDT = ((stimT+nMod1*65535) - (sourceT+nMod2*65535));

        //the jitter/metronome is the difference between the current sourceTime
        //minus the previous source time
        //additionally, we are putting this in relation to the "theoretical" value,
        //which is the blockSize, and converting this to ms
		double jitter = abs(((sourceT+nMod2*65535)-prevSourceT) - blockSize/sampleRate*1000);

        //add the jitter and processing latencies to their respective arrays
		metronomeDiff.push_back(jitter);
        procLat.push_back(procDT);

        //update stim and source times
        oldStimT = stimT;
        oldSourceT = sourceT;
        prevSourceT = (sourceT+nMod2*65535);
    }

    //calculate the stats for the processing latency using our vector helper functions
	basicStats procStats;
	procStats.mean = vMean(&procLat);
	procStats.std = vStd(&procLat);
	procStats.min = vMin(&procLat);
	procStats.max = vMax(&procLat);
	procStats.max = vMedian(&procLat);
    if (thisTask.exportData)
        procStats.vals = procLat;
        
    procStats.taskName = "ProcLat ";
    procStats.desc = "StimulusTime - SourceTime";

    //...now do the same for the metronome
    basicStats metronome;
	metronome.mean = vMean(&metronomeDiff);
	metronome.max = vMax(&metronomeDiff);
	metronome.min = vMin(&metronomeDiff);
	metronome.std = vStd(&metronomeDiff);
	metronome.median = vMedian(&metronomeDiff);
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
        vidOutputStats.mean = vidSystemStats.mean - ampStats.mean - procStats.mean;
		vidOutputStats.std = sqrt(pow(vidSystemStats.std,2) + pow(ampStats.std,2) + pow(procStats.std,2));
        vidOutputStats.min = 0;//vidSystemStats.min - ampStats.min - procStats.min;
		vidOutputStats.max = 0;//vidSystemStats.max - ampStats.max - procStats.max;
		vidOutputStats.median = 0;//vidSystemStats.max - ampStats.max - procStats.max;
        if (thisTask.exportData)
            vidOutputStats.vals.clear();
        vidOutputStats.taskName = "VidOut  ";
        vidOutputStats.desc = "Video output latency";
        latencyStats.push_back(vidOutputStats);
    }

    //same as the video stats above
    if (thisTask.amp.flag && thisTask.aud.flag && ampStats.mean != -1 && audSystemStats.mean != -1)
    {
        basicStats audOutputStats;
        audOutputStats.mean = audSystemStats.mean - ampStats.mean - procStats.mean;
		audOutputStats.std = sqrt(pow(audSystemStats.std,2) + pow(ampStats.std,2) + pow(procStats.std,2));
        audOutputStats.min = 0;//audSystemStats.min - ampStats.min - procStats.min;
		audOutputStats.max = 0;//audSystemStats.max - ampStats.max - procStats.max;
		audOutputStats.median = 0;
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
	tmpStat.std = -1;
	tmpStat.min = -1;
	tmpStat.max = -1;
	tmpStat.median = -1;

	if (chNum >= nChannels || chNum < 0)
		return tmpStat;

    /*  Data is no longer being normalized!
    //normalize the data by subtracting the minimum value, and dividing by the max
    //giving us values between 0 and 1
	double sMin = getMin(signal[chNum], nSamples);
	double *sigTmp = new double[nSamples];
	for (int i = 0; i < nSamples; i++)
		sigTmp[i] = signal[chNum][i] - sMin;

	//now normalize the data
	double sMax = getMax(sigTmp, nSamples);
    if (sMax == 0)
		return tmpStat;

	for (int i = 0; i < nSamples; i++)
		sigTmp[i] /= sMax;
    */
	//get the time differences
    int sigPos = 0;
	bool risingEdge, fallingEdge;
    for (int sample = blockSize; sample < nSamples-blockSize; sample += blockSize)
    {
        //record the starting sample
		sigPos = sample;
		risingEdge = false;
		fallingEdge = false;
		
        //first check that the signal already is not above threshold,
        //and if it is, just continue
        //if (signal(chNum, sigPos) > thresh && signal(chNum, sigPos-1) >= thresh)
        //    continue;

        //go through the signal, and increment the signal position until it
        //crosses the threshold
		while (!risingEdge && (signal[chNum][sigPos] <= thresh) && ((sigPos-sample) < blockSize))
		{
			risingEdge = (signal[chNum][sigPos] >= thresh) && (signal[chNum][sigPos-1]< thresh);
			sigPos++;
		}
		if (sigPos - sample >= blockSize)
		{
			sigPos = sample;
			//try again with a falling edge
            while (!fallingEdge && (signal[chNum][sigPos] >= thresh) && ((sigPos-sample) < blockSize))
			{
				fallingEdge = (signal[chNum][sigPos] < thresh) && (signal[chNum][sigPos-1] >= thresh);
				sigPos++;
			}
		}
        /*while ((signal(chNum, sigPos) <= thresh)
                && ((sigPos-sample) < blockSize))
            sigPos++;*/

        //the signal has crossed the threshold, so record the time difference
        //from the first sample in the block to when this occurred,
        //converted to ms, and add it to the tDiff vector
        if (sigPos >= sample)
            tDiff.push_back(((float)sigPos-(float)sample)/sampleRate*1000);
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
        if (thisTask.exportData)
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
basicStats analysis::doThreshAnalysis(int chNum, string stateName, int stateVal)
{
	vector<double> tDiff;
	basicStats tmpStat;
	tmpStat.mean = -1;
	tmpStat.std = -1;
	tmpStat.min = -1;
	tmpStat.max = -1;
	tmpStat.median = -1;

    //normalize the data by subtracting the minimum value, and dividing by the max
    //giving us values between 0 and 1
	if (chNum >= nChannels || chNum < 0)
		return tmpStat;

	map<string, double*>::iterator itr = states.find(stateName);
	if (itr == states.end())
		return tmpStat;

        /*  we are not normalizing any more...for now
    double sMin = getMin(signal[chNum], nSamples);
    double *sigTmp = new double[nSamples];
    for (int i = 0; i < nSamples; i++)
        sigTmp[i] = signal[chNum][i] - sMin;

    //now normalize the data
	double sMax = getMax(sigTmp, nSamples);
	if (sMax == 0)
		return tmpStat;
    for (int i = 0; i < nSamples; i++)
        sigTmp[i] /= sMax;
                 */
    //get the time differences
    //this finds both EACH state transition and COMBINED state transitions
    map<int, vector<double> > stateTDiffs;

    int sigPos = 0;
    int dState;
    bool risingEdge = false;
	for (int sample = 1; sample < nSamples-blockSize; sample += 1)
    {
		//get the transition value
		dState = (states[stateName][sample]-states[stateName][sample-1]);
        //we are only looking for positive CHANGES in the state
        if (dState <= 0)
            continue;

        sigPos = sample;
        risingEdge = false;
        //first check that the signal already is not above threshold,
        //and if it is, just continue
        /*if (signal(chNum, sigPos) > thresh && signal(chNum, sigPos-1) > thresh)
            continue;
        */
        //look for the sample in the signal where it crosses the threshold
        /*
        bool nextState = false;
        while (sigPos < nSamples && signal(chNum, sigPos) < thresh && !nextState )
        {
            sigPos++;
            if( states[stateName][sigPos] != 0 && states[stateName][sigPos] != dState)
                nextState = true;
        }

        if (nextState)
            continue;   */
        while (!risingEdge && (sigPos < nSamples) && (signal[chNum][sigPos] <= thresh) )
        {
            risingEdge = (signal[chNum][sigPos] >= thresh) && (signal[chNum][sigPos-1] < thresh);
            sigPos++;
        }
        //calculate the time difference
		float D = ((float)sigPos-(float)sample)/sampleRate*1000;

		if (sigPos > sample && sigPos < nSamples)
		{
            //store the time difference if the state change value is 0 or equal to what we specified
            
			if (stateVal == 0 || (stateVal == dState))
				tDiff.push_back(D);
            else if (stateVal == -1)
			{
                //if we set state val to -1, we record all differences, and keep track of
                //the times for each dState value
				tDiff.push_back(D);
				stateTDiffs[dState].push_back(D);
			}
        }
    }

	if (tDiff.size() == 0)
		return tmpStat;

    //calculate the stats on the tDiff vector

    tmpStat.mean = vMean(&tDiff);
    tmpStat.std = vStd(&tDiff);
    tmpStat.min = vMin(&tDiff);
	tmpStat.max = vMax(&tDiff);
	tmpStat.median = vMedian(&tDiff);
    if (thisTask.exportData)
        tmpStat.vals = tDiff;

    stringstream str;
    str << "Ch " << chNum <<" v. " << stateName;
    if (stateVal > 0)
        str <<"("<<stateVal<<")";
        
    tmpStat.desc = str.str();

    if (stateVal >= 0)
        return tmpStat;

    //now compare all of the states if stateVal == -1
    //we keep the state with the smallest overall mean and return its
    map<int, vector<double> >::iterator it;
    it = stateTDiffs.begin();

    double minMean = tmpStat.mean;
    for (it = stateTDiffs.begin(); it != stateTDiffs.end(); it++)
    {
        double tmpMean = vMean(&it->second);
        if (tmpMean < minMean)
        {
                minMean = tmpMean;
                tmpStat.mean = vMean(&it->second);
                tmpStat.std = vStd(&it->second);
                tmpStat.min = vMin(&it->second);
				tmpStat.max = vMax(&it->second);
				tmpStat.median = vMedian(&it->second);
                tmpStat.vals = it->second;

                stringstream str;
                str << "Ch " << chNum <<" v. " << stateName << "("<<it->first<<")";
                tmpStat.desc = str.str();
        }
    }

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
	double prevVal = signal[ch][0];
    for (int s = 1; s < nSamples; s++)
    {
		if (signal[ch][s] == 0 && prevVal == 0)
            droppedSamples++;

		if (signal[ch][s] >= 1e7) // 10 million uV, or 10 V
            droppedSamples++;

        prevVal = signal[ch][s];
  }
}

void analysis::checkDroppedSamples()
{
    double prevVal;
    droppedSamples = 0;
    for (int ch = 0; ch < nChannels; ch++)
    {
		prevVal = signal[ch][0];
        for (int s = 1; s < nSamples; s++)
        {
			if (signal[ch][s] == 0 && prevVal == 0)
                droppedSamples++;

			if (abs(signal[ch][s]) >= 1e7) // 10 million uV, or 10 V
                droppedSamples++;

			prevVal = signal[ch][s];
        }
    }
}

/*
From here, these are all helper functions to find the mean, min, max etc of
the vector<double> arrays
*/

double analysis::getMin(double *d, int n)
{
    double v = 0;
    for (int i = 0; i < n; i++)
        v = dMin(d[i], v);

    return v;
}

double analysis::getMax(double *d, int n)
{
    double v = 0;
    for (int i = 0; i < n; i++)
        v = dMax(d[i], v);

    return v;
}


double analysis::vMean(vector<double> *a)
{
    double v = 0;
    for (int i=0; i < (int)a->size(); i++)
        v += (*a)[i];

    if (a->size() > 0)
        v /= a->size();

    return v;
}

double analysis::vMedian(vector<double> *a)
{
	vector<double> tmp = *a;
	std::sort(tmp.begin(), tmp.end());
	return *(tmp.begin()+tmp.size()/2);
}

double analysis::vStd(vector<double> *a)
{
    double m = vMean(a);
    double v = 0;
    for (int i = 0; i < (int)a->size(); i++)
        v += ((*a)[i] - m)*((*a)[i] - m);

    if (a->size() > 0)
        v /= a->size();

    v = sqrt(v);
    return v;
}

double analysis::vMax(vector<double> *a)
{
    if (a->size() == 0)
        return 0;
        
    double v = (*a)[0];
    for (int i=1; i < (int)a->size(); i++)
        v = ((*a)[i] > v) ? ((*a)[i]) : v;

    return v;
}

double analysis::vMin(vector<double> *a)
{
    if (a->size() == 0)
        return 0;
        
    double v = (*a)[0];
    for (int i=1; i < (int)a->size(); i++)
        v = ((*a)[i] < v) ? ((*a)[i]) : v;

    return v;
}

bool analysis::isMember(vector<string> strArr, string str)
{
    for (int i = 0; i < (int)strArr.size(); i++)
    {
        if (strArr[i] == str)
            return true;
    }
	return false;
}

void analysis::print(FILE * out, vector<basicStats> minReqs)
{
    fprintf(out, "%s\n",shortFname(fName).c_str());
    //resOut << std::endl<<fName<<":"<<std::endl;
	bool ok = true;

    for (int t = 0; t < latencyStats.size(); t++)
    {
        basicStats tmpTask = latencyStats[t];
        fprintf(out, "\t%s\t",(tmpTask.taskName).c_str());
        //resOut<<tmpTask.taskName<<": ";
        int tfound = 0;

        //compare this against the min requirements by searching through the minreqs array
        //and checking if the names match; if they do, see if the mean and stddev are within the bounds

        for (int a = 0; a < minReqs.size(); a++)
        {
            if (tolower(strtrim(tmpTask.taskName)) == tolower(strtrim(minReqs[a].taskName)))
            {
                tfound = 1;
                if (tmpTask.mean <= (minReqs[a].mean) && tmpTask.mean >= 0)
                    fprintf(out, "%s\t","pass");
                else if (tmpTask.mean == -1)
                {
                    tfound = 2;
                    fprintf(out, "%s\t","no stimuli");
                    continue;
                }
                else
                {
                    fprintf(out, "%s\t","fail");
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
            fprintf(out, "%1.2f\t%1.2f\t%1.2f\t%1.2f\t%1.2f\n",tmpTask.mean, tmpTask.median, tmpTask.std, tmpTask.min, tmpTask.max);
        }
    }

    fprintf(out,"\tDropped Samples\t\t%d/%d\n",droppedSamples, checkedSamples);
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

    for (int i = 0; i < latencyStats.size(); i++)
    {
        basicStats tmpTask = latencyStats[i];
        if (tmpTask.vals.size() == 0)
            continue;
        o<<tmpTask.taskName;
        o.precision(4);
        for (int s = 0; s < tmpTask.vals.size(); s++)
            o << ", " << tmpTask.vals[s];
        o << endl;
    }

    o.close();
    return true;
}
