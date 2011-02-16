////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: Adam Wilson
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#ifndef analysisGUI_H
#define analysisGUI_H
#include <QtGui>
#include "certificationClass.h"
#include "progressClass.h"

class ResultsGUI : public QWidget{
public:
	ResultsGUI(QString fpath);
private:
	QTextEdit *mDisp;
};

class analysisGUI : public QWidget{
	Q_OBJECT

public:
	analysisGUI();
	void parseDir(QString dir, QStringList *files);
private:
	void initUI();

	QListWidget *mFileList;
	QPushButton *mAddFilesBtn;
	QPushButton *mDelFilesBtn;
	QPushButton *mAddIniBtn;
	QPushButton *mRunBtn;
	QLineEdit *mIniFileBox;
	QPushButton *mAddCfgBtn;
	QPushButton *mSetOutDirBtn;
	QLineEdit *mCfgFileBox;
	QLineEdit *mOutputDirBox;
	QTextEdit *mOutputText;
	int mRecursionDepth;

	QString mIniFile;
	QString mCfgFile;
	QString mOutputDir;
	//QDebugStream *out;
	progressClass *mProgress;
	QProgressBar *mProgressBar;
	QLabel *mProgressLabel;
	certificationClass *mCert;


public slots:
	void addIniBtn();
	void addCfgBtn();
	void setOutputDir();
	void addFilesBtn();
	void delFilesBtn();
	void runBtn();
	void threadFinished();
};
#endif // analysisGUI_H
