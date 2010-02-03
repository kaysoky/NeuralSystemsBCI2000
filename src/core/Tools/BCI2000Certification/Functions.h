/* (C) 2000-2010, BCI2000 Project
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
#include <math.h>
#include <algorithm>
#include <iostream>
#include "TaskType.h"
#include "structs.h"
//#include "analysisClass.h"

using namespace std;

string shortFname(string fname);
string getCurDateTime();
bool parseDir(string dir, vector<string> *fnames);
bool parseCfg(double *thresh, string *outfilepath, string *datDir, vector<basicStats*> *minReqs);
bool parseCfg(double *thresh, string *outfilepath, string *datDir, vector<basicStats*> *minReqs, string fileLoc);
string tolower(string str);
string strtrim(string str);
//Tasks parseIni();
vector<string>* parseParm(string parmName);
bool isMember(vector<string>, string);
string getFullDir(string dirPath);
double getMin(double *d, int n);
double getMax(double *d, int n);
inline double dMin(double a, double b){return (a<b) ? (a) : (b);}
inline double dMax(double a, double b){return (a>b) ? (a) : (b);}
inline double dAbs(double d){return (d >= 0) ? (d) : (d*-1);}
double vMean(vector<double> *a);
double vStd(vector<double> *a);
double vMax(vector<double> *a);
double vMin(vector<double> *a);
double vMedian(vector<double> *a);
//---------------------------------------------------------------------------
#endif
