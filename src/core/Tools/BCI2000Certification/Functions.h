/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef FunctionsH
#define FunctionsH
#include <string>
#include <dirent.h>
//#include <stdlib.h>
#include <direct.h>
#include <vector>
#include <iostream>
#include "TaskType.h"
#include "analysisClass.h"

using namespace std;

string shortFname(string fname);
bool parseDir(string dir, vector<string> &fnames);
bool parseCfg(double &thresh, string &outfilepath, string &datDir, vector<basicStats> &minReqs);
Tasks parseIni();
vector<string> parseParm(string parmName);
string getFullDir(string dirPath);

//---------------------------------------------------------------------------
#endif
