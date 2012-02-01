////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: Adam Wilson, University of Wisconsin-Madison
// Date: 12-16-07
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

#include "CertLauncher.h"

CertLauncher::CertLauncher()
{
    mTasksRemaining = false;
	mCurTask = -1;
	mDataDir = "";
	mWinLeft = mWinTop = mWinWidth = mWinHeight = 0;
	useWinLeft = useWinTop = useWinWidth = useWinHeight = false;
    //tasks.clear();
}

CertLauncher::~CertLauncher()
{}

bool CertLauncher::nextTask()
{
    while (true)
    {
        mCurTask++;
        if (mCurTask >= (int)tasks.size())
        {
            mTasksRemaining = false;
            return mTasksRemaining;
        }
        if (!tasks[mCurTask].skip)
        {
            mTasksRemaining = true;
            return mTasksRemaining;
        }
    }
}

bool CertLauncher::parseIni()
{
    parseIni("BCI2000Certification.ini");
}
bool CertLauncher::parseIni(string file)
{
	tasks.clear();
    tasks.init(file);

    if (tasks.getReturnCode() != 0)
        return false;

    bool allTasksOK = true;
    for (unsigned int i = 0; i < tasks.size(); i++)
    {
        bool curTaskOK = true;

		if (tasks[i].taskName == "" ){
            allTasksOK = false;
            curTaskOK = false;
        }
        if (tasks[i].SignalSource == "" && tasks.GlobalSource == ""){
            allTasksOK = false;
            curTaskOK = false;
        }
        if (tasks[i].SigProc == ""){
            allTasksOK = false;
            curTaskOK = false;
        }
        if (tasks[i].App == ""){
            allTasksOK = false;
            curTaskOK = false;
        }
        if (tasks[i].parmFile.size() == 0){
            allTasksOK = false;
            curTaskOK = false;
        }

        if (!curTaskOK)
        {
            tasks[i].skip = true;
        }
        else
        {
            //it++;
        }
    }

    mCurTask = -1;
    mTasksRemaining = (tasks.size() > 0);

    return allTasksOK;
}



bool CertLauncher::launchProgs()
{
    //check that the programs exist

    //----------------------------

    string fs = "\\";

    TaskType curTaskC = tasks[mCurTask];
    stringstream operat, comm;
    operat <<"cd .." << fs << ".." << fs << "prog & ";
    operat << "start operat --OnConnect \"-LOAD PARAMETERFILE ";
    operat << " .." << fs << "tools" << fs << "BCI2000Certification"<<fs<<"parms" << fs << "CertificationMain.prm;";
    for (int i = 0; i < curTaskC.parmFile.size(); i++)
    {
        operat << " LOAD PARAMETERFILE ";
		//operat << " .." << fs << "tools" << fs << "BCI2000Certification"<<fs<<"parms" << fs << curTaskC.parmFile[i] <<";";
		operat <<  curTaskC.parmFile[i] <<";";
    }

	operat <<" SETCONFIG;\"";
	operat <<" --OnSetConfig \"-SET STATE Running 1\"";
    operat << " --OnSuspend \"-Quit\";";


    //cout << operat.str() <<endl;

    //system("cd .." << fs << ".." << fs << "prog");
    //system("dir /w");
    system(operat.str().c_str());

    //wait a bit...
	Sleep(500);


    comm.str("");
    comm << "start .." << fs << ".." << fs << "prog" << fs << ""<<curTaskC.SigProc << " 127.0.0.1"<<endl;
    system(comm.str().c_str());

    Sleep(500);
    comm.str("");
	comm << "start .." << fs << ".." << fs << "prog" << fs << ""<<curTaskC.App << " 127.0.0.1";
	if (useWinLeft)
		comm << " --WindowLeft-" << mWinLeft;
	if (useWinTop)
		comm << " --WindowTop-" << mWinTop;
	if (useWinWidth)
		comm << " --WindowWidth-" << mWinWidth;
	if (useWinHeight)
		comm << " --WindowHeight-" << mWinHeight;
	comm << endl;
	system(comm.str().c_str());

	//launch each module
	comm.str("");
	if (curTaskC.SignalSource != "")
		comm << "start .." << fs << ".." << fs << "prog" << fs << curTaskC.SignalSource;
	else
		comm << "start .." << fs << ".." << fs << "prog" << fs << tasks.GlobalSource;

	comm <<" --SubjectName-"<<curTaskC.taskName;

	if (curTaskC.sampleRate > 0)
		comm << " --SamplingRate-" << curTaskC.sampleRate;

	if (curTaskC.blockSize > 0)
		comm << " --SampleBlockSize-" << curTaskC.blockSize;

	if (mDataDir != "")
		comm << " --DataDirectory-" << mDataDir;


	Sleep(100);
	string tmp(comm.str());
	cout << tmp;
	system(comm.str().c_str());
    return true;
}

bool CertLauncher::monitorProgs()
{
    // get the output file to write the log data to
    string logFile = getLogFile();

    //remove the .exe from each program first
    int pos = 0;
    pos = tasks[mCurTask].SignalSource.find(".exe");
    if (pos >= 0)
        tasks[mCurTask].SignalSource.erase(pos, string::npos);
    pos = tasks[mCurTask].SigProc.find(".exe");
    if (pos >= 0)
        tasks[mCurTask].SigProc.erase(pos, string::npos);
    pos = tasks[mCurTask].App.find(".exe");
    if (pos >= 0)
        tasks[mCurTask].App.erase(pos, string::npos);

    //call the platform dependent cpu monitoring script
    stringstream comm;
    comm << "start cscript cpuMon.vbs " << tasks[mCurTask].SignalSource << " ";
    comm << tasks[mCurTask].SigProc << " " << tasks[mCurTask].App << " ";
    comm << logFile << " " << "1000";
    system(comm.str().c_str());

    //this function runs until the processes have exited
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;
    bool procNotFound = false;
    while (!procNotFound)
    {
        procNotFound = true;
        if (!EnumProcesses (aProcesses, sizeof(aProcesses), &cbNeeded) )
            return false;

        cProcesses = cbNeeded / sizeof(DWORD);

        Sleep(1000);
        for (i = 0; i < cProcesses; i++)
        {
            if (aProcesses[i] != 0)
            {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                                false, aProcesses[i]);
                char szProcessName[MAX_PATH] = "<unknown>";
                if (hProcess != NULL)
                {
                    HMODULE hMod;
                    DWORD cbNeeded2;

                    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded2))
                    {
                        GetModuleBaseName(hProcess, hMod, szProcessName,
                                sizeof(szProcessName)/sizeof(TCHAR));
                    }
                }
                string procName(szProcessName);
                CloseHandle(hProcess);

                if(procName == "operat.exe")
                    procNotFound = false;
            }
        }
        if (procNotFound)
        {
            //check if wscript is running?
            return true;
        }
    }
}

//-------------------------------------
string CertLauncher::getLogFile()
{
    //setup the files
    /*
    string prmPath = "parms" + fs + tasks[mCurTask].taskName;

    logFile << "latencyData" << fs;
    string subjName = tasks[mCurTask].taskName;
    //prepare to read and parse the parm file
    ifstream in;
    in.open(prmPath.c_str());
    if (!in.is_open())
        return "";  */

        /*
    string line;
    while (getline(in, line) && subjName == "")
    {
		//setup the string stream, and tokenize the first...um...token
        stringstream ss(line);
        string strTok;
        ss >> strTok;
        ss >> strTok; // we want the parameter name, not it's category
        ss >> strTok;
        if (strTok == "SubjectName=")
            ss >> subjName;
    }
    in.close();  */

    //if (subjName == "")
    //    return "";
    //just assume it is session1
    string fs = "\\";
    stringstream logFile;
    logFile << "Data" <<fs<<tasks[mCurTask].taskName << "001" << fs << tasks[mCurTask].taskName <<"_CPU.log";
    return logFile.str();
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
