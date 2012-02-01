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
#include <QString>
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
    bool open(std::string file, Tasks &taskTypes);
    bool close();
    void clear();

    basicStats doThreshAnalysis(int chNum);
    basicStats doThreshAnalysis(int chNum, std::string stateName, std::vector<int>);
    bool doThreshAnalysis(double thresh);
    std::string getTaskName(){return thisTask.getTaskName().toStdString();}
    bool getSkip(){return thisTask.skip;}
    bool getExportData(){return thisTask.exportData;}

    bool print(FILE * out, std::vector<basicStats*> minReqs, int);
    bool exportData(std::string expfile);
    std::string getSubjectName(){return SubjectName;}
    double thresh;
    TaskType thisTask;
    std::string getFileName(){return fName;}
    std::vector<basicStats> latencyStats;
private:
    std::string fName;
    std::string FileInitials, SubjectName, SubjectSession, SubjectRun;
    BCI2000FileReader *dFile;
    bool mIsOpen;
    int nSamples;
    int nChannels;
    float sampleRate;
    int blockSize;
    int mIgnoreDur;

    void checkDroppedSamples(int ch);
    void checkDroppedSamples();

    std::map<std::string, double*> states;
    std::map<std::string, double*>::iterator it;
    std::vector<std::string> stateNames;
    std::vector<std::string> ignoreStates;

    int nStates;
    unsigned int droppedSamples, checkedSamples;
};


//---------------------------------------------------------------------------
#endif // analysisClassH
