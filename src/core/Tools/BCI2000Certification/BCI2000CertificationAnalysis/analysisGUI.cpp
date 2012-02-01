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
#include "analysisGUI.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

analysisGUI::analysisGUI()
{
	initUI();
	//out = new QDebugStream(std::cout, mOutputText);
	mCert = NULL;
}

void analysisGUI::initUI()
{
	this->setMinimumWidth(500);
	QGridLayout *mainLayout = new QGridLayout;
	this->setLayout(mainLayout);

	mFileList = new QListWidget;
	mFileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mainLayout->addWidget(new QLabel("Data Files"),0,0);
	mainLayout->addWidget(mFileList,1,0,1,4);

	mAddFilesBtn = new QPushButton("+");
	mDelFilesBtn = new QPushButton("-");
	mainLayout->addWidget(mAddFilesBtn,2,2,1,1);
	mainLayout->addWidget(mDelFilesBtn,2,3,1,1);

	mIniFileBox = new QLineEdit;
	mAddIniBtn = new QPushButton("...");
	QWidget *iniWidget = new QWidget;
	QGridLayout *iniLayout = new QGridLayout;
	iniWidget->setLayout(iniLayout);
	iniLayout->setContentsMargins(0,0,0,0);
	iniLayout->addWidget(new QLabel("*.ini File"), 0,0,1,1);
	iniLayout->addWidget(mIniFileBox,1,0,1,5);
	iniLayout->addWidget(mAddIniBtn,1,5,1,1);
	mainLayout->addWidget(iniWidget, 4,0,1,4);

	mCfgFileBox = new QLineEdit;
	mCfgFileBox->setText("BCI2000Certification.cfg");
	mAddCfgBtn = new QPushButton("...");
	QWidget *cfgWidget = new QWidget;
	QGridLayout *cfgLayout = new QGridLayout;
	cfgWidget->setLayout(cfgLayout);
	cfgLayout->setContentsMargins(0,0,0,0);
	cfgLayout->addWidget(new QLabel("*.cfg File"), 0,0,1,1);
	cfgLayout->addWidget(mCfgFileBox,1,0,1,5);
	cfgLayout->addWidget(mAddCfgBtn,1,5,1,1);
	mainLayout->addWidget(cfgWidget, 5,0,1,4);

	mOutputDirBox = new QLineEdit;
	mOutputDirBox->setText("./");
	mSetOutDirBtn = new QPushButton("...");
	QWidget *outputWidget = new QWidget;
	QGridLayout *outputLayout = new QGridLayout;
	outputWidget->setLayout(outputLayout);
	outputLayout->setContentsMargins(0,0,0,0);
	outputLayout->addWidget(new QLabel("Output Directory"), 0,0,1,1);
	outputLayout->addWidget(mOutputDirBox,1,0,1,5);
	outputLayout->addWidget(mSetOutDirBtn,1,5,1,1);
	mainLayout->addWidget(outputWidget, 6,0,1,4);

	mRunBtn = new QPushButton("Run");
	mainLayout->addWidget(mRunBtn,7,3,1,1);

	mProgressLabel = new QLabel("");
	mProgressBar = new QProgressBar();
	mProgress = new progressClass(true, mProgressLabel, mProgressBar);
	//mOutputText = new QTextEdit;
	//mOutputText->setReadOnly(true);
	mainLayout->addWidget(mProgressLabel,8,0,1,4);
	mainLayout->addWidget(mProgressBar,9,0,1,4);

	connect(mAddIniBtn, SIGNAL(clicked()), this, SLOT(addIniMnu()));
	connect(mAddCfgBtn, SIGNAL(clicked()), this, SLOT(addCfgBtn()));
	connect(mSetOutDirBtn, SIGNAL(clicked()), this, SLOT(setOutputDir()));
	connect(mAddFilesBtn, SIGNAL(clicked()), this, SLOT(addFilesBtn()));
	connect(mDelFilesBtn, SIGNAL(clicked()), this, SLOT(delFilesBtn()));
	connect(mRunBtn, SIGNAL(clicked()), this, SLOT(runBtn()));
}

void analysisGUI::addIniBtn()
{
	QString fname = QFileDialog::getOpenFileName(this, "Select INI File", 0, "ini files (*.ini)");

	if (fname.size() == 0)
		return;

	mIniFile = fname;
	mIniFileBox->setText(mIniFile);
}


void analysisGUI::addCfgBtn()
{
	QString fname = QFileDialog::getOpenFileName(this, "Select CFG File", 0, "cfg files (*.cfg)");

	if (fname.size() == 0)
		return;

	mCfgFile = fname;
	mCfgFileBox->setText(mCfgFile);
}

void analysisGUI::addFilesBtn()
{
	QString fdir = QFileDialog::getExistingDirectory(this, "Select Data Directory");
	if (fdir.size() == 0)
		return;

	QStringList flist;
	mRecursionDepth = 0;
	parseDir(fdir, &flist);
	mFileList->addItems(flist);
}

void analysisGUI::setOutputDir()
{
	QString fdir = QFileDialog::getExistingDirectory(this, "Select Output Directory");
	if (fdir.size() == 0)
		return;

	mOutputDir = fdir;
	mOutputDirBox->setText(mOutputDir);
}

void analysisGUI::delFilesBtn()
{
	int p = 0;
	while (p < mFileList->count()){
		if (mFileList->item(p)->isSelected())
			delete mFileList->takeItem(p);
		else
			p++;
	}
}

void analysisGUI::runBtn()
{
	if (mCert == NULL){
		mCert = new certificationClass;
		connect(mCert, SIGNAL(finished()), this, SLOT(threadFinished()));
	}
	else if (mCert->isRunning()){
		mCert->terminate();
		while (!mCert->isFinished())
			mCert->wait(250);

		delete mCert;
		mCert = NULL;
		mRunBtn->setText("Run");
		return;
	}
	else{
		delete mCert;
		mCert = NULL;
		mCert = new certificationClass;
		connect(mCert, SIGNAL(finished()), this, SLOT(threadFinished()));
		mRunBtn->setText("Run");
	}

	QStringList fnames;
	for (int i = 0; i < mFileList->count(); i++){
		fnames.push_back(mFileList->item(i)->text());
	}
	mProgress->init(fnames.size()*2);
	if (!mCert->init(fnames, mIniFileBox->text(), mCfgFileBox->text(), mOutputDirBox->text(), mProgress)){
		QMessageBox::critical(this, "Initialization Error",mCert->lastError());
		return;
	}
	mCert->start();
	mRunBtn->setText("Cancel");

}

void analysisGUI::threadFinished()
{
	cout << "Finished analysis!" << endl;
	QString tmppath(mCert->getOutputFilePath());
	ResultsGUI *results = new ResultsGUI(tmppath);
	results->show();
	mRunBtn->setText("Run");
}

void analysisGUI::parseDir(QString dir, QStringList *files)
{
	mRecursionDepth++;
	QDir direct(dir);

	//first, get a list of all *.dat files in this directory, and add them to the list
	QStringList filters;
	filters << "*.dat";
	QFileInfoList tmpList = direct.entryInfoList(filters);
	for (int i = 0; i < tmpList.size(); i++)
		files->append(tmpList[i].absoluteFilePath());

	//now get a list of all directories under the current directory
	filters.clear();
	QFileInfoList subdirs = direct.entryInfoList(filters, QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < subdirs.size(); i++){
		parseDir(subdirs[i].absoluteFilePath(), files);
	}
	mRecursionDepth--;
	if (mRecursionDepth == 0)
	{
		//get the ini file if it exists
		direct.setPath(dir);
		filters.clear();
		filters << "*.ini";
		tmpList = direct.entryInfoList(filters);
		//only use one for now
		if (tmpList.size() > 0){
			mIniFile = tmpList[0].absoluteFilePath();
			mIniFileBox->setText(mIniFile);
		}
	}
}

ResultsGUI::ResultsGUI(QString fpath) :
QWidget(0, Qt::Window)
{
	mDisp = new QTextEdit;
	QHBoxLayout *layout = new QHBoxLayout;
	this->setLayout(layout);
	this->setWindowTitle("Certification Results");
	this->setMinimumWidth(1000);
	layout->addWidget(mDisp);
	mDisp->setCurrentFont(QFont("courier"));
	QFile mFile(fpath);
	if (!mFile.open(QIODevice::ReadOnly | QIODevice::Text)){
		int err = mFile.error();
		mDisp->append("Error reading results file (" + QString(err) + ")");
	}
	else{
		QTextStream in(&mFile);
		mDisp->append(in.readAll());
	}
}
