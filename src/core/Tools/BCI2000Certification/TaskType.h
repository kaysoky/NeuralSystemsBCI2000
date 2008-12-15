/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef TaskTypeH
#define TaskTypeH
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;
struct basicStats
{
    double mean, std, min, max, median;
    vector<double> vals;
    string taskName;
    string desc;
};

struct analysisType
{
    int ch;
    std::string state;
    std::vector<int> stateVal;
    bool flag;
};

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
/*
    TaskType operator[](const int i){return tasks[i];}
    void push_back(TaskType &t){tasks.push_back(t);}
    TaskType pop
private:
    std::vector<TaskType> tasks;
    */
};

//---------------------
string tolower(string str)
{
    for (unsigned int i = 0; i < str.length(); i++)
        str[i] = tolower(str[i]);
    return str;
}

string strtrim(string str)
{
	string::size_type pos = str.find_last_not_of(' ');
	if (pos != string::npos)
	{
		str.erase(pos+1);
		pos = str.find_first_not_of(' ');
		if (pos != string::npos)
			str.erase(0,pos);
	}
	else
	{
    	str.erase(str.begin(), str.end());
	}
	return str;
}
//---------------------------------------------------------------------------
#endif
