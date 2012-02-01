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
#include "certificationClass.h"

using namespace std;

certificationClass::certificationClass()
{
	mResOut = NULL;
}

certificationClass::~certificationClass()
{
	if (mResOut != NULL)
		fclose(mResOut);
}

void certificationClass::run()
{
	//std::cout << "Starting BCI2000 Latency Analysis" << endl;

	for (int i = 0; i < mFileNames.size(); i++)
	{
		//create an analysis object to use for this file
        analysis *an = new analysis;

		//open the file; this checks if the file is valid/exists, loads the data (signal, states, parms)
		//and determines the task type based on the definitions loaded from the ini file
		mProgress->increment("Opening " + getShortFileName(mFileNames[i]));
        //cout <<"Opening "<< getShortFileName(mFileNames[i]) << "...";
		if (!an->open(mFileNames[i].toStdString(), mTaskTypes))
        {
            //if there is an error or the file cannot be classified, just continue with the next one
            if (an->getTaskName() == "Unknown")
            {
                //resOut <<mFileNames[i] <<": task undefined in BCI2000Certification.ini file. Skipping"<<endl;
                cout << "Undefined task. Skipping!"<<endl;
				mProgress->increment("Undefined task. Skipping!");
                fprintf(mResOut, "%s\tundefined\n",mFileNames[i].toStdString().c_str());
                continue;
            }
			if (an->getTaskName().size() == 0)
            {
				mProgress->increment("file not found!");
                fprintf(mResOut, "%s\tNot Found\n",getShortFileName(mFileNames[i]).toStdString().c_str());
                continue;
            }
        }

		//add the task to the checked types, which is used later for output
		//checkedTypes.push_back(thisTask);
        //update the stdio and log


        if (an->getSkip())
        {
            //fprintf(mResOut, "%s\tSkipped\n",getShortFileName(mFileNames[i]).c_str());
			mProgress->increment("Skipping " +getShortFileName(mFileNames[i]));
            continue;
        }
        else
        {
            //mResOut << "Testing " <<getShortFileName(mFileNames[i]) <<"..." <<endl;
			mProgress->increment("Testing " + getShortFileName(mFileNames[i]));
        }

		//actually run the analysis on this task
        an->doThreshAnalysis(0.0);
        an->close();
		//close out the analysis
        mAnalyses.push_back(an);

        //write data to csv file if necessary
       // if (an->getExportData())
         //   an->exportData(getLogFile(&(an->thisTask)));
        //an.clear();

	}

	fprintf(mResOut, "Certification Results: %s\n", mDateTime.toStdString().c_str());

	//cout<<"Results..."<<endl;
	//the mAnalyses vector now contains an array for the results for each file
	//the results for each file contains results based on what was tested (e.g., video, audio, metronome, etc)
	//based on what was specified in the ini file
	//these results are compared against the minimum requires specified in the cfg file, and
	//compliance determination is output to the log and stdio
	int passed = 0;
	vector<double> amplat, damplat, vidlat, audlat, blockdur;
	vector<int> failedList;
	for (unsigned int i = 0; i < mAnalyses.size(); i++)
	{
		if (i % 10 == 0)
			fprintf(mResOut, "\nFile\tTask\tPass/Fail\tAvg(ms)\tMed(ms)\tStd(ms)\tMin(ms)\tMax(ms)\n");
		if (!mAnalyses[i]->getSkip())
			if(mAnalyses[i]->print(mResOut, mMinReqs, i))
				passed++;
			else
				failedList.push_back(i);

		//get average amp, video, and audio latencies
		for (size_t j = 0; j < mAnalyses[i]->latencyStats.size(); j++){
			if (mAnalyses[i]->latencyStats[j].taskName.trimmed().toLower().compare("vidout") == 0){
				vidlat.insert(vidlat.begin(),
							  mAnalyses[i]->latencyStats[j].vals.begin(),
							  mAnalyses[i]->latencyStats[j].vals.end());
			}
			else if (mAnalyses[i]->latencyStats[j].taskName.trimmed().toLower().compare("audout") == 0){
				audlat.insert(audlat.begin(),
							  mAnalyses[i]->latencyStats[j].vals.begin(),
							  mAnalyses[i]->latencyStats[j].vals.end());
			}
			else if (mAnalyses[i]->latencyStats[j].taskName.trimmed().toLower().compare("jitter") == 0){
				blockdur.insert(blockdur.begin(),
								mAnalyses[i]->latencyStats[j].vals.begin(),
								mAnalyses[i]->latencyStats[j].vals.end());
			}
		}

	}
	fprintf(mResOut,"\n-------------------------------\n");
	//char buf[512];
	if (vidlat.size() > 0)
		fprintf(mResOut,"Video Output Latency: %1.2f +/- %1.2f ms\n", vMean(&vidlat), vStd(&vidlat));
	if (audlat.size() > 0)
		fprintf(mResOut,"Audio Output Latency: %1.2f +/- %1.2f ms\n", vMean(&audlat), vStd(&audlat));
	if (blockdur.size() > 0)
		fprintf(mResOut,"Block Duration: %1.2f +/- %1.2f ms\n", vMean(&blockdur), vStd(&blockdur));

	fprintf(mResOut,"\n-------------------------------\n%d/%d tests passed.\n",passed, mAnalyses.size());
	//sprintf(buf, "\n-------------------------------\n%d/%d tests passed.\n",passed, mAnalyses.size());
	//cout << buf;
	//close the log file
	mProgress->increment("Testing Complete!");
	if (passed != mAnalyses.size()){
		fprintf(mResOut, "\nWarning: the following test configurations did not pass:\n");
		for (size_t i=0; i < failedList.size(); i++)
			fprintf(mResOut, " %d", failedList[i]);
		fprintf(mResOut, "\n");
	}
	fclose(mResOut);

    //clear the mAnalyses
    for (size_t i = 0; i < mAnalyses.size(); i++)
		delete mAnalyses[i];

	for (size_t i = 0; i < mMinReqs.size(); i++)
		delete mMinReqs[i];
	//delete datDir;

	return;
}
bool certificationClass::init(QStringList fileNames, QString iniFile, QString cfgFile, QString outputDir, progressClass *progress)
{
	mOutputDir = outputDir;
	mIniFile = iniFile;
	mCfgFile = cfgFile;
	mFileNames = fileNames;
	mProgress = progress;

	//if no file names were found based on the input args given, quit gracefully
    if (mFileNames.size() == 0)
    {
        mError = "No file names specified or found.";
		return false;
	}

	if (!parseCfg()){
		return false;
	}
	genResultsFile();
	mTaskTypes.init(mIniFile);

	//open the result file output stream, and quit if there is an error
	mResOut = fopen(mOutFilePath.toStdString().c_str(), "w");
	if (mResOut == NULL)
	{
		mError = "Error opening output file at: " + mOutFilePath + ".";
		return false;
	}

	return true;
}

QString certificationClass::getShortFileName(QString str){
	int pos = str.lastIndexOf("/");
	if (pos != -1)
		str = str.right(str.size()-pos-1);

	return str;
}

bool certificationClass::parseCfg()
{
	mMinReqs.clear();

	ifstream file;
	file.open(mCfgFile.toStdString().c_str());
	if (!file.is_open())
	{
		file.close();
		mError = "Error finding or opening " + mCfgFile + ". Ensure that this file exists.";
		//cout << "Error opening BCI2000Certification.cfg. Quitting."<<endl;
		return false;
	}

	/*
	The BCI2000Certification.cfg file contains definitions for the minimum requirements
	for any BCI2000 system. It has a format of:
	amp mval stdval
	proc mval stdval
	output mval stdval
	system mval stdval
	jitter mval stdval

	in which the "vals" are the values in ms of the maximum value allowed for tha parameter,
	and stdval is the standard deviation. The stdval can be left out if desired.
	These values are NOT task specific; this must be true for all tests for a system
	to be BCI2000 compatible.
	*/

	//parse the file if it exists
	string line;
	//datDir->clear();
	while (getline(file, line))
	{
		//bool ok = true;
		stringstream ss(line);
		string strTok;
		ss >> strTok;

		//check the QString tokens for acceptable keywords

        if (tolower(strTok) == "task")
        {
            //if it is not one of the above tokens, then it describes a task component
            ss >> strTok;
            basicStats *tmpStat = new basicStats;

            //set default values for the mean and standard deviation
            //generally, the mean is always set, but the stddev is not
            //if we use the >> operator and ss is empty, then the values are not changed,
            //and the acceptable "NA" values are stored
            float sMean = 0, sStd = -1;
            ss >> sMean;
            ss >> sStd;
            tmpStat->mean = sMean;
			tmpStat->std = sStd;
			tmpStat->taskName = strTok.c_str();
            mMinReqs.push_back(tmpStat);
            continue;
        }

        //if we get here, then the configuration file is not setup correctly
		//mError = "The cfg file does not appear to be configured correctly.";
        //file.close();
        //return false;

	}
    file.close();

	return true;
}

void certificationClass::genResultsFile()
{
	//add the date+time to the outfilepath
	mDateTime = getCurDateTime();
	QDir tmp(mOutputDir);
	mOutFilePath = tmp.absolutePath() + "/results_" + mDateTime + ".txt";


}

QString certificationClass::getCurDateTime()
{
	return QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
}
