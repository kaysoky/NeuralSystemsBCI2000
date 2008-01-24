/*
Author: Adam Wilson, University of Wisconsin-Madison
Date: 12-16-07

This is the flowchart for

(C) 2000-2008, BCI2000 Project
http://www.bci2000.org
*/
//---------------------------------------------------------------------------

#pragma hdrstop

#include "CertLauncher.h"

CertLauncher::CertLauncher()
{
    mTasksRemaining = false;
    mCurTask = -1;
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
    tasks.init("BCI2000Certification.ini");

    bool allTasksOK = true;
    for (unsigned int i = 0; i < tasks.size(); i++)
    {
        bool curTaskOK = true;

        if (tasks[i].taskName == ""){
            allTasksOK = false;
            curTaskOK = false;
        }
        if (tasks[i].SignalSource == ""){
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
        if (tasks[i].parmFile == ""){
            allTasksOK = false;
            curTaskOK = false;
        }
        if (tasks[i].taskFolder == ""){
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
    mCurTask = 0;
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
    operat << " .." << fs << "tools" << fs << "BCI2000Certification"<<fs<<"parms" << fs << "latencyTest.prm;";
    operat << " LOAD PARAMETERFILE ";
    operat << " .." << fs << "tools" << fs << "BCI2000Certification"<<fs<<"parms" << fs << curTaskC.parmFile <<"; SETCONFIG;\"";
    operat << " --OnSuspend \"-Quit\";";


    //cout << operat.str() <<endl;

    //system("cd .." << fs << ".." << fs << "prog");
    //system("dir /w");
    system(operat.str().c_str());

    //wait a bit...
    for (int i = 0 ;i < 1000000; )
        i++;

    //launch each module
    comm.str("");
    comm << "start .." << fs << ".." << fs << "prog" << fs << ""<<curTaskC.SignalSource << " 127.0.0.1"<<endl;
    system(comm.str().c_str());

    comm.str("");
    comm << "start .." << fs << ".." << fs << "prog" << fs << ""<<curTaskC.SigProc << " 127.0.0.1"<<endl;
    system(comm.str().c_str());

    Sleep(500);
    comm.str("");
    comm << "start .." << fs << ".." << fs << "prog" << fs << ""<<curTaskC.App << " 127.0.0.1"<<endl;
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
    string fs = "\\";
    string prmPath = "parms" + fs + tasks[mCurTask].parmFile;
    stringstream logFile;
    logFile << "latencyData" << fs;
    string subjName = "";
    //prepare to read and parse the parm file
    ifstream in;
    in.open(prmPath.c_str());
    if (!in.is_open())
        return "";

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
    in.close();

    if (subjName == "")
        return "";
    //just assume it is session1
    logFile << subjName << "001" << fs << subjName <<"_CPU.log";
    return logFile.str();
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
