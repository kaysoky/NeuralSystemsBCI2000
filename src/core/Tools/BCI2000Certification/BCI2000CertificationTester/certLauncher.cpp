/*
Author: Adam Wilson, University of Wisconsin-Madison
Date: 12-16-07

$BEGIN_BCI2000_LICENSE$

This file is part of BCI2000, a platform for real-time bio-signal research.
[ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]

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

#pragma hdrstop

#include "certLauncher.h"

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
    return parseIni("BCI2000Certification.ini");
}
bool CertLauncher::parseIni(QString file)
{
	tasks.clear();
    tasks.init(file);

    if (tasks.getReturnCode() != 0)
        return false;
        
	QList<int> invalidTasks;
	bool allTasksOK = tasks.checkTasks(invalidTasks);
        
    mCurTask = -1;
    mTasksRemaining = (tasks.size() > 0);

    return allTasksOK;
}

bool CertLauncher::launchProgs()
{	
	operat = new QProcess;
	source = new QProcess;
	sigproc = new QProcess;
	app = new QProcess;

	TaskType curTask = tasks[mCurTask];
	QString comm;
	QDir curDir = QDir::current();
	
	
	comm.clear();
	comm += QString("%1/%2 --OnConnect \"-LOAD PARAMETERFILE ").arg(tasks.getProgPath()).arg(tasks.getOperator());
	comm += QString("%1/tools/BCI2000Certification/parms/CertificationMain.prm;").arg(tasks.getBCI2000Path());
    for (int i = 0; i < curTask.getParmFiles().size(); i++)
    {
        comm += " LOAD PARAMETERFILE ";
		comm += QString("%1/tools/BCI2000Certification/parms/%2;").arg(tasks.getBCI2000Path()).arg(curTask.getParmFiles()[i]);
    }

	if (tasks.getAutoCfg())
		comm += " SETCONFIG;\"";
	else
		comm += "\"";

	if (tasks.getAutoStart())
		comm += " --OnSetConfig \"-SET STATE Running 1;\""; 

	if (tasks.getAutoQuit())
		comm += " --OnSuspend \"-Quit;\"";

	qDebug() << comm;
	//tmp = comm.readAll();
	operat->setWorkingDirectory(tasks.getProgPath());
	operat->start(comm);

	if (!operat->waitForStarted(3000))
		return false;

	comm.clear();
	comm += QString("%1/%2 127.0.0.1").arg(tasks.getProgPath()).arg(curTask.getSigProc());
	sigproc->start(comm);
	qDebug() << comm;
	if (!sigproc->waitForStarted(3000))
		return false;

	comm.clear();
	comm += QString("%1/%2 127.0.0.1").arg(tasks.getProgPath()).arg(curTask.getApp());
	if (tasks.useWindowLeft())
		comm += QString(" --WindowLeft-%1").arg(tasks.getWindowLeft());
	if (tasks.useWindowTop())
		comm += QString(" --WindowTop-%1").arg(tasks.getWindowTop());
	if (tasks.useWindowWidth())
		comm += QString(" --WindowWidth-%1").arg(tasks.getWindowWidth());
	if (tasks.useWindowHeight())
		comm += QString(" --WindowHeight-%1").arg(tasks.getWindowHeight());

	qDebug() << comm;
	app->start(comm);
	if (!app->waitForStarted(3000))
		return false;

	comm.clear();
	if (curTask.getSource().size() > 0)
		comm += QString("%1/%2 127.0.0.1").arg(tasks.getProgPath()).arg(curTask.getSource());
	else
		comm += QString("%1/%2 127.0.0.1").arg(tasks.getProgPath()).arg(tasks.getGlobalSource());	
	
	comm += " --SubjectName-" + curTask.getTaskName();

	if (curTask.getSampleRate() > 0)
		comm += QString(" --SamplingRate-%1").arg(curTask.getSampleRate());

	if (curTask.getBlockSize() > 0)
		comm += QString(" --SampleBlockSize-%1").arg(curTask.getBlockSize());

	if (mDataDir != "")
		comm += " --DataDirectory-" + mDataDir;

	qDebug() << comm;
	source->start(comm);
	if (!source->waitForStarted(3000))
		return false;
	
	operat->waitForFinished(-1);
	source->waitForFinished(-1);
	sigproc->waitForFinished(-1);
	app->waitForFinished(-1);
	delete operat;
	delete source;
	delete sigproc;
	delete app;
	return true;
}

//---------------------------------------------------------------------------
#ifdef __BORLANDC__
# pragma package(smart_init)
#endif // __BORLANDC__
