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
#pragma hdrstop

#include "Functions.h"
#include <time.h>

using namespace std;

/*------------------------------------------
parseDir
description: recursively searches a directory, and stores all *.dat files found in a string vector
input: string dir - the directory to search
		vector<string> &fnames - the vector that stores the file names; this is passed by reference,
			and is changed by this function
output: bool - used by the recursion; returns true if the dir is a directory, and false if it is not
	(and therefore is probably a file)
*/
bool parseDir(string dir, vector<string> *fnames)
{
	//first check that the directory exists
	/*
	DIR *dp;
	struct dirent *ep;
	dp = opendir(dir.c_str());
	if (dp != NULL)
	{
		//it is a directory
		while (ep = readdir(dp))
		{
			if (ep->d_name[0] == '.')
				continue;
			string nextEntry(dir);
			nextEntry.append("\\");
			nextEntry.append(ep->d_name);
			if (!parseDir(nextEntry, fnames))
			{
				//it is a file...possibly
				//check if it ends with .dat
				int datPos = nextEntry.find(".dat");
				if (datPos != -1)
					fnames->push_back(nextEntry);
			}
			else
			{
				//it is a directory...AND has already been parsed
				//all hail the power of recursion!
			}
		}
		closedir(dp);
		return true;
	}
	else
	{
		//is it a file?
		return false;
	}
	*/
	return true;
}
/*
getFullDir
description: returns the full path of a directory (dirPath), whose path is relative to the current working directory
input: string dirPath- the relative directory to find the full path for
output: string - the full path of dirPath
*/
/*
string getFullDir(string dirPath)
{
	char curpath[512];
	//get the current working directory
	getcwd(curpath, 512);
	//cout <<curpath<<endl;

	//create a new string to use (and return)
	string fullpath(curpath);

	//go through the current dirPath, and find the first relative directory separator
    int pos = dirPath.find(".\\");
    if (pos != -1)
        dirPath.replace(pos, 2,"");

	pos = dirPath.find("..\\");
	while (pos != -1)
	{
		//go through the fullpath, and remove a directory level in the full path
		//for every relative level found in the dirPath
		int npos = fullpath.find_last_of("\\");
		fullpath.erase(npos);
		dirPath.replace(pos, 3, "");
		pos = dirPath.find("..\\");
	}
	fullpath += "\\";
	//append the remaining dirPath to the fullpath
	fullpath.append(dirPath);
	return fullpath;
}
*/
/*---------------------------------------
parseParm
description: parses a bci2000 parameter file to find the save location for the file(s)
input: string parmName - the path of the parameter file
output: vector<string> - a string array containing every dat file in the location specified by the parm file
*/
/*
vector<string>* parseParm(string parmName)
{
	//open the parm file, and do initial error checking
	ifstream file;
	file.open(parmName.c_str());
	vector<string> *fnames = new vector<string>;
	if (!file.is_open())
	{
		cout << "Unable to find parameter file: "<< parmName<<". Quitting."<<endl;
		exit(0);
	}

	//if the file was found, go through the file to find the DataDirectory parameter
	string line, DataDirectory="";
	bool DataDirFound = false;
	while(getline(file, line) && !DataDirFound)
	{
		string::size_type loc = line.find("DataDirectory");
		//if the parameter line has DataDirectory, parse it
		if (loc != string::npos) {
			//we found the line with the data directory, so get the rest of the info
			DataDirFound = true;
			int pos1 = line.find_first_of(" ", loc+1)+1;
			int pos2 = line.find_first_of(" ", pos1+1);
			DataDirectory = line.substr(pos1, pos2-pos1);
		}
	}
	//close the parm file
    file.close();
	if (!DataDirFound)
	{
		cout <<parmName<<" does not contain a DataDirectory parameter. Exiting."<<endl;
		exit(0);
	}

	//get the full path of the dataDirectory, and parse it for more info
	string fullpath = getFullDir(DataDirectory);
	parseDir(fullpath, fnames);

	return fnames;
}

*/

/*---------------------------------------
parseCfg
description: parse the configuration file, which should never really be touched by the user, and get default settings
input: double thresh - the threshold that is used for detection in analysis; usually 0.25
	string outfilepath - the path of the file that will contain the analysis results
	string datDir - the default data directory
output: vector<basicStats> - this array of basicStats contains the minimum requirements for each task
	for the task to be considered bci2000 compliant
		- additionally, all input parameters are passed by reference and are modified by this function

bool parseCfg(double *thresh, string *outfilepath, string *datDir, vector<basicStats*> *minReqs)
{
	return parseCfg(thresh, outfilepath, datDir, minReqs, string("BCI2000Certification.cfg"));
}
*/

double getMin(double *d, int n)
{
    double v = 0;
    for (int i = 0; i < n; i++)
        v = dMin(d[i], v);

    return v;
}

double getMax(double *d, int n)
{
    double v = 0;
    for (int i = 0; i < n; i++)
        v = dMax(d[i], v);

    return v;
}


double vMean(vector<double> *a)
{
    double v = 0;
    for (int i=0; i < (int)a->size(); i++)
        v += (*a)[i];

    if (a->size() > 0)
        v /= a->size();

    return v;
}

double vMedian(vector<double> *a)
{
	vector<double> tmp = *a;
	std::sort(tmp.begin(), tmp.end());
    double v = *(tmp.begin()+tmp.size()/2);

    return v;
}
double vStd(vector<double> *a)
{
    double m = vMean(a);
    double v = 0;
    for (int i = 0; i < (int)a->size(); i++)
        v += ((*a)[i] - m)*((*a)[i] - m);

    if (a->size() > 0)
        v /= a->size();

    v = sqrt(v);
    return v;
}

double vMax(vector<double> *a)
{
    if (a->size() == 0)
        return 0;

    double v = (*a)[0];
    for (int i=1; i < (int)a->size(); i++)
        v = ((*a)[i] > v) ? ((*a)[i]) : v;

    return v;
}

double vMin(vector<double> *a)
{
    if (a->size() == 0)
        return 0;

    double v = (*a)[0];
    for (int i=1; i < (int)a->size(); i++)
        v = ((*a)[i] < v) ? ((*a)[i]) : v;

    return v;
}

void removeNPercentile(vector<double> *a, double perc)
{
	std::sort(a->begin(), a->end());
	size_t s = a->size();
	double b = (perc/100)*double(s)/2;
	while (b > 0 && a->size() > 0){
		a->erase(a->begin());
		a->pop_back();
		b--;
	}
}

bool isMember(vector<string> strArr, string str)
{
    for (int i = 0; i < (int)strArr.size(); i++)
    {
        if (strArr[i] == str)
            return true;
    }
    return false;
}



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
#ifdef __BORLANDC__
# pragma package(smart_init)
#endif // __BORLANDC__
