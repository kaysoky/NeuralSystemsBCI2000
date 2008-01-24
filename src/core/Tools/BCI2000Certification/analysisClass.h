/* (C) 2000-2008, BCI2000 Project
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

#include <map>
#include <vector>
#include <math.h>

using namespace std;

struct basicStats
{
    double mean, std, min, max;
    vector<double> vals;
    string taskName;
    string desc;
};



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
    basicStats doThreshAnalysis(int chNum, string stateName, int);
	bool doThreshAnalysis();
    string getTaskName(){return thisTask.taskName;}
    bool getSkip(){return thisTask.skip;}

    void print(ofstream&, vector<basicStats> minReqs);
	double thresh;
private:
	string fName;
	string FileInitials, SubjectName, SubjectSession, SubjectRun;
    BCI2000FileReader *dFile;
    bool mIsOpen;
    int nSamples;
    int nChannels;
    float sampleRate;
    int blockSize;
    TaskType thisTask;

    void checkDroppedSamples(int ch);
    double getMin(double *d, int n);
    double getMax(double *d, int n);
    double dMin(double a, double b){return (a<b) ? (a) : (b);};
    double dMax(double a, double b){return (a>b) ? (a) : (b);};
    double dAbs(double d){return (d >= 0) ? (d) : (d*-1);};
    double vMean(vector<double> *a);
    double vStd(vector<double> *a);
    double vMax(vector<double> *a);
    double vMin(vector<double> *a);
    double **signal;

    map<string, double*> states;
    map<string, double*>::iterator it;
    vector<string> stateNames;
    vector<string> ignoreStates;
    bool isMember(vector<string>, string);
    vector<basicStats> latencyStats;

    int nStates;
    unsigned short droppedSamples;
};


//---------------------------------------------------------------------------
#endif
