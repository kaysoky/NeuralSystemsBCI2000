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

    map<string, double*> states;
    map<string, double*>::iterator it;
    vector<string> stateNames;
    vector<string> ignoreStates;

    int nStates;
    unsigned int droppedSamples, checkedSamples;
};


//---------------------------------------------------------------------------
#endif
