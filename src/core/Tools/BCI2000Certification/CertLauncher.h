////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: Adam Wilson, University of Wisconsin-Madison
// Date: 12-16-07
// The CertLauncher handles several responsibilities, including:
// - parsing the BCI2000Certification.ini file for task info
//     e.g., which programs to run
// - running the programs for each configuration
// - monitoring processes and logging CPU load
//
// It is designed to be as platform independent as possible, but
// #DEFINE statements are used where necessary, particularly when
// programs need to be launched, and in the process monitoring functions
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
Author: Adam Wilson, University of Wisconsin-Madison
Date: 12-16-07
The CertLauncher handles several responsibilities, including:
- parsing the BCI2000Certification.ini file for task info
    e.g., which programs to run
- running the programs for each configuration
- monitoring processes and logging CPU load

It is designed to be as platform independent as possible, but
#DEFINE statements are used where necessary, particularly when
programs need to be launched, and in the process monitoring functions

$BEGIN_BCI2000_LICENSE$

This file is part of BCI2000, a platform for real-time bio-signal research.
[ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]

BCI2000 is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

BCI2000 is distributed in the hope that it will be useful, but
                        WITHOUT ANY WARRANTY
- without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

$END_BCI2000_LICENSE$
*/
//---------------------------------------------------------------------------

#ifndef CertLauncherH
#define CertLauncherH

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>

//here is windows stuff
#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <tchar.h>

#include "TaskType.h"

/*class TaskConfig {
public:
    string name;
    string SignalSource;
    string SigProc;
    string App;
    string parmFile;
    string folder;

    void clear()
    {
        name = "";
        SignalSource="";
        SigProc = "";
        App="";
        parmFile="";
        folder="";
    }
};*/

class CertLauncher
{
private:

    bool mTasksRemaining;
    int mCurTask;
	string getLogFile();

public:
    CertLauncher();
	~CertLauncher();
	Tasks tasks;

	bool nextTask();
	void reset(){mCurTask = -1;}
    bool tasksRemain(){return mTasksRemaining;}

	bool parseIni();
	bool parseIni(string);
    bool launchProgs();
    bool monitorProgs();

    int nTasks(){return tasks.size();}
    int taskReturnCode(){return tasks.getReturnCode();}

	TaskType& operator[](const int i){return tasks[i];}
	string GlobalSource(){return tasks.GlobalSource;}
	void setGlobalSource(string s){tasks.GlobalSource = s;}
	int GetCurrentTask(){return mCurTask;}
	string mDataDir;
	int mWinLeft, mWinTop, mWinWidth, mWinHeight;
	bool useWinLeft, useWinTop, useWinWidth, useWinHeight;
};
//---------------------------------------------------------------------------
#endif
