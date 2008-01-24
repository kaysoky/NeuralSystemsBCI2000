/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#pragma hdrstop

#include "Functions.h"

/*------------------------------------------
shortFname
description: removes the directory/path from a file name
input: string fname - the file name, including the full path
output: string - the file name, with the path removed
*/
string shortFname(string fname)
{
	//find the last directory separator position, and remove everything up to and including it
	int pos = fname.find_last_of("\\");
	pos = fname.find_last_of("\\",pos-1);
	return fname.substr(pos+1);
}

/*------------------------------------------
parseDir
description: recursively searches a directory, and stores all *.dat files found in a string vector
input: string dir - the directory to search
		vector<string> &fnames - the vector that stores the file names; this is passed by reference,
			and is changed by this function
output: bool - used by the recursion; returns true if the dir is a directory, and false if it is not
	(and therefore is probably a file)
*/
bool parseDir(string dir, vector<string> &fnames)
{
	//first check that the directory exists
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
					fnames.push_back(nextEntry);
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
}
/*
getFullDir
description: returns the full path of a directory (dirPath), whose path is relative to the current working directory
input: string dirPath- the relative directory to find the full path for
output: string - the full path of dirPath
*/
string getFullDir(string dirPath)
{
	char curpath[_MAX_PATH];
	//get the current working directory
	getcwd(curpath, _MAX_PATH);
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

/*---------------------------------------
parseParm
description: parses a bci2000 parameter file to find the save location for the file(s)
input: string parmName - the path of the parameter file
output: vector<string> - a string array containing every dat file in the location specified by the parm file
*/
vector<string> parseParm(string parmName)
{
	//open the parm file, and do initial error checking
	ifstream file;
	file.open(parmName.c_str());
	vector<string> fnames;
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



/*---------------------------------------
parseCfg
description: parse the configuration file, which should never really be touched by the user, and get default settings
input: double thresh - the threshold that is used for detection in analysis; usually 0.25
	string outfilepath - the path of the file that will contain the analysis results
	string datDir - the default data directory
output: vector<basicStats> - this array of basicStats contains the minimum requirements for each task
	for the task to be considered bci2000 compliant
		- additionally, all input parameters are passed by reference and are modified by this function
*/
bool parseCfg(double &thresh, string &outfilepath, string &datDir, vector<basicStats> &minReqs)
{
    outfilepath = "";
    datDir = "";
    thresh = .25;
    minReqs.clear();

	ifstream file;
	file.open("BCI2000Certification.cfg");
	if (!file.is_open())
	{
		file.close();
		cout << "Error opening BCI2000Certification.cfg. Quitting."<<endl;
		return false;
	}

	/*
	The BCI2000Certification.cfg file contains definitions for the minimum requirements
	for any BCI2000 system. It has a format of:
	amp mval stdval
	proc mval stdval
	output mval stdval
	system mval stdval
	jitter mval stdval

	in which the "vals" are the values in ms of the maximum value allowed for tha parameter,
	and stdval is the standard deviation. The stdval can be left out if desired.
	These values are NOT task specific; this must be true for all tests for a system
	to be BCI2000 compatible.
	*/

	//parse the file if it exists
	string line;
	while (getline(file, line))
	{
		//bool ok = true;
		stringstream ss(line);
		string strTok;
		ss >> strTok;

		//check the string tokens for acceptable keywords
		if (tolower(strTok) == "threshold")
		{
			ss >> thresh;
			continue;
		}
		if (tolower(strTok) == "resultout")
		{
			ss >> outfilepath;
			continue;
		}
		if (tolower(strTok) == "datadir")
		{
			ss >> datDir;
			continue;
		}

		//if it is not one of the above tokens, then it describes a task component
		basicStats tmpStat;

		//set default values for the mean and standard deviation
		//generally, the mean is always set, but the stddev is not
		//if we use the >> operator and ss is empty, then the values are not changed,
		//and the acceptable "NA" values are stored
		float sMean = 0, sStd = -1;
		ss >> sMean;
		ss >> sStd;
		tmpStat.mean = sMean;
		tmpStat.std = sStd;
		tmpStat.taskName = strTok;
		minReqs.push_back(tmpStat);
	}
    file.close();

    //add the date+time to the outfilepath
    char dateStr[10];
	char timeStr[10];
	_strdate(dateStr);
	_strtime(timeStr);

    //remove the extension temporarily
    int pos = 0;
    string ext = "";
    pos = outfilepath.find('.',0);
    ext = outfilepath.substr(pos, string::npos);
    outfilepath.erase(pos, string::npos);

    outfilepath = outfilepath + "_" + dateStr + "_" + timeStr + ext;
    //replace slashes with -s
    pos = 0;
    while (pos != string::npos)
    {
        pos = outfilepath.find_first_of('/',pos);
        if (pos != string::npos)
            outfilepath.replace(pos, 1, "-");
    }
    pos = 0;
    while (pos != string::npos)
    {
        pos = outfilepath.find_first_of(':',pos);
        if (pos != string::npos)
            outfilepath.replace(pos, 1, "-");
    }

    if (outfilepath == "" || datDir == "")
    {
        cout << "The BCI2000Certification.cfg file does not contain the required settings."<<endl;
        return false;
    }

	return true;
}

/*---------------------------
parseIni
description: parses the BCI2000Certification.ini file, in which the task types are defined
input: NA
output: Tasks - an array of TaskType, which describes the task and how to analyze the data for that task
*/
Tasks parseIni()
{
	//check that the ini file exists, and gracefully exit if it does not
    Tasks taskTypes;
    taskTypes.init("BCI2000Certification.ini");
    return taskTypes;
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
