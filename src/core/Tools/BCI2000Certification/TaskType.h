/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef TaskTypeH
#define TaskTypeH
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "structs.h"
#include "Functions.h"

using namespace std;


class TaskType
{
public:
    TaskType();
    ~TaskType();
	std::string taskName;
	std::string taskFolder;
    std::vector<std::string> parms;
    std::vector<std::string> states;
    std::string SignalSource, SigProc, App;
	std::vector<std::string> parmFile;
	std::vector<std::string> parmFileDisp;
    analysisType amp;
    analysisType dAmp;
    analysisType vid;
	analysisType aud;
	float blockSize, sampleRate;
    bool skip;
    bool exportData;
	void initTasks();
	void addParm(std::string);
	void delParm(int);
};

class Tasks : public std::vector<TaskType>
{
public:
    Tasks();
    Tasks(std::string fname);
    ~Tasks();

    void init(std::string fname);
    void parseIni();
	int getReturnCode(){return returnCode;}
	string GlobalSource;
	bool writeIni(string);
private:
    int returnCode;
};


//---------------------------------------------------------------------------
#endif
