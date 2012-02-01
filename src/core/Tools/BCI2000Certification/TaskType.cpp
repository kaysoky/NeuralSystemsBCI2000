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

#include "TaskType.h"
#include <iostream>
#include <string>
#include <QFile>
#include <QTextStream>
#include <QDir>


using namespace std;
//----------------------------------------
Tasks::Tasks()
{
	clear();
	mUseWinLeft = mUseWinTop = mUseWinWidth = mUseWinHeight = false;
}


//----------------------------------------
Tasks::Tasks(QString fname)
{
	clear();
	init(fname);
	mUseWinLeft = mUseWinTop = mUseWinWidth = mUseWinHeight = false;
}

//----------------------------------------
Tasks::~Tasks()
{
	clear();
}

/*---------------------------
parseIni
description: parses the BCI2000Certification.ini file, in which the task types are defined
input: NA
output: Tasks - an array of TaskType, which describes the task and how to analyze the data for that task
*/

//----------------------------------------
void Tasks::init(QString fname)
{
	QFile file(fname);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
		returnCode = -1;
		return;
	}

	QTextStream in(&file);
	QString err;
	//setup the initial tasks
	QString line;
	mGlobalSource = "";
	mGlobalSourceValid = false;
	mProgPathValid = false;

	mOperatorExe = "";
	//TaskType curTask;
	//initTaskType(curTask);

	QString strTok;
	bool exportData = false;

	//go through each line of the ini and parse it

	while (!in.atEnd()) {
		QString line = in.readLine();
		line = line.trimmed();
		if (line.size() >= 2){
			if (line.at(0) == '%'){
				continue;
			}
		}
		else{
			continue;
		}

		QStringList strToks = line.split(" ");
		//check that this is a valid non-empty line
		if (strToks.size() == 0)
			continue;

		//this is a global signal source, which will be set for any task that does not
		//specify one; individual tasks will overwrite this
		if (strToks[0].toLower() == "source")
		{
			if (strToks.size() >= 2)
				setGlobalSource(strToks[1], err);
		}
		else if (strToks[0].toLower() == "export")
		{
			exportData = true;
		}
		//go through and check out the possible acceptable tokens, and assign the values accordingly
		else if (strToks[0].toLower() == "name")
		{
			//create a new task, and give it its name
			TaskType curTask;

			curTask.mTaskName = strToks[1];

			curTask.exportData = exportData;
			//go through until the END is found
			bool atEnd = false;
			while (!in.atEnd() && !atEnd)
			{
				line = in.readLine();
				line = line.trimmed();
				if (line.size() == 0)
					continue;
				if (line[0] == '%')
					continue;

				strToks = line.split(" ");

				if (strToks[0].toLower() == "states")
				{
					for (int i = 1; i < strToks.size(); i++)
						curTask.mStates.push_back(strToks[i]);
				}

				else if (strToks[0].toLower() == "parms")
				{
					for (int i = 1; i < strToks.size(); i++)
						curTask.mParms.push_back(strToks[i]);
				}
				else if (strToks[0].toLower() == "amp")
				{
					if (strToks.size() >= 2)
						curTask.setAmpCh(strToks[1] ,err);
				}
				else if (strToks[0].toLower() == "damp")
				{
					if (strToks.size() >= 2)
						curTask.setDAmpCh(strToks[1] ,err);
				}
				else if (strToks[0].toLower() == "vid")
				{
					if (strToks.size() >= 4){
						curTask.setVidCh(strToks[1] ,err);
						curTask.setVidState(strToks[2] ,err);
						strToks.removeFirst();
						strToks.removeFirst();
						strToks.removeFirst();
						curTask.setVidVals(strToks.join(" ") ,err);
					}
				}

				else if (strToks[0].toLower() == "aud")
				{
					if (strToks.size() >= 4){
						curTask.setAudCh(strToks[1] ,err);
						curTask.setAudState(strToks[2] ,err);
						strToks.removeFirst();
						strToks.removeFirst();
						strToks.removeFirst();
						curTask.setAudVals(strToks.join(" ") ,err);
					}

				}
				else if (strToks[0].toLower() == "source")
				{
					if (strToks.size() >= 2)
						curTask.setSource(strToks[1] ,err);
				}
				else if (strToks[0].toLower() == "sigproc")
				{
					if (strToks.size() >= 2)
						curTask.setSigProc(strToks[1] ,err);
				}
				else if (strToks[0].toLower() == "app")
				{
					if (strToks.size() >= 2)
						curTask.setApp(strToks[1] ,err);
				}
				else if (strToks[0].toLower() == "parm")
				{
					if (strToks.size() >= 2)
						curTask.addParm(strToks[1]);
				}
				else if (strToks[0].toLower() == "samplingrate")
				{
					if (strToks.size() >= 2)
						curTask.setSampleRate(strToks[1] ,err);
				}
				else if (strToks[0].toLower() == "blocksize")
				{
					if (strToks.size() >= 2)
						curTask.setBlockSize(strToks[1] ,err);
				}
				else if (strToks[0].toLower() == "skip")
				{
					curTask.skip = true;
				}
				else if (strToks[0].toLower() == "end")
				{
					//the end token specifies that the current task definition is complete,
					//so add it to the array, and start a new task
					atEnd = true;
					this->push_back(curTask);
					//taskTypes.push_back(curTask);

					//initTaskType(curTask);
				}
				else if (strToks[0].toLower()  == "name")
				{
					// this is an error
					returnCode = -2;
					return;
				}
				else
				{
					//cout <<"Unrecognized token: "<< strTok<<". Skipping."<<endl;
				}
			} //end while getline
		}//end if strtok = "name"
	}
	file.close();

	//go through all tasks and set the global source for any that weren't specified
	/*
	for (size_t i = 0; i < this->size(); i++)
	{
		if ((*this)[i].mSignalSource == "" && mGlobalSourceValid)
			(*this)[i].mSignalSource = mGlobalSource;
	} */

	//now check for duplicate task names
	for (unsigned int i = 0; i < this->size(); i++)
	{
		for (unsigned int j = i+1; j < this->size(); j++)
		{
			if ((*this)[i].mTaskName.compare((*this)[j].mTaskName) == 0)
			{
				returnCode = -3;
				return;
			}
		}
	}

	returnCode = 0;
}

bool Tasks::checkTasks(QList<int> &list)
{
	bool allTasksOK = true;
	for (unsigned int i = 0; i < this->size(); i++)
	{
		if (!(*this)[i].checkErrors()){
			allTasksOK = false;
			list.push_back(i);
		}
	}
	return allTasksOK;
}



bool Tasks::writeIni(QString fname)
{
	QFile file(fname);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	QTextStream out(&file);

	//first write the global setting if it exists
	if (mGlobalSource.size() > 0)
		out << "source " << mGlobalSource << "\n\n";

	for (size_t i = 0; i < this->size(); i++)
	{
		TaskType tmpTask = (*this)[i];
		out << "Name " << tmpTask.getTaskName() << "\n";
		if (!tmpTask.skip)
			out << "%";
		out << "skip"<<"\n";


		if (tmpTask.getAmpFlag())
			out << "amp " << tmpTask.getAmpCh() << "\n";


		if (tmpTask.getDAmpFlag())
			out << "damp " << tmpTask.getDAmpCh() << "\n";

		if (tmpTask.getVidFlag()){
			out << "vid " << tmpTask.getVidCh() << " " << tmpTask.getVidState();
			for (size_t k = 0; k < tmpTask.getVidVals().size(); k++)
				out << " " << tmpTask.getVidVals()[k];
			out << "\n";
		}

		if (tmpTask.getAudFlag()){
			out << "vid " << tmpTask.getAudCh() << " " << tmpTask.getAudState();
			for (size_t k = 0; k < tmpTask.getAudVals().size(); k++)
				out << " " << tmpTask.getAudVals()[k];
			out << "\n";
		}

		if (tmpTask.getSource().size() > 0)
			out << "source " << tmpTask.getSource() << "\n";

		if (tmpTask.getSigProc().size() > 0)
			out << "sigproc " << tmpTask.getSigProc() << "\n";

		if (tmpTask.getSampleRate() > 0)
			out << "samplingrate " << tmpTask.getSampleRate() << "\n";

		if (tmpTask.getBlockSize() > 0)
			out << "blocksize " << tmpTask.getBlockSize() << "\n";

		if (tmpTask.getApp().size() > 0)
			out << "app " << tmpTask.getApp() << "\n";

		for (int j = 0; j < tmpTask.getParmFiles().size(); j++)
			out << "parm " << tmpTask.getParmFiles()[j] << "\n";

		/*
		for (size_t j = 0; j < tmpTask.parms.size(); j++)
			out << "parms " << tmpTask.parms[j] << "\n";

		for (size_t j = 0; j < tmpTask.states.size(); j++)
			out << "states " << tmpTask.states[j] << "\n";
		*/
		out << "end" << "\n\n";

	}

	file.close();

	return true;
}

TaskState Tasks::setProgPath(QString path, QString& err)
{
	QDir bci2000Path(path);
	mOperatorExe = "";
	mBCI2000Path = "";
	if (!bci2000Path.exists())
	{
		mProgPath = "";
		err = "This path does not exist";
		mProgPathValid = false;
		return /*TaskState::*/Invalid;
	}

	//check if the path contains operat.exe or operator.exe
	QStringList filters, exes;
	filters << "*.exe";
	bci2000Path.setNameFilters(filters);
	exes = bci2000Path.entryList(filters);
	if(exes.contains("operat.exe",Qt::CaseInsensitive)){
		err = "";
		mProgPathValid = true;
		mOperatorExe = "operat.exe";
		mProgPath = bci2000Path.absolutePath();
		bci2000Path.cdUp();
		mBCI2000Path = bci2000Path.absolutePath();
		return /*TaskState::*/Valid;
	}
	else if ( exes.contains("operator.exe",Qt::CaseInsensitive)){
		err = "";
		mProgPathValid = true;
		mOperatorExe = "operator.exe";
		mProgPath = bci2000Path.absolutePath();
		bci2000Path.cdUp();
		mBCI2000Path = bci2000Path.absolutePath();
		return /*TaskState::*/Valid;
	}
	else
	{
		mProgPath = "";
		mProgPathValid = false;
		err = "The path does not contain BCI2000 executables.";
		return /*TaskState::*/Invalid;
	}
}

TaskState Tasks::setGlobalSource(QString path, QString& err)
{
	if (!QDir::isAbsolutePath(path) && mProgPathValid){
		path = mProgPath + "/" + path;
	}
	mGlobalSource = path;
	mGlobalSourceValid = true;
	err = "";
	/*
	for (size_t i = 0; i < this->size(); i++)
	{
		if ((*this)[i].mSignalSource == "" && mGlobalSourceValid)
			(*this)[i].mSignalSource = mGlobalSource;
	} */
	return /*TaskState::*/Valid;
}

void TaskType::addParm(QString str)
{
	str.replace("\\", "/");
	mParmFiles << str;
	int pos = str.lastIndexOf("/");
	if (pos != -1)
		mParmFilesDisp << str.right(str.size()-pos-1);
	else
		mParmFilesDisp << str;

}

void TaskType::delParm(int id)
{

	if (id < 0 || id >= (int)mParmFiles.size())
		return;

	mParmFiles.removeAt(id);
	mParmFilesDisp.removeAt(id);
}

//-----------------------------------
TaskType::TaskType()
{
	mTaskName = "";
	mParms.clear();
	mParmFiles.clear();
	mParmFilesDisp.clear();
	mStates.clear();
	mSampleRate = 0;
	mBlockSize = 0;
	mAmp.ch = -1;
	mAmp.state = "";
	mAmp.stateVal.clear();
	mAmp.flag = false;
	mDAmp.ch = -1;
	mDAmp.state = "";
	mDAmp.stateVal.clear();
	mDAmp.flag = false;
	mVid.ch = -1;
	mVid.state = "";
	mVid.stateVal.clear();
	mVid.flag = false;
	mAud.ch = -1;
	mAud.state = "";
	mAud.stateVal.clear();
	mAud.flag = false;
	skip = false;
	exportData = false;
	for (int i = 0; i <= /*TaskMasks::*/BlockSize; i++)
		mErrors.push_back(false);
}

//-----------------------------------
TaskType::~TaskType()
{
}

//---------------------------------------------------------------------------

TaskState TaskType::setTaskName(QString name, QString& err)
{
	err = "";
	if (name.contains(QRegExp("\\s+")) || name.length() == 0){
		err = "The task name may not contain spaces and must have at least 1 character.";
		mErrors[/*TaskMasks::*/Amp] = true;
		return /*TaskState::*/Invalid;
	}

	mTaskName = name;
	mErrors[/*TaskMasks::*/Amp] = false;
	return /*TaskState::*/Valid;
}

TaskState TaskType::setTaskParms(QStringList str, QString& err)
{
	err = "";
	return /*TaskState::*/Valid;
}

TaskState TaskType::setTaskStates(QStringList str, QString& err)
{
	err = "";
	return /*TaskState::*/Valid;
}

TaskState TaskType::setSource(QString str, QString& err)
{
	mSignalSource = str;
	err = "";
	mErrors[/*TaskMasks::*/SignalSource] = false;
	return /*TaskState::*/Valid;
}

TaskState TaskType::setSigProc(QString str, QString& err){
	mSigProc = str;
	err = "";
	mErrors[/*TaskMasks::*/SigProc] = false;
	return /*TaskState::*/Valid;
}

TaskState TaskType::setApp(QString str, QString& err){
	mApp = str;
	err = "";
	mErrors[/*TaskMasks::*/App] = false;
	return /*TaskState::*/Valid;
}

TaskState TaskType::setAmpCh(int val, QString& err)
{
	if (val < 0){
		mAmp.flag = false;
		err = "The channel value must be >= 0";
		mErrors[/*TaskMasks::*/Amp] = true;
		return /*TaskState::*/Invalid;
	}
	mAmp.ch = val;
	mAmp.flag = true;
	mErrors[/*TaskMasks::*/Amp] = false;
	return /*TaskState::*/Valid;
}
TaskState TaskType::setAmpCh(QString val, QString& err)
{
	if (val.size() == 0){
		err = "";
		mAmp.flag = false;
		mErrors[/*TaskMasks::*/Amp] = false;
		return /*TaskState::*/Blank;
	}
	bool ok;
	int v = val.toInt(&ok);
	if (ok)
		return setAmpCh(v, err);

	err = "The channel value must be a valid integer >= 0";
	mErrors[/*TaskMasks::*/Amp] = true;
	mAmp.flag = false;
	return /*TaskState::*/Invalid;
}

TaskState TaskType::setDAmpCh(int val, QString& err)
{
	if (val < 0){
		err = "The channel value must be >= 0";
		mErrors[/*TaskMasks::*/DAmp] = true;
		mDAmp.flag = false;
		return /*TaskState::*/Invalid;
	}
	mDAmp.ch = val;
	mDAmp.flag = true;
	mErrors[/*TaskMasks::*/DAmp] = false;
	return /*TaskState::*/Valid;
}
TaskState TaskType::setDAmpCh(QString val, QString& err)
{
	if (val.size() == 0){
		err = "";
		mDAmp.flag = false;
		mErrors[/*TaskMasks::*/DAmp] = false;
		return /*TaskState::*/Blank;
	}
	bool ok;
	int v = val.toInt(&ok);
	if (ok)
		return setDAmpCh(v, err);

	err = "The channel value must be a valid integer > 0";
	mDAmp.flag = false;
	mErrors[/*TaskMasks::*/DAmp] = true;
	return /*TaskState::*/Invalid;
}

TaskState TaskType::setSampleRate(float val, QString& err)
{
	if (val <= 0){
		err = "The sample rate must be > 0";
		mErrors[/*TaskMasks::*/SampleRate] = true;
		return /*TaskState::*/Invalid;
	}
	mSampleRate = val;
	mErrors[/*TaskMasks::*/SampleRate] = false;
	return /*TaskState::*/Valid;
}
TaskState TaskType::setSampleRate(QString val, QString& err)
{
	bool ok;
	float v = val.toFloat(&ok);
	if (ok)
		return setSampleRate(v, err);

	err = "The sample rate must be a valid floating point value > 0";
	mErrors[/*TaskMasks::*/SampleRate] = true;
	return /*TaskState::*/Invalid;
}

TaskState TaskType::setBlockSize(int val, QString& err)
{
	if (val <= 0){
		err = "The block size must be an integer > 0";
		mErrors[/*TaskMasks::*/BlockSize] = true;
		return /*TaskState::*/Invalid;
	}
	mBlockSize = val;
	mErrors[/*TaskMasks::*/BlockSize] = false;
	return /*TaskState::*/Valid;

}
TaskState TaskType::setBlockSize(QString val, QString& err)
{
	bool ok;
	int v = val.toInt(&ok);
	if (ok)
		return setBlockSize(v, err);

	err = "The block size must be a valid integer value > 0";
	mErrors[/*TaskMasks::*/BlockSize] = true;
	return /*TaskState::*/Invalid;
}

TaskState TaskType::setVidCh(int val, QString& err)
{
	if (val < 0){
		err = "The video channel value must be >= 0";
		mVid.flag = false;
		mVid.ch = -1;
		mErrors[/*TaskMasks::*/VidCh] = true;
		return /*TaskState::*/Invalid;
	}

	mVid.ch = val;
	mErrors[/*TaskMasks::*/VidCh] = false;
	if (mVid.state.size() > 0 &&
		mVid.stateVal.size() > 0)
	{
		mVid.flag = true;
	}
	return /*TaskState::*/Valid;
}
TaskState TaskType::setVidCh(QString val, QString& err)
{
	bool ok;
	if (val.size() == 0 &&
		mVid.state.size() == 0 &&
		mVid.stateVal.size() == 0)
	{
		err = "";
		mVid.ch = -1;
		mVid.flag = false;
		mErrors[/*TaskMasks::*/VidCh] = false;
		return /*TaskState::*/Blank;
	}
	if (val.size() == 0 &&
		(mVid.state.size() > 0 ||
		mVid.stateVal.size() > 0)){
			mVid.flag = true;
			mVid.ch = -1;
			err = "This video parameter is required";
			mErrors[/*TaskMasks::*/VidCh] = true;
			return /*TaskState::*/SemiValid;
	}

	int v = val.toInt(&ok);
	if (ok)
		return setVidCh(v, err);

	mErrors[/*TaskMasks::*/VidCh] = true;
	err = "The video channel must be a valid integer value > 0";
	mVid.flag = false;
	return /*TaskState::*/Invalid;
}

TaskState TaskType::setVidState(QString str, QString& err)
{
	if (str.size() == 0 &&
		mVid.ch == -1 &&
		mVid.stateVal.size() == 0)
	{
		err = "";
		mVid.state.clear();
		mVid.flag = false;
		mErrors[/*TaskMasks::*/VidState] = true;
		return /*TaskState::*/Blank;
	}
	if (str.size() == 0 &&
		(mVid.ch >= 0 ||
		mVid.stateVal.size() > 0)){
			mVid.flag = false;
			mVid.state.clear();
			err = "This video parameter is required";
			mErrors[/*TaskMasks::*/VidState] = true;
			return /*TaskState::*/SemiValid;
	}
	if (str.contains(QRegExp("\\s+"))){
		err = "State names may not contain spaces.";
		mErrors[/*TaskMasks::*/VidState] = true;
		return /*TaskState::*/Invalid;
	}
	if (mVid.ch >= 0 &&
		mVid.stateVal.size() > 0)
	{
		mVid.flag = true;
	}
	mErrors[/*TaskMasks::*/VidState] = false;
	mVid.state = str;
	return /*TaskState::*/Valid;
}

TaskState TaskType::setVidVals(vector<int> vals, QString& err)
{
	err = "";
	mVid.stateVal = vals;
	mErrors[/*TaskMasks::*/VidVals] = false;
	if (mVid.state.size() > 0 &&
		mVid.ch >= 0)
	{
		mVid.flag = true;
	}
	return /*TaskState::*/Valid;
}
TaskState TaskType::setVidVals(QString str, QString& err)
{
	if (str.size() == 0 &&
		mVid.ch == -1 &&
		mVid.state.size() == 0)
	{
		err = "";
		mErrors[/*TaskMasks::*/VidVals] = false;
		mVid.stateVal.clear();
		mVid.flag = false;
		return /*TaskState::*/Blank;
	}
	if (str.size() == 0 &&
		(mVid.ch >= 0 ||
		mVid.state.size() > 0)){
			mVid.flag = false;
			mVid.stateVal.clear();
			err = "This video parameter is required";
			mErrors[/*TaskMasks::*/VidVals] = true;
			return /*TaskState::*/SemiValid;
	}
	mVid.stateVal.clear();
	QStringList tmpStrList = str.trimmed().split(QRegExp("\\s+"));
	vector<int> tmpVec;
	int tmp;
	bool ok = true;
	for (int i = 0; i < tmpStrList.size() && ok; i++){
		tmp = tmpStrList[i].toInt(&ok);
		if (ok){
			tmpVec.push_back(tmp);
		}
		else{
			err = "State values must be valid integers.";
			mErrors[/*TaskMasks::*/VidVals] = true;
			mVid.flag = false;
			mVid.stateVal.clear();
			return /*TaskState::*/Invalid;
		}
	}
	return setVidVals(tmpVec, err);
}

TaskState TaskType::setAudCh(int val, QString& err)
{
	if (val < 0){
		err = "The audio channel value must be >= 0";
		mAud.flag = false;
		mErrors[/*TaskMasks::*/AudCh] = true;
		mAud.ch = -1;
		return /*TaskState::*/Invalid;
	}

	mAud.ch = val;
	mErrors[/*TaskMasks::*/AudCh] = false;
	if (mAud.state.size() > 0 &&
		mAud.stateVal.size() > 0)
	{
		mAud.flag = true;
	}
	return /*TaskState::*/Valid;
}
TaskState TaskType::setAudCh(QString val, QString& err)
{
	bool ok;
	if (val.size() == 0 &&
		mAud.state.size() == 0 &&
		mAud.stateVal.size() == 0)
	{
		err = "";
		mAud.ch = -1;
		mErrors[/*TaskMasks::*/AudCh] = false;
		mAud.flag = false;
		return /*TaskState::*/Blank;
	}
	if (val.size() == 0 &&
		(mAud.state.size() > 0 ||
		mAud.stateVal.size() > 0)){
			mAud.flag = false;
			mErrors[/*TaskMasks::*/AudCh] = true;
			mAud.ch = -1;
			err = "This audio parameter is required";
			return /*TaskState::*/SemiValid;
	}

	int v = val.toInt(&ok);
	if (ok)
		return setAudCh(v, err);

	err = "The audio channel must be a valid integer value > 0";
	mErrors[/*TaskMasks::*/AudCh] = true;
	mAud.flag = false;
	return /*TaskState::*/Invalid;
}

TaskState TaskType::setAudState(QString str, QString& err)
{
	if (str.size() == 0 &&
		mAud.ch == -1 &&
		mAud.stateVal.size() == 0)
	{
		err = "";
		mErrors[/*TaskMasks::*/AudState] = false;
		mAud.state.clear();
		mAud.flag = false;
		return /*TaskState::*/Blank;
	}
	if (str.size() == 0 &&
		(mAud.ch >= 0 ||
		mAud.stateVal.size() > 0)){
			mAud.flag = false;
			mAud.state.clear();
			err = "This audio parameter is required";
			mErrors[/*TaskMasks::*/AudState] = true;
			return /*TaskState::*/SemiValid;
	}
	if (str.contains(QRegExp("\\s+"))){
		err = "State names may not contain spaces.";
		mErrors[/*TaskMasks::*/AudState] = true;
		return /*TaskState::*/Invalid;
	}
	mAud.state = str;
	mErrors[/*TaskMasks::*/AudState] = false;
	if (mAud.ch >= 0 &&
		mAud.stateVal.size() > 0)
	{
		mAud.flag = true;
	}
	return /*TaskState::*/Valid;
}

TaskState TaskType::setAudVals(vector<int> vals, QString& err)
{
	err = "";
	mAud.stateVal = vals;
	mErrors[/*TaskMasks::*/AudVals] = false;
	if (mAud.state.size() > 0 &&
		mAud.ch >= 0)
	{
		mAud.flag = true;
	}
	return /*TaskState::*/Valid;
}
TaskState TaskType::setAudVals(QString str, QString& err)
{
	if (str.size() == 0 &&
		mAud.ch == -1 &&
		mAud.state.size() == 0)
	{
		err = "";
		mErrors[/*TaskMasks::*/AudVals] = false;
		mAud.stateVal.clear();
		mAud.flag = false;
		return /*TaskState::*/Blank;
	}
	if (str.size() == 0 &&
		(mAud.ch >= 0 ||
		mAud.state.size() > 0)){
			mAud.flag = false;
			mAud.stateVal.clear();
			err = "This audio parameter is required";
			mErrors[/*TaskMasks::*/AudVals] = true;
			return /*TaskState::*/SemiValid;
	}
	mAud.stateVal.clear();
	QStringList tmpStrList = str.trimmed().split(QRegExp("\\s+"));
	vector<int> tmpVec;
	int tmp;
	bool ok = true;
	for (int i = 0; i < tmpStrList.size() && ok; i++){
		tmp = tmpStrList[i].toInt(&ok);
		if (ok){
			tmpVec.push_back(tmp);
		}
		else{
			err = "State values must be valid integers.";
			mErrors[/*TaskMasks::*/AudVals] = true;
			mAud.flag = false;
			mAud.stateVal.clear();
			return /*TaskState::*/Invalid;
		}
	}
	return setAudVals(tmpVec, err);
}

bool TaskType::checkErrors()
{
	bool curTaskOK = true;

	for (int i = 0; i < mErrors.size() && curTaskOK; i++){
		curTaskOK &= !mErrors[i];
	}
	return curTaskOK;
}

TaskState Tasks::setWindowLeft(int val, QString& err)
{
	err = "";
	mWinLeft = val;
	mUseWinLeft = true;
	return /*TaskState::*/Valid;
}

TaskState Tasks::setWindowLeft(QString val, QString& err)
{
	if (val.size() == 0){
		err = "";
		mWinLeft = 0;
		mUseWinLeft = false;
		return /*TaskState::*/Blank;
	}
	bool ok;
	int v = val.toInt(&ok);
	if (ok)
		return setWindowLeft(v, err);

	err = "The Window Left value must be a valid integer.";
	mUseWinLeft = false;
	return /*TaskState::*/Invalid;
}

TaskState Tasks::setWindowTop(int val, QString& err)
{
	err = "";
	mWinTop = val;
	mUseWinTop = true;
	return /*TaskState::*/Valid;
}
TaskState Tasks::setWindowTop(QString val, QString& err)
{
	if (val.size() == 0){
		err = "";
		mWinTop = 0;
		mUseWinTop = false;
		return /*TaskState::*/Blank;
	}
	bool ok;
	int v = val.toInt(&ok);
	if (ok)
		return setWindowTop(v, err);

	err = "The Window Top value must be a valid integer.";
	mUseWinTop = false;
	return /*TaskState::*/Invalid;
}

TaskState Tasks::setWindowWidth(int val, QString& err)
{
	err = "";
	mWinWidth = val;
	mUseWinWidth = true;
	return /*TaskState::*/Valid;
}
TaskState Tasks::setWindowWidth(QString val, QString& err)
{
	if (val.size() == 0){
		err = "";
		mWinWidth = 0;
		mUseWinWidth = false;
		return /*TaskState::*/Blank;
	}
	bool ok;
	int v = val.toInt(&ok);
	if (ok)
		return setWindowWidth(v, err);

	err = "The Window Left value must be a valid integer >= 0";
	mUseWinWidth = false;
	return /*TaskState::*/Invalid;
}

TaskState Tasks::setWindowHeight(int val, QString& err)
{
	err = "";
	mWinHeight = val;
	mUseWinHeight = true;
	return /*TaskState::*/Valid;
}
TaskState Tasks::setWindowHeight(QString val, QString& err)
{if (val.size() == 0){
		err = "";
		mWinHeight = 0;
		mUseWinHeight = false;
		return /*TaskState::*/Blank;
	}
	bool ok;
	int v = val.toInt(&ok);
	if (ok)
		return setWindowHeight(v, err);

	mWinHeight = 0;
	err = "The Window Left value must be a valid integer >= 0";
	mUseWinHeight = false;
	return /*TaskState::*/Invalid;
}
