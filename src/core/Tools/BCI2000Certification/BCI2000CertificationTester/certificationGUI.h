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
#ifndef certificationGUI_H
#define certificationGUI_H
#include <QtGui>
#include "certLauncher.h"
#include "progressClass.h"

class labeledEdit : public QWidget
{
public:
  labeledEdit(QString labelName)
  {
    QVBoxLayout *layout = new QVBoxLayout;
    edit = new QLineEdit;
    this->setLayout(layout);
    layout->addWidget(new QLabel(labelName));
    layout->addWidget(edit);
  }
  void setText(QString txt)
  {
    edit->setText(txt);
  }
  void setText(int txt)
  {
    edit->setText(QString("%1").arg(txt));
  }
  void setText(float txt)
  {
    edit->setText(QString("%1").arg(txt));
  }
  QLineEdit *edit;
};

class getFileWidget : public QWidget
{
  Q_OBJECT
public:
  getFileWidget(QString labelName, QString fileDlgTxt, QString fileFilter)
  {
    mFileDlgTxt = fileDlgTxt;
    mFileFilter = fileFilter;
    mFileName = "";
    QGridLayout *layout = new QGridLayout;
    this->setLayout(layout);
    mGetFileBtn = new QPushButton("...");
    edit = new QLineEdit;
    layout->addWidget(new QLabel(labelName),0,0,1,1);
    layout->addWidget(edit,1,0,1,3);
    layout->addWidget(mGetFileBtn,1,3,1,1);
    connect (mGetFileBtn, SIGNAL(clicked()), this, SLOT(getFile()));
  }

  QString getFileName(){return mFileName;}
  QLineEdit *edit;
public slots:
  void getFile()
  {
    QString fileName = QFileDialog::getOpenFileName(this, mFileDlgTxt, NULL, mFileFilter);
    if (fileName.size() == 0)
      return;

    mFileName = fileName;
    edit->setText(mFileName);
  }
  void setFile(QString file)
  {
    mFileName = file;
    edit->setText(mFileName);
  }
  QString getCurrentFile()
  {
    return mFileName;
  }

private:
  QPushButton *mGetFileBtn;

  QString mFileName;
  QString mFileDlgTxt;
  QString mFileFilter;
};

class getDirWidget : public QWidget
{
  Q_OBJECT
public:
  getDirWidget(QString labelName, QString fileDlgTxt)
  {
    mFileDlgTxt = fileDlgTxt;
    mFileName = "";
    QGridLayout *layout = new QGridLayout;
    this->setLayout(layout);
    mGetFileBtn = new QPushButton("...");
    edit = new QLineEdit;
    layout->addWidget(new QLabel(labelName),0,0,1,1);
    layout->addWidget(edit,1,0,1,3);
    layout->addWidget(mGetFileBtn,1,3,1,1);
    connect(mGetFileBtn, SIGNAL(clicked()), this, SLOT(getFile()));
  }

  QString getDirName(){return mFileName;}
  QLineEdit *edit;
public slots:
  void getFile()
  {
    QString fileName = QFileDialog::getExistingDirectory(this, mFileDlgTxt);
    if (fileName.size() == 0)
      return;

    mFileName = fileName;
    edit->setText(mFileName);
  }

  void setFile(QString file)
  {
    mFileName = file;
    edit->setText(mFileName);
  }

private:
  QPushButton *mGetFileBtn;

  QString mFileName;
  QString mFileDlgTxt;
  QString mFileFilter;
};

class certificationGUI : public QMainWindow{
  Q_OBJECT

public:
  certificationGUI();

private:
  void initUI();
  void initVals();
  bool initIni();
  void setInfoState(QWidget *, TaskState, QString);

  //task list
  QListWidget *mFileList;
  QCheckBox *mSelectAll;
  QPushButton *mAddTaskBtn;
  QPushButton *mDelTaskBtn;
  QPushButton *mCopyTaskBtn;

  //task details
  labeledEdit *mTaskNameBox;
  labeledEdit *mSignalSourceBox;
  labeledEdit *mSignalProcBox;
  labeledEdit *mAppBox;
  labeledEdit *mSampleRateBox;
  labeledEdit *mBlockSizeBox;
  labeledEdit *mAmpBox;
  labeledEdit *mDAmpBox;
  labeledEdit *mVidChBox;
  labeledEdit *mVidStateBox;
  labeledEdit *mVidValsBox;
  labeledEdit *mAudChBox;
  labeledEdit *mAudStateBox;
  labeledEdit *mAudValsBox;
  QListWidget *mParmList;
  QPushButton *mAddParmBtn;
  QPushButton *mDelParmBtn;

  //controls
  QPushButton *mStartBtn;
  QPushButton *mCancelBtn;
  QPushButton *mAnalyzeBtn;
  QProgressBar *mProgressBar;
  QLabel *mProgressLabel;
  QCheckBox *mConfigOnStartChk;
  QCheckBox *mRunOnStartChk;
  QCheckBox *mQuitOnSuspendChk;

  //global
  labeledEdit *mWindowLeft;
  labeledEdit *mWindowTop;
  labeledEdit *mWindowWidth;
  labeledEdit *mWindowHeight;
  getFileWidget *mGlobalSignalBox;
  getDirWidget *mDataSaveBox;
  getDirWidget *mBCI2000DirBox;

public slots:
  void addParmsBtn();
  void delParmsBtn();
  void openIniMnu();
  void saveIniMnu();
  void saveAsIniMnu();
  void saveIni();
  void setOutputDir();
  void updateParmPanel();
  void taskListClicked();
  void checkCurTask();
  void checkCurTask(int);
  void startBtn();
  void threadFinished();

  void updateTaskNameBox();
  void updateSignalSourceBox();
  void updateSignalProcBox();
  void updateAppBox();
  void updateSampleRateBox();
  void updateBlockSizeBox();
  void updateAmpBox();
  void updateDAmpBox();
  void updateVidChBox();
  void updateVidStateBox();
  void updateVidValsBox();
  void updateAudChBox();
  void updateAudStateBox();
  void updateAudValsBox();

  void updateBCI2000Dir();
  void updateGlobalSource();
  void updateWindowLeft();
  void updateWindowTop();
  void updateWindowWidth();
  void updateWindowHeight();
  void updateConfigOnStart();
  void updateRunOnStart();
  void updateQuitOnSuspend();
protected:
  void closeEvent(QCloseEvent *);

private: //data members
  CertLauncher mCT;
  QString mIniFile;
  QString mOutputDir;
  int mCurTask;
  bool mEdited;
  QString mBCI2000Dir;
  QColor mValidColor, mInvalidColor, mSemiValidColor;
  progressClass *mProgress;


  class RunThread : public QThread{
  public:
    RunThread(progressClass *progress, CertLauncher *CL){
      mProgress = progress;
      mCL = CL;
    };
    void run();
  private:
    progressClass *mProgress;
    CertLauncher *mCL;
  };

  RunThread *mRunThread;
};
#endif // certificationGUI_H
