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
#ifndef FunctionsH
#define FunctionsH

#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <iostream>
#include "TaskType.h"
#include "structs.h"


bool parseDir(std::string dir, std::vector<std::string> *fnames);
bool parseCfg(double *thresh, std::string *outfilepath, std::string *datDir, std::vector<basicStats*> *minReqs);
bool parseCfg(double *thresh, std::string *outfilepath, std::string *datDir, std::vector<basicStats*> *minReqs, std::string fileLoc);
std::string tolower(std::string str);
std::string strtrim(std::string str);
//Tasks parseIni();
std::vector<std::string>* parseParm(std::string parmName);
bool isMember(std::vector<std::string>, std::string);
std::string getFullDir(std::string dirPath);
double getMin(double *d, int n);
double getMax(double *d, int n);
inline double dMin(double a, double b){return (a<b) ? (a) : (b);}
inline double dMax(double a, double b){return (a>b) ? (a) : (b);}
inline double dAbs(double d){return (d >= 0) ? (d) : (d*-1);}
double vMean(std::vector<double> *a);
double vStd(std::vector<double> *a);
double vMax(std::vector<double> *a);
double vMin(std::vector<double> *a);
double vMedian(std::vector<double> *a);
void removeNPercentile(std::vector<double> *a, double perc);
//---------------------------------------------------------------------------
#endif // FunctionsH
