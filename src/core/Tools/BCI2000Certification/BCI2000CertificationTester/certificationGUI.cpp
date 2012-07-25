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
#include "certificationGUI.h"
#include <iostream>
#include <vector>
#include <string>

#pragma warning( disable : 4482 )

certificationGUI::certificationGUI()
{
	mValidColor.setRgb(0,255,0);
	mInvalidColor.setRgb(255,0,0);
	mSemiValidColor.setRgb(255,255,0);
	initUI();
	mCurTask = -1;
	mEdited = false;
	mBCI2000Dir = "../../prog";
	initVals();
	mRunThread = NULL;
}

void certificationGUI::initUI()
{
	this->setMinimumWidth(400);
	this->setMinimumHeight(400);
	QTabWidget *widget = new QTabWidget;
	setCentralWidget(widget);
	QGridLayout *mainLayout2 = new QGridLayout;
	QWidget *controlWidget = new QWidget;
	QGridLayout *mainLayout = new QGridLayout;
	QWidget *configWidget = new QWidget;
	widget->addTab(configWidget, "Configuration");
	widget->addTab(controlWidget, "Control");
	configWidget->setLayout(mainLayout);
	controlWidget->setLayout(mainLayout2);

	//task list objects
	QGroupBox *taskGroup = new QGroupBox("Task List");
	QGridLayout *taskLayout = new QGridLayout;
	taskGroup->setLayout(taskLayout);
	mFileList = new QListWidget;
	mFileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mSelectAll = new QCheckBox("Select All");
	mAddTaskBtn = new QPushButton("+");
	mDelTaskBtn = new QPushButton("-");
	mCopyTaskBtn = new QPushButton("Copy");
	taskLayout->addWidget(mFileList,0,0,1,3);
	taskLayout->addWidget(mSelectAll,1,0,1,2);
	taskLayout->addWidget(mAddTaskBtn,2,0,1,1);
	taskLayout->addWidget(mDelTaskBtn,2,1,1,1);
	taskLayout->addWidget(mCopyTaskBtn,2,2,1,1);
	taskGroup->setMaximumWidth(230);
	//SIGNALS/SLOTS
	mainLayout->addWidget(taskGroup,0,0,1,1);

	// controls
	QGroupBox *controlGroup = new QGroupBox("Controls");
	QGridLayout *controlLayout = new QGridLayout;
	controlGroup->setLayout(controlLayout);
	mStartBtn = new QPushButton("Start");
	//mCancelBtn = new QPushButton("Cancel");
	mAnalyzeBtn = new QPushButton("Analyze");

	mConfigOnStartChk = new QCheckBox("Auto Set Config");
	mRunOnStartChk = new QCheckBox("Auto Start");
	mQuitOnSuspendChk = new QCheckBox("Auto Quit");
	mConfigOnStartChk->setTristate(false);
	mRunOnStartChk->setTristate(false);
	mQuitOnSuspendChk->setTristate(false);
	mConfigOnStartChk->setCheckState(Qt::Checked);
	mRunOnStartChk->setCheckState(Qt::Checked);
	mQuitOnSuspendChk->setCheckState(Qt::Checked);
	connect(mConfigOnStartChk, SIGNAL(stateChanged(int)), this, SLOT(updateConfigOnStart()));
	connect(mRunOnStartChk, SIGNAL(stateChanged(int)), this, SLOT(updateRunOnStart()));
	connect(mQuitOnSuspendChk, SIGNAL(stateChanged(int)), this, SLOT(updateQuitOnSuspend()));

	QGroupBox *progressGroup = new QGroupBox("Progress");
	QVBoxLayout *progressLayout = new QVBoxLayout;
	progressGroup->setLayout(progressLayout);
	mProgressBar = new QProgressBar;
	mProgressLabel = new QLabel("");
	mProgress = new progressClass(true, mProgressLabel, mProgressBar);
	progressLayout->addWidget(mProgressLabel);
	progressLayout->addWidget(mProgressBar);

	controlLayout->addWidget(mConfigOnStartChk,0,0,1,1);
	controlLayout->addWidget(mRunOnStartChk,0,1,1,1);
	controlLayout->addWidget(mQuitOnSuspendChk,0,2,1,1);
	controlLayout->addWidget(mStartBtn,1,1,1,1);
	//controlLayout->addWidget(mCancelBtn,0,1,1,1);
	controlLayout->addWidget(mAnalyzeBtn,1,2,1,1);
	controlLayout->addWidget(progressGroup,2,0,1,3);
	//SIGNALS
	mainLayout2->addWidget(controlGroup,1,0,1,1);

	mainLayout2->setColumnStretch(1,100);
	mainLayout2->setRowStretch(2,100);

	//globals
	QGroupBox *globalGroup = new QGroupBox("Global Settings");
	QGridLayout *globalLayout = new QGridLayout;
	globalGroup->setLayout(globalLayout);
	mWindowLeft = new labeledEdit("Window Left");
	mWindowTop = new labeledEdit("Window Top");
	mWindowWidth = new labeledEdit("Window Width");
	mWindowHeight = new labeledEdit("Window Height");
	mGlobalSignalBox = new getFileWidget("Global Signal Source", "Select the Global Signal Source", "exe file (*.exe)");
	mDataSaveBox = new getDirWidget("Data Output Directory","Select a Data Output Directory");
	mBCI2000DirBox = new getDirWidget("BCI2000 Directory","Select the BCI2000 prog Directory");

	/*mConfigOnStartChk;
	mRunOnStart;
	mQuitOnSuspend;
*/
	globalLayout->addWidget(mWindowLeft,0,0);
	globalLayout->addWidget(mWindowTop,0,1);
	globalLayout->addWidget(mWindowWidth,0,2);
	globalLayout->addWidget(mWindowHeight,0,3);
	globalLayout->addWidget(mGlobalSignalBox,1,0,1,4);
	globalLayout->addWidget(mDataSaveBox,2,0,1,4);
	globalLayout->addWidget(mBCI2000DirBox,3,0,1,4);

	connect(mWindowLeft->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateWindowLeft()));
	connect(mWindowTop->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateWindowTop()));
	connect(mWindowWidth->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateWindowWidth()));
	connect(mWindowHeight->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateWindowHeight()));

	connect(mBCI2000DirBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateBCI2000Dir()));
	connect(mGlobalSignalBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateGlobalSource()));
	//signals
	mainLayout2->addWidget(globalGroup,0,0,1,1);

	//parameters
	QGroupBox *detailsGroup = new QGroupBox("Task Details");
	QGridLayout *detailsLayout = new QGridLayout;
	detailsGroup->setLayout(detailsLayout);
	mTaskNameBox = new labeledEdit("Task Name");
	mSignalSourceBox = new labeledEdit("Signal Source");
	mSignalProcBox = new labeledEdit("Signal Processing");
	mAppBox = new labeledEdit("Application");
	mSampleRateBox = new labeledEdit("Sample Rate");
	mBlockSizeBox = new labeledEdit("Sample Block Size");
	mAmpBox = new labeledEdit("Amp Channel");
	mDAmpBox = new labeledEdit("Digital Amp Channel");
	mVidChBox = new labeledEdit("Video Channel");
	mVidStateBox = new labeledEdit("Video State");
	mVidValsBox = new labeledEdit("Video State Values");
	mAudChBox = new labeledEdit("Audio Channel");
	mAudStateBox = new labeledEdit("Audio State");
	mAudValsBox = new labeledEdit("Audio State Values");

	connect(mTaskNameBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateTaskNameBox()));
	connect(mSignalSourceBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateSignalSourceBox()));
	connect(mSignalProcBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateSignalProcBox()));
	connect(mAppBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateAppBox()));
	connect(mSampleRateBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateSampleRateBox()));
	connect(mBlockSizeBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateBlockSizeBox()));
	connect(mAmpBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateAmpBox()));
	connect(mDAmpBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateDAmpBox()));
	connect(mVidChBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateVidChBox()));
	connect(mVidStateBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateVidStateBox()));
	connect(mVidValsBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateVidValsBox()));
	connect(mAudChBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateAudChBox()));
	connect(mAudStateBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateAudStateBox()));
	connect(mAudValsBox->edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateAudValsBox()));

	QGroupBox *parmGroup = new QGroupBox("Parameters");
	QGridLayout *parmLayout = new QGridLayout;
	parmGroup->setLayout(parmLayout);
	mParmList = new QListWidget;
	mAddParmBtn = new QPushButton("+");
	mDelParmBtn = new QPushButton("-");
	parmLayout->addWidget(mParmList,0,0,3,3);
	parmLayout->addWidget(mAddParmBtn,3,1);
	parmLayout->addWidget(mDelParmBtn,3,2);
	parmGroup->setMaximumHeight(150);
	mParmList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(mAddParmBtn, SIGNAL(clicked()), this, SLOT(addParmsBtn()));
	connect(mDelParmBtn, SIGNAL(clicked()), this, SLOT(delParmsBtn()));

	detailsLayout->addWidget(mTaskNameBox,0,0,1,3);
	detailsLayout->addWidget(mSignalSourceBox,1,0);
	detailsLayout->addWidget(mSignalProcBox,2,0);
	detailsLayout->addWidget(mAppBox,3,0);
	detailsLayout->addWidget(parmGroup,1,1,3,2);

	QGroupBox *eventGroup = new QGroupBox("Events Configuration");
	QGridLayout *eventLayout = new QGridLayout;
	eventGroup->setLayout(eventLayout);
	eventLayout->addWidget(mSampleRateBox,0,0);
	eventLayout->addWidget(mBlockSizeBox,0,1);
	eventLayout->addWidget(mAmpBox,1,0);
	eventLayout->addWidget(mDAmpBox,1,1);
	eventLayout->addWidget(mVidChBox,2,0);
	eventLayout->addWidget(mVidStateBox,2,1);
	eventLayout->addWidget(mVidValsBox,2,2);
	eventLayout->addWidget(mAudChBox,3,0);
	eventLayout->addWidget(mAudStateBox,3,1);
	eventLayout->addWidget(mAudValsBox,3,2);
	detailsLayout->addWidget(eventGroup,4,0,1,3);

	mainLayout->addWidget(detailsGroup,0,1);

	//QMenuBar *menuBar = new QMenuBar;
	QMenu *fileMenu;
	fileMenu = menuBar()->addMenu("File");
	QAction *openAct = new QAction("Open", this);
	QAction *saveAct = new QAction("Save", this);
	QAction *saveAsAct = new QAction("Save As", this);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	connect(openAct, SIGNAL(triggered()), this, SLOT(openIniMnu()));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(saveIniMnu()));
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAsIniMnu()));

	eventLayout->setSpacing(0);
	detailsLayout->setSpacing(0);
	parmLayout->setSpacing(0);
	globalLayout->setSpacing(0);

	connect(mFileList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(taskListClicked()));
	connect(mStartBtn, SIGNAL(clicked()), this, SLOT(startBtn()));
}

void certificationGUI::initVals()
{
	mWindowLeft->setText("0");
	mWindowTop->setText("0");
	mWindowWidth->setText("800");
	mWindowHeight->setText("800");
	mGlobalSignalBox->setFile("");
	mDataSaveBox->setFile("data");
	mBCI2000DirBox->setFile(mBCI2000Dir);
	updateBCI2000Dir();
}

void certificationGUI::closeEvent(QCloseEvent *ev)
{
	if (mEdited){
		int ret = QMessageBox::question(this, "Save INI File","Changes have been made to the tasks. Would you like to save and exit, ignore and exit, or cancel?",
			QMessageBox::Save | QMessageBox::Ignore | QMessageBox::Cancel);

		if (ret == QMessageBox::Cancel){
			ev->ignore();
			return;
		}
		if(ret == QMessageBox::Save)
			saveIniMnu();
	}
	ev->accept();
}

void certificationGUI::addParmsBtn()
{
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;

	QStringList fname = QFileDialog::getOpenFileNames(this, "Select BCI2000 Parameter Files", 0, "prm files (*.prm)");

	if (fname.size() == 0)
		return;

	mEdited = true;
	for (int i = 0; i < fname.size(); i++)
		mCT[mCurTask].addParm(fname[i]);

	updateParmPanel();
}


void certificationGUI::openIniMnu()
{
	QString fname = QFileDialog::getOpenFileName(this, "Select INI File", 0, "ini files (*.ini)");

	if (fname.size() == 0)
		return;

	mIniFile = fname;
	initIni();

	//updateParmPanel();
}

void certificationGUI::saveIniMnu()
{
	if (mIniFile.size() == 0){
		saveAsIniMnu();
	}
	else{
		saveIni();
	}
}

void certificationGUI::saveAsIniMnu()
{
	QString fname = QFileDialog::getSaveFileName(this, "Save INI File", mIniFile, "ini files (*.ini)");

	if (fname.size() == 0)
		return;

	mIniFile = fname;
	saveIni();
}

void certificationGUI::saveIni()
{
	QList<int> badTasks;
	if (!mCT.tasks.checkTasks(badTasks)){
		QString str = "";
		for (int i = 0; i < badTasks.size(); i++){
			str = QString("%1%2: %3\n").arg(str).arg(i).arg(mCT[i].getTaskName());
		}
		QMessageBox msg;
		msg.setText("Some defined tasks contain errors. Do you want to continue? Show details to see tasks that will not be run.");
		msg.setDetailedText(str);
		msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msg.setDefaultButton(QMessageBox::No);
		int ret = msg.exec();
		if (ret == QMessageBox::No)
			return;
	}

	mCT.tasks.writeIni(mIniFile);
	mEdited = false;
}

bool certificationGUI::initIni()
{
	if (mIniFile != ""){
		if (!mCT.parseIni(mIniFile))
		{
			if (mCT.taskReturnCode() == -1)
			{
				QMessageBox::critical(this, "Missing File", QString("Unable to find " + mIniFile + ". Make sure this file is located in BCI2000/tools/BCI2000Certification before continuing."));
				return false;
			}
			else if (mCT.taskReturnCode() == -3)
			{
				QMessageBox::critical(this, "Duplicate Tasks", "Duplicate task names found in BCI2000Certification.ini. Remove or rename duplicates, and try again.");
				return false;
			}
			else{
				QString st = "";
				for (int i = 0; i < mCT.nTasks(); i++){
					if (mCT[i].skip)
						st = QString("%1%2: %3\n").arg(st).arg(i).arg(mCT[i].getTaskName());
				}

				QMessageBox msg;
				//msg.setText("Incomplete task definitions");
				msg.setText("Some defined tasks are incomplete. Do you want to continue? Show details to see tasks that will not be run.");
				msg.setDetailedText(st);
				msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
				msg.setDefaultButton(QMessageBox::No);
				int ret = msg.exec();
				if (ret == QMessageBox::No)
					return false;
			}
		}
	}
	//now check that the values are ok
	//QList<int> invalidTasks;
	//mCT.tasks.checkTasks(invalidTasks);

	//invalidTasks.push_back(10);
	mFileList->clear();
	for (int i = 0; i < mCT.nTasks(); i++){
		QListWidgetItem *tmpItem = new QListWidgetItem(mCT[i].getTaskName());
		tmpItem->setCheckState(mCT[i].skip ? Qt::Unchecked : Qt::Checked);
		mFileList->addItem(tmpItem);
		checkCurTask(i);
	}

	mGlobalSignalBox->setFile(mCT.tasks.getGlobalSource());
	updateBCI2000Dir();

	return true;
}

void certificationGUI::checkCurTask()
{
	checkCurTask(mCurTask);
}
void certificationGUI::checkCurTask(int i)
{
	if (i < 0 || i > mCT.nTasks())
		return;

	if (mCT[i].checkErrors()){
		mFileList->item(i)->setBackgroundColor(QColor(255,255,255));
	}
	else{
		mFileList->item(i)->setBackgroundColor(QColor(255,50,50));
	}
}

void certificationGUI::delParmsBtn()
{
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;

	int ret = QMessageBox::question(this, "Remove Parameters", "Are you sure you want to remove these parameter files?",
		QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

	if (ret == QMessageBox::Cancel)
		return;

	int p = 0;
	while (p < mParmList->count()){
		if (mParmList->item(p)->isSelected()){
			mCT[mCurTask].delParm(p);
			delete mParmList->takeItem(p);
		}
		else
			p++;
	}
}
void certificationGUI::updateTaskNameBox()
{
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;
	//check for spaces
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setTaskName(mTaskNameBox->edit->text(), err);
	setInfoState(mTaskNameBox->edit, st, err);
	if (st == /*TaskState::*/Valid)
		mFileList->item(mCurTask)->setText(mTaskNameBox->edit->text());

	checkCurTask();
}

void certificationGUI::updateSignalSourceBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setSource(mSignalSourceBox->edit->text(), err);
	setInfoState(mSignalSourceBox->edit, st, err);
	checkCurTask();
}

void certificationGUI::updateSignalProcBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setSigProc(mSignalProcBox->edit->text(), err);
	setInfoState(mSignalProcBox->edit, st, err);
	checkCurTask();
}

void certificationGUI::updateAppBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setApp(mAppBox->edit->text(), err);
	setInfoState(mAppBox->edit, st, err);
	checkCurTask();
}

void certificationGUI::updateSampleRateBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setSampleRate(mSampleRateBox->edit->text(), err);
	setInfoState(mSampleRateBox->edit, st, err);
	checkCurTask();
}

void certificationGUI::updateBlockSizeBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setBlockSize(mBlockSizeBox->edit->text(), err);
	setInfoState(mBlockSizeBox->edit, st, err);
	checkCurTask();
}

void certificationGUI::updateAmpBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setAmpCh(mAmpBox->edit->text(), err);
	setInfoState(mAmpBox->edit, st, err);
	checkCurTask();
}

void certificationGUI::updateDAmpBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setDAmpCh(mDAmpBox->edit->text(), err);
	setInfoState(mDAmpBox->edit, st, err);
	checkCurTask();
}

void certificationGUI::updateVidChBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setVidCh(mVidChBox->edit->text(), err);
	setInfoState(mVidChBox->edit, st, err);
	checkCurTask();
	if (st == /*TaskState::*/Blank){
			setInfoState(mVidStateBox->edit, /*TaskState::*/Blank, "");
			setInfoState(mVidValsBox->edit, /*TaskState::*/Blank, "");
			checkCurTask();
			return;
	}


	/*
	if(mVidStateBox->edit->text().size() == 0){
		setInfoState(mVidStateBox->edit, TaskState::SemiValid, "This video parameter is required");
	}
	if(mVidValsBox->edit->text().size() == 0){
		setInfoState(mVidValsBox->edit, TaskState::SemiValid, "This video parameter is required");
	}
	*/
}

void certificationGUI::updateVidStateBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setVidState(mVidStateBox->edit->text(), err);
	setInfoState(mVidStateBox->edit, st, err);
	checkCurTask();
	if (st == /*TaskState::*/Blank){
			setInfoState(mVidChBox->edit, /*TaskState::*/Blank, "");
			setInfoState(mVidValsBox->edit, /*TaskState::*/Blank, "");
			return;
	}

	/*
	if(mVidChBox->edit->text().size() == 0){
		setInfoState(mVidChBox->edit, TaskState::SemiValid, "This video parameter is required");
	}
	if(mVidValsBox->edit->text().size() == 0){
		setInfoState(mVidValsBox->edit, TaskState::SemiValid, "This video parameter is required");
	}
	*/
}

void certificationGUI::updateVidValsBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setVidVals(mVidValsBox->edit->text(), err);
	setInfoState(mVidValsBox->edit, st, err);
	checkCurTask();
	if (st == /*TaskState::*/Blank){
			setInfoState(mVidChBox->edit, /*TaskState::*/Blank, "");
			setInfoState(mVidStateBox->edit, /*TaskState::*/Blank, "");
			return;
	}
/*
	if(mVidChBox->edit->text().size() == 0){
		setInfoState(mVidChBox->edit, TaskState::SemiValid, "This video parameter is required");
	}
	if(mVidStateBox->edit->text().size() == 0){
		setInfoState(mVidStateBox->edit, TaskState::SemiValid, "This video parameter is required");
	}
	*/
}

void certificationGUI::updateAudChBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setAudCh(mAudChBox->edit->text(), err);
	setInfoState(mAudChBox->edit, st, err);
	checkCurTask();
	if (st == /*TaskState::*/Blank){
			setInfoState(mAudStateBox->edit, /*TaskState::*/Blank, "");
			setInfoState(mAudValsBox->edit, /*TaskState::*/Blank, "");
			return;
	}
/*
	if(mAudStateBox->edit->text().size() == 0){
		p = mAudStateBox->edit->palette();
		p.setColor(QPalette::Normal, QPalette::Base, TaskState::SemiValid);
		mAudStateBox->edit->setPalette(p);
	}
	if(mAudValsBox->edit->text().size() == 0){
		p = mAudValsBox->edit->palette();
		p.setColor(QPalette::Normal, QPalette::Base, TaskState::SemiValid);
		mAudValsBox->edit->setPalette(p);
	}
	*/
}

void certificationGUI::updateAudStateBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setAudState(mAudStateBox->edit->text(), err);
	setInfoState(mAudStateBox->edit, st, err);
	checkCurTask();
	if (st == /*TaskState::*/Blank){
			setInfoState(mAudChBox->edit, /*TaskState::*/Blank, "");
			setInfoState(mAudValsBox->edit, /*TaskState::*/Blank, "");
			return;
	}
/*
	if(mAudChBox->edit->text().size() == 0){
		p = mAudChBox->edit->palette();
		p.setColor(QPalette::Normal, QPalette::Base, TaskState::SemiValid);
		mAudChBox->edit->setPalette(p);
	}
	if(mAudValsBox->edit->text().size() == 0){
		p = mAudValsBox->edit->palette();
		p.setColor(QPalette::Normal, QPalette::Base, TaskState::SemiValid);
		mAudValsBox->edit->setPalette(p);
	}
	*/
}

void certificationGUI::updateAudValsBox()
{
	mEdited = true;
	QString err;
	TaskState st = mCT[mCurTask].setAudVals(mAudValsBox->edit->text(), err);
	setInfoState(mAudValsBox->edit, st, err);
	checkCurTask();
	if (st == /*TaskState::*/Blank){
			setInfoState(mAudChBox->edit, /*TaskState::*/Blank, "");
			setInfoState(mAudStateBox->edit, /*TaskState::*/Blank, "");
			return;
	}
/*
	if(mAudChBox->edit->text().size() == 0){
		p = mAudChBox->edit->palette();
		p.setColor(QPalette::Normal, QPalette::Base, TaskState::SemiValid);
		mAudChBox->edit->setPalette(p);
	}
	if(mAudStateBox->edit->text().size() == 0){
		p = mAudStateBox->edit->palette();
		p.setColor(QPalette::Normal, QPalette::Base, TaskState::SemiValid);
		mAudStateBox->edit->setPalette(p);
	}
	*/
}


void certificationGUI::taskListClicked()
{
	if (!(mCurTask < 0 || mCurTask > mCT.nTasks())){
		//check for errors in existing task
		if (!mCT[mCurTask].checkErrors()){
			int ret = QMessageBox::warning(this, "Existing errors in task",
				"The current task has errors. If you change tasks, "
				"these changes will be lost. Do you want to continue?",
				QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
			if (ret != QMessageBox::Yes){
				mFileList->setCurrentRow(mCurTask);
				return;
			}
		}
	}


	mCurTask = mFileList->currentRow();

	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;
	if(mFileList->currentItem()->checkState() == Qt::Unchecked)
		mCT[mCurTask].skip = true;
	else
		mCT[mCurTask].skip = false;
    updateParmPanel();
}

void certificationGUI::setOutputDir()
{
	QString fdir = QFileDialog::getExistingDirectory(this, "Select Output Directory");
	if (fdir.size() == 0)
		return;

	mOutputDir = fdir;
	mDataSaveBox->setFile(mOutputDir);
}

void certificationGUI::updateParmPanel()
{
	if (mCurTask < 0 || mCurTask > mCT.nTasks())
		return;

	QString strTmp;
	mTaskNameBox->setText(mCT[mCurTask].getTaskName());
	mParmList->clear();
	for (int i=0; i < mCT[mCurTask].getParmFiles().size(); i++){
		QListWidgetItem *tmp = new QListWidgetItem(mCT[mCurTask].getParmFilesDisp()[i]);
		tmp->setData(Qt::UserRole, mCT[mCurTask].getParmFiles()[i]);
		mParmList->addItem(tmp);
	}

	if (mCT[mCurTask].getAmpFlag())
		mAmpBox->setText(mCT[mCurTask].getAmpCh());
	if (mCT[mCurTask].getDAmpFlag())
		mDAmpBox->setText(mCT[mCurTask].getDAmpCh());

	if (mCT[mCurTask].getVidCh() != -1)
		mVidChBox->setText(mCT[mCurTask].getVidCh());
	else
		mVidChBox->setText("");

	mVidStateBox->setText( mCT[mCurTask].getVidState());

	for (size_t k = 0; k < mCT[mCurTask].getVidVals().size(); k++){
		strTmp = QString("%1 %2").arg(strTmp).arg(mCT[mCurTask].getVidVals()[k]);
	}
	strTmp = strTmp.trimmed();
	mVidValsBox->setText(strTmp);

	if (mCT[mCurTask].getAudCh() != -1)
		mAudChBox->setText(mCT[mCurTask].getAudCh());
	else
		mAudChBox->setText("");

	mAudStateBox->setText( mCT[mCurTask].getAudState());
	strTmp.clear();
	for (size_t k = 0; k < mCT[mCurTask].getAudVals().size(); k++){
		strTmp = QString("%1 %2").arg(strTmp).arg(mCT[mCurTask].getAudVals()[k]);
	}
	strTmp = strTmp.trimmed();
	mAudValsBox->setText(strTmp);

	mSampleRateBox->setText(mCT[mCurTask].getSampleRate());
	mBlockSizeBox->setText(mCT[mCurTask].getBlockSize());
	mSignalSourceBox->setText(mCT[mCurTask].getSource());
	mSignalProcBox->setText(mCT[mCurTask].getSigProc());
	mAppBox->setText(mCT[mCurTask].getApp());
}

void certificationGUI::startBtn()
{
	if (mEdited){
		int ret = QMessageBox::warning(this, "Edited tasks",
			"Some tasks have been edited, and the ini file has not been saved. Do you want to save the file and continue, continue without saving, or cancel?",
			QMessageBox::Save | QMessageBox::Ignore | QMessageBox::Cancel, QMessageBox::Cancel);

		switch (ret){
			case QMessageBox::Cancel:
				return;
			case QMessageBox::Save:
				saveIni();
		};
	}


	if (mRunThread == NULL){
		mProgress->init(mCT.nTasks());
		mRunThread = new RunThread(mProgress, &mCT);
		connect(mRunThread, SIGNAL(finished()), this, SLOT(threadFinished()));
	}
	else if (mRunThread->isRunning()){
		mRunThread->terminate();
		while (!mRunThread->isFinished())
			mRunThread->wait(250);

		delete mRunThread;
		mRunThread = NULL;
		mStartBtn->setText("Run");
		return;
	}
	else{
		delete mRunThread;
		mRunThread = NULL;
		mProgress->init(mCT.nTasks());
		mRunThread = new RunThread(mProgress, &mCT);
		connect(mRunThread, SIGNAL(finished()), this, SLOT(threadFinished()));
		mStartBtn->setText("Run");
	}


	mRunThread->start();
	mStartBtn->setText("Cancel");
}

void certificationGUI::RunThread::run()
{

	mCL->reset();
	mCL->nextTask();

	while(mCL->tasksRemain() && ! this->isFinished())
	{
		this->msleep(500);
		mProgress->increment((*mCL)[mCL->GetCurrentTask()].getTaskName());
		mCL->launchProgs();
		//mCL->monitorProgs();
		mCL->nextTask();
	}
}


void certificationGUI::threadFinished()
{
	mStartBtn->setText("Run");
}


void certificationGUI::updateBCI2000Dir()
{
	QString err;
	TaskState st = mCT.tasks.setProgPath(mBCI2000DirBox->edit->text(), err);
	setInfoState(mBCI2000DirBox->edit,st, err);
}

void certificationGUI::updateGlobalSource()
{
	QString err;
	TaskState st = mCT.tasks.setGlobalSource(mGlobalSignalBox->edit->text(), err);
	setInfoState(mGlobalSignalBox->edit,st, err);
}

void certificationGUI::updateWindowLeft()
{
	QString err;
	TaskState st = mCT.tasks.setWindowLeft(mWindowLeft->edit->text(), err);
	setInfoState(mWindowLeft->edit,st, err);
}
void certificationGUI::updateWindowTop()
{
	QString err;
	TaskState st = mCT.tasks.setWindowTop(mWindowTop->edit->text(), err);
	setInfoState(mWindowTop->edit,st, err);
}
void certificationGUI::updateWindowWidth()
{
	QString err;
	TaskState st = mCT.tasks.setWindowWidth(mWindowWidth->edit->text(), err);
	setInfoState(mWindowWidth->edit,st, err);
}
void certificationGUI::updateWindowHeight()
{
	QString err;
	TaskState st = mCT.tasks.setWindowHeight(mWindowHeight->edit->text(), err);
	setInfoState(mWindowHeight->edit,st, err);
}

void certificationGUI::updateConfigOnStart()
{
	mCT.tasks.setAutoCfg(mConfigOnStartChk->checkState() > 0);
}

void certificationGUI::updateRunOnStart()
{
	mCT.tasks.setAutoStart(mRunOnStartChk->checkState() > 0);
}

void certificationGUI::updateQuitOnSuspend()
{
	mCT.tasks.setAutoQuit(mQuitOnSuspendChk->checkState() > 0);
}


void certificationGUI::setInfoState(QWidget *w, TaskState c, QString s)
{
	QPalette p = w->palette();
	switch (c){
		case /*TaskState::*/Valid:
			p.setColor(QPalette::Normal, QPalette::Base, Qt::green);
			break;
		case /*TaskState::*/Invalid:
			p.setColor(QPalette::Normal, QPalette::Base, Qt::red);
			break;
		case /*TaskState::*/SemiValid:
			p.setColor(QPalette::Normal, QPalette::Base, Qt::yellow);
			break;
		case /*TaskState::*/Blank:
			p.setColor(QPalette::Normal, QPalette::Base, Qt::white);
			break;
		default:
			p.setColor(QPalette::Normal, QPalette::Base, Qt::white);
			break;
	}

	w->setPalette(p);
	w->setToolTip(s);
}

