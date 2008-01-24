/*
BCI2000Certification.cpp
Author: Adam Wilson, University of Wisconsin-Madison, 2007

The BCI2000Certification.cpp file contains the main program for the certification
application, along with helper functions for parsing input and parameter files,
and providing help/output. These include:

printHelp - print an abridged or verbose help to the screen giving info about the program and it use
shortFName - extract the filename from the full path (e.g. for "c:\bci2000\latencyTest.prm", returns
	"latencyTest.prm"
parseDir - recursively parses a directory tree looking for bci2000 *.dat files, returning
	the file names in a string vector
getFullDir - changes a relative path (e.g. with "..\..\" etc) into a full path
parseParm - parses a BCI2000 parameter file to find file save information
parseInut - parses command line parameters to determine what to do with them
parseCfg - parses the BCI2000Certification.cfg file for permanent settings
parseIni - parses the BCI2000Certification.ini file, which contains all of the test types to analyze
main - controls the main program

The analysis itself is handled by the analysis class. See the analysisClass.cpp/h file for more info.

(C) 2000-2008, BCI2000 Project
http://www.bci2000.org
*/
//---------------------------------------------------------------------------
//#include <stdio.h>
#include <time.h>
#include <string>
//#include <fstream>
#include <vector>
#include <dirent.h>
//#include <stdlib.h>
#include <direct.h>
#include <sys/types.h>
#include "Functions.h"
#include "analysisClass.h"
#include "TaskType.h"

using namespace std;

/*
printHelp
input: int howMuch - 0 for short output, 1 for verbose - this depends on the command line parms or errors that occur
output: NA
*/
void printHelp(int howMuch)
{
	if (howMuch == 0)
	{
		cout<<"usage: BCI2000Certification [-p|-d|-h|fileNames] [options]"<<endl;
		cout<<"Passing no parameters will use the default data directory defined"<<endl;
		cout<<"in the BCI2000Certification.cfg file."<<endl;
		cout<<"-p [prmFile]: Specify a BCI2000 parameter file to get dat files."<<endl;
		cout<<"-d [datFile]: Specify a text file with the names of dat files on each line."<<endl;
		cout<<"fileNames: Specify individual data files on the command line."<<endl;
		cout<<"-h: Print expanded help."<<endl;
		cout<<"Note: file names with spaces should be surrounded with quotes."<<endl;
		return;
	}
	cout<<"BCI2000Certification.exe usage and expected parameters, and explains the BCI2000Certification.ini file:"<<endl<<endl;
	cout<<"The latency analysis program must be called with one"<<endl;
	cout<<"of three parameters, specifying either a BCI2000 prm"<<endl;
	cout<<"file (-p), a text file containing a listof *.dat"<<endl;
	cout<<"files, or specific *.dat files. Details are below."<<endl<<endl;

	cout<<"-p [*.prm file]: The argument following the -p"<<endl;
	cout<<"\tparameter is the name ofa BCI2000 parameter"<<endl;
	cout<<"\tfile. This file must contain the DataDirectory"<<endl;
	cout<<"\tparameter, which specifies the directory in"<<endl;
	cout<<"\twhich the data to be analyzed is stored. All"<<endl;
	cout<<"\tBCI2000 *.dat files contained under this"<<endl;
	cout<<"\tdirectory will potentially be analyzed,"<<endl;
	cout<<"\tdepending on the contents of the"<<endl;
	cout<<"\tBCI2000Certification.ini file, where task types"<<endl;
	cout<<"\tare defined based on the names of SessionFolders."<<endl;
	cout<<"\tFile names with spaces should be enclosed with double"<<endl;
	cout<<"\tquotation marks."<<endl<<endl;
	cout<<"\texample:"<<endl;
	cout<<"\tBCI2000Certification -p \"c:\\BCI2000\\parms\\subject01.prm\""<<endl<<endl;

	cout<<"-d [textFile]: The textFile is the location of"<<endl;
	cout<<"\ta text file whose contents are the names"<<endl;
	cout<<"\tindividual *.dat files, one file per line"<<endl;
	cout<<"\tIn this file, the file names do not need"<<endl;
	cout<<"\tto be enclosed with quotations, even if"<<endl;
	cout<<"\tthe name contains spaces. However, if the"<<endl;
	cout<<"\tfile name itself contains spaces, it should"<<endl;
	cout<<"\tbe enclosed in double quotes."<<endl<<endl;
	cout<<"\texample:"<<endl;
	cout<<"\tBCI2000Certification -d \"c:\\Users\\BCI\\My Documents\\datList.txt\""<<endl<<endl;

	cout<<"[fileNames]: You can explicitly list *.dat files"<<endl;
	cout<<"\tto be analyzed at the command line as well. As"<<endl;
	cout<<"\tin the other cases, file names with spaces should be"<<endl;
	cout<<"\tenclosed in double quotes."<<endl<<endl;
	cout<<"\texample:"<<endl;
	cout<<"\tBCI2000Certification \"c:\\BCI2000\\data\\test001\\test001R01.dat\" \"c:\\BCI2000\\data\\test001\\test001R02.dat\""<<endl<<endl;

	cout<<"------------------------------------------------"<<endl;
	cout<<"BCI2000Certification.ini file: This file contains the test definitions that will determine how each *.dat file is analyzed."<<endl;
	cout<<"It has the general following structure:"<<endl<<endl;
	cout<<"\tName testName"<<endl;
	cout<<"\tFolder testFolder"<<endl;
	cout<<"\tamp ch"<<endl;
	cout<<"\tvid ch state stateVal"<<endl;
	cout<<"\taud ch state stateVal"<<endl;
	cout<<"\tstates state1 state2"<<endl;
	cout<<"\tparms parm1 parm2"<<endl;
	cout<<"\tend"<<endl;
	cout<<"Of these parameters, the Name, Folder, and End are the only required ones. The BCI2000Certification.ini file ";
	cout<<"can contain multiple analysis definitions; the analysis that will be performed ";
	cout<<"on a given data set is inferred from the parameters and states in that data set. ";
	cout<<"Parameter specifications for the BCI2000Certification.ini file are listed below."<<endl<<endl;
	cout<<"REQUIRED PARAMETERS:"<<endl;
	cout<<"Name:\tThe name of the test that will be run. This is displayed at the final"<<endl;
	cout<<"\tprint-out, and can be used to distinguish between similar tasks."<<endl;
	cout<<"Folder:\tThe folder in which this type of data can be found. This"<<endl;
	cout<<"\tis taken from the SubjectName parameter in the BCI2000 dat file."<<endl;
	cout<<"\tThis allows tests using the same BCI2000 modules to be used in"<<endl;
	cout<<"\tdifferent configurations, so that different states can be tested"<<endl;
	cout<<"\tif desired."<<endl;
	cout<<"End:\tThis specifies the end of the current task definition, and"<<endl;
	cout<<"\tmust be present."<<endl;
	cout<<"OPTIONAL PARAMETERS:"<<endl;
	cout<<"amp [CH]: This performs an amp latency analysis, using the ch"<<endl;
	cout<<"\tparameter provided. The ch is usually set to 0, although this"<<endl;
	cout<<"\tcan be changed for different configurations. If amp is not specified,"<<endl;
	cout<<"\tthe amp analysis will not be done; note that this will also prevent"<<endl;
	cout<<"\tany output analysis from being completed as well."<<endl;
	cout<<"vid [CH] [STATE] [STATEVAL]: This performs a video output analysis"<<endl;
	cout<<"\ton the CH specified. It uses the STATE specified and looks for"<<endl;
	cout<<"\tchanges in this STATE to the STATEVAL specified. For example,"<<endl;
	cout<<"\tif STATE was TargetCode and STATEVAL was 3, the analysis would"<<endl;
	cout<<"\tlook for changes in the TargetCode state to 3, and look for"<<endl;
	cout<<"\ta rising edge in the data recorded on the CH channel. STATEVAL can"<<endl;
	cout<<"\talso be set to 0 to use the values for EVERY state transition, or"<<endl;
	cout<<"\tto -1 to use the fastest (lowest mean time) state, although this"<<endl;
	cout<<"\tmight not produce optimal results in the case of outliers."<<endl;
	cout<<"aud [CH] [STATE] [STATEVAL]: This performs an auditory analysis"<<endl;
	cout<<"\tusing the data recorded on channel CH using STATE and STATEVAL"<<endl;
	cout<<"\tas triggers. This is otherwise identical to the vid parameter."<<endl;
	cout<<"states [STATE1 STATE2...]: Specifies additional states that MUST"<<endl;
	cout<<"\tbe present in the file in order for the file to qualify"<<endl;
	cout<<"\tas this test type. If the Folder parameter matches the"<<endl;
	cout<<"\tSubjectName, but the states listed are not present, then this"<<endl;
	cout<<"\tfile will still not be listed as this test type. This will be"<<endl;
	cout<<"\tuseful for custom tasks which may be based on existing tasks"<<endl;
	cout<<"\tand that contain new states, and should use a different"<<endl;
	cout<<"\tdifferent test analysis than the original task."<<endl;
	cout<<"parms [PARM1 PARM2...]: A list of parameters that MUST be present"<<endl;
	cout<<"\tin the data file in order for it to qualify as this analysis type."<<endl;
}

/*----------------------------------------------
parseInput
description: parses command line parameters
input: int argc, char* argv[] - standard c/c++ input arguments
	string datDir - the path of the default data directory, specified in the .cfg file
output: vector<string> - the array of files to be tested, dependent on the input parameters
*/
vector<string> parseInput(int argc, char* argv[],string datDir)
{
	//setup variables to use
    vector<string> fnames;
    int pos = 1;
    bool doParms = false;
    bool doFile = false;
	bool doList = false;
	bool useDefDir = (argc < 2);

    int parmPos = 2;

	//go through each argument until there are none left
	//this first loop is a first pass, checking for general errors in syntax and setting up
	//what to do on the 2nd pass
    while (pos < argc)
    {
        if (strcmp(argv[pos],"-f") == 0)
        {
			//the -f parameter means that the next parm should be a file name containing the names of files
			//to analyze
            pos++;
            if (pos >= argc)
            {
				printHelp(0);
                exit(0);
            }
            doFile = true;
		}
		else if (strcmp(argv[pos],"-h") == 0)
		{
			//-h means print verbose help
			printHelp(1);
			exit(0);
		}
        else if (strcmp(argv[pos],"-p") == 0)
        {
			//-p means a specific parameter file is used to find files to analyze
            pos++;
            if (pos >= argc)
            {
				printHelp(0);
                exit(0);
            }
            parmPos = pos;
            doParms = true;
		}
		else if (useDefDir)
		{
			//if there are no parameters, then use the default data directory
		}
        else
        {
			//if there are additional arguments, then the program assumes that these are file names to be
			//explicitly analyzed
            if (pos >= argc)
            {
				printHelp(0);
                exit(0);
            }
            doList = true;
        }
        pos++;
    }

	//only one analyis type can be used, so if more than one argument is used, print an error
	if ((doParms && doList) || (doParms && doFile) || (doList && doFile))
    {
		printHelp(1);
        exit(0);
    }

	//from here, we create the list of files to analyze
    pos = 1;
    if (doList)
    {
		//each argument is a dat file to be analyzed
		//the actual analysis program later handles checking whther the file exists, and if it is valid
		//so there is nothing left to do except add the file names
        for (int i = 1; i < argc; i++)
            fnames.push_back(argv[i]);
        return fnames;
    }
    if (doParms)
    {
		//use a parameter file to get the directory location, and get all possible dat files
        char ftmp[512];
        ifstream file;
    	fnames = parseParm(argv[parmPos]);
        return fnames;
	}
	if (useDefDir)
	{
		//use the default directory, and find all dat files
		string fullpath = getFullDir(datDir);
		parseDir(fullpath, fnames);
	}
    if (doFile)
    {
		//extract file names from a text file
		//each line contains a separate dat file
		//existence and validity are checked later
        pos++;
        fnames.clear();
        string line;
        ifstream file;
        file.open(argv[pos]);
		while(getline(file, line))
		{
			fnames.push_back(line);
		}
        file.close();
        return fnames;
    }
    return fnames;
}







/*---------------------------------------
main
description: the main function; handles I/O, initialization, and calles the analysis routines
input: command-line args
output: none (print to screen, and to the log file)
*/
int main(int argc, char* argv[])
{
	//initial variables
	vector<string> fnames;
	Tasks taskTypes;
	vector<basicStats> minReqs;
	double thresh;
	string outfilepath;
	string datDir;

	//parse the configuration, input, and ini files to get the list of files to analyze
	if (!parseCfg(thresh, outfilepath, datDir, minReqs))
        exit(-1);

	fnames = parseInput(argc, argv, datDir);
	taskTypes = parseIni();

	//open the result file output stream, and quit if there is an error
	ofstream resOut;
	resOut.open(outfilepath.c_str(), ios::trunc | ios::out);
	if (!resOut.is_open())
	{
		cout << "Error opening output file at: "<< outfilepath <<". Quitting."<<endl;
		exit(0);
	}

	//these keep track of the analyses to run, the file names, etc
    vector< analysis*> analyses;

	//if no file names were found based on the input args given, quit gracefully
    if (fnames.size() == 0)
    {
        cout << "No file names specified or found. Quitting."<<endl;
        exit(0);
	}

	//character strings that hold the date and time for logging purposes
	char dateStr[10];
	char timeStr[10];
	_strdate(dateStr);
	_strtime(timeStr);

    cout <<"* Writing results to " << outfilepath<<endl;
	//write the header info to the log and the screen
	resOut <<dateStr<<" - "<<timeStr<<endl;
	resOut << "\nStarting BCI2000 Latency Analysis";
	resOut << "\n---------------------------------"<<endl;
	cout << "\nStarting BCI2000 Latency Analysis";
	cout << "\n---------------------------------"<<endl;

	//run the analysis on each file found
    for (unsigned int i = 0; i < fnames.size(); i++)
    {
		//create an analysis object to use for this file
        analysis *an = new analysis;

		//open the file; this checks if the file is valid/exists, loads the data (signal, states, parms)
		//and determines the task type based on the definitions loaded from the ini file
		if (!an->open(fnames[i], taskTypes))
        {
            //if there is an error or the file cannot be classified, just continue with the next one
            if (an->getTaskName() == "Unknown")
            {
                resOut <<fnames[i] <<": task undefined in BCI2000Certification.ini file. Skipping"<<endl;
                continue;
            }
            if (an->getTaskName() == "")
            {
                resOut <<fnames[i] <<": cannot find file. Skipping."<<endl;
                continue;
            }
        }

		//add the task to the checked types, which is used later for output
		//checkedTypes.push_back(thisTask);
        //update the stdio and log


        if (an->getSkip())
        {
            resOut << "Skipping " <<shortFname(fnames[i]) <<"..." <<endl;
    		cout << "Skipping " <<shortFname(fnames[i]) <<"..." <<endl;
            continue;
        }
        else
        {
            resOut << "Testing " <<shortFname(fnames[i]) <<"..." <<endl;
    		cout << "Testing " <<shortFname(fnames[i]) <<"..." <<endl;
        }

		//actually run the analysis on this task
        an->doThreshAnalysis();

		//close out the analysis
        analyses.push_back(an);
        //an.clear();
    }

	//finally, determine whether the system is BCI2000 compliant
	resOut<<endl<<"--------------------------------------------"<<endl;
	resOut<<"Results: (mean, std, min, max)"<<endl;
	cout<<"Results..."<<endl;
	//the analyses vector now contains an array for the results for each file
	//the results for each file contains results based on what was tested (e.g., video, audio, metronome, etc)
	//based on what was specified in the ini file
	//these results are compared against the minimum requires specified in the cfg file, and
	//compliance determination is output to the log and stdio
	for (unsigned int i = 0; i < analyses.size(); i++)
	{
        if (!analyses[i]->getSkip())
            analyses[i]->print(resOut, minReqs);
	}
	//close the log file
	resOut.close();

    //clear the analyses
    for (int i = 0; i < analyses.size(); i++)
        delete analyses[i];

    return 0;
}
//---------------------------------------------------------------------------
