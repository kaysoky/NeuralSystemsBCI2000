/* (C) 2000-2009, BCI2000 Project
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

    void print(FILE*, vector<basicStats> minReqs);
    bool exportData(string expfile);
	double thresh;
    TaskType thisTask;
    string getFileName(){return fName;}

private:
	string fName;
	string FileInitials, SubjectName, SubjectSession, SubjectRun;
    BCI2000FileReader *dFile;
    bool mIsOpen;
    int nSamples;
    int nChannels;
    float sampleRate;
    int blockSize;


    void checkDroppedSamples(int ch);
    void checkDroppedSamples();
    double getMin(double *d, int n);
    double getMax(double *d, int n);
    double dMin(double a, double b){return (a<b) ? (a) : (b);};
    double dMax(double a, double b){return (a>b) ? (a) : (b);};
    double dAbs(double d){return (d >= 0) ? (d) : (d*-1);};
    double vMean(vector<double> *a);
    double vStd(vector<double> *a);
    double vMax(vector<double> *a);
	double vMin(vector<double> *a);
	double vMedian(vector<double> *a);
    //double **signal;

    map<string, double*> states;
    map<string, double*>::iterator it;
    vector<string> stateNames;
    vector<string> ignoreStates;
    bool isMember(vector<string>, string);
    vector<basicStats> latencyStats;

    int nStates;
    unsigned int droppedSamples, checkedSamples;
};


//---------------------------------------------------------------------------
#endif
