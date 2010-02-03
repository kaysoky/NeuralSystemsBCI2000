/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef analysisClassH
#define analysisClassH
#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include "BCI2000FileReader.h"
#include "TaskType.h"
#include "Functions.h"

#include <map>
#include <vector>
#include <math.h>

using namespace std;


//void initTaskType(TaskType &t);

class analysis
{
public:
    analysis();
    ~analysis();
    bool open(string file, Tasks &taskTypes);
    bool close();
    void clear();

    basicStats doThreshAnalysis(int chNum);
    basicStats doThreshAnalysis(int chNum, string stateName, vector<int>);
	bool doThreshAnalysis(double thresh);
    string getTaskName(){return thisTask.taskName;}
    bool getSkip(){return thisTask.skip;}
    bool getExportData(){return thisTask.exportData;}

	bool print(FILE * out, vector<basicStats*> minReqs, int);
    bool exportData(string expfile);
	double thresh;
    TaskType thisTask;
    string getFileName(){return fName;}
	vector<basicStats> latencyStats;
private:
	string fName;
	string FileInitials, SubjectName, SubjectSession, SubjectRun;
    BCI2000FileReader *dFile;
    bool mIsOpen;
    int nSamples;
    int nChannels;
    float sampleRate;
    int blockSize;
	int mIgnoreDur;

    void checkDroppedSamples(int ch);
    void checkDroppedSamples();
    //double **signal;

    map<string, double*> states;
    map<string, double*>::iterator it;
    vector<string> stateNames;
    vector<string> ignoreStates;  

    int nStates;
    unsigned int droppedSamples, checkedSamples;
};


//---------------------------------------------------------------------------
#endif
