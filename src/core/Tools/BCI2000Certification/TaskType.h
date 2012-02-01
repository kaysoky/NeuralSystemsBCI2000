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
#ifndef TaskTypeH
#define TaskTypeH
#include <QtCore>
#include <QString>
#include <QStringList>
#include <sstream>
#include <fstream>
#include <vector>
#include "structs.h"
#include "Functions.h"

enum TaskMasks{
  TaskName = 0,
  SignalSource,
  SigProc,
  App,
  ParmFiles,
  Amp,
  DAmp,
  VidCh,
  VidState,
  VidVals,
  AudCh,
  AudState,
  AudVals,
  SampleRate,
  BlockSize
};

enum TaskState {
  Valid,
  Invalid,
  SemiValid,
  Blank
};

class TaskType
{
public:
    TaskType();
    ~TaskType();
friend class Tasks;
    bool skip;
    bool valid;
    bool exportData;
    void initTasks();
    void addParm(QString);
    void delParm(int);
    bool checkErrors();
private:
    QString mTaskName;
    QStringList mParms;
    QStringList mStates;
    QString mSignalSource, mSigProc, mApp;
    QStringList mParmFiles;
    QStringList mParmFilesDisp;
    analysisType mAmp;
    analysisType mDAmp;
    analysisType mVid;
    analysisType mAud;
    int mBlockSize;
    float mSampleRate;

    QList<bool> mErrors;

public:
  //member functions
  TaskState setTaskName(QString, QString&);
  QString getTaskName(){
    return mTaskName;
  };

  QStringList getParmFiles(){
    return mParmFiles;
  };
  QStringList getParmFilesDisp(){
    return mParmFilesDisp;
  };

  TaskState setTaskParms(QStringList, QString&);
  QStringList getTaskParms(){
    return mParms;
  };

  TaskState setTaskStates(QStringList, QString&);
  QStringList getTaskStates(){
    return mStates;
  };

  TaskState setSource(QString, QString&);
  QString getSource(){
    return mSignalSource;
  };

  TaskState setSigProc(QString, QString&);
  QString getSigProc(){
    return mSigProc;
  };

  TaskState setApp(QString, QString&);
  QString getApp(){
    return mApp;
  };

  TaskState setAmpCh(int, QString&);
  TaskState setAmpCh(QString, QString&);
  int getAmpCh(){
    return mAmp.ch;
  };
  bool getAmpFlag(){
    return mAmp.flag;
  };

  TaskState setDAmpCh(int, QString&);
  TaskState setDAmpCh(QString, QString&);
  int getDAmpCh(){
    return mDAmp.ch;
  };
  bool getDAmpFlag(){
    return mDAmp.flag;
  };

  TaskState setSampleRate(float, QString&);
  TaskState setSampleRate(QString, QString&);
  float getSampleRate(){
    return mSampleRate;
  };

  TaskState setBlockSize(int, QString&);
  TaskState setBlockSize(QString, QString&);
  int getBlockSize(){
    return mBlockSize;
  };

  TaskState setVidCh(int, QString&);
  TaskState setVidCh(QString, QString&);
  TaskState setVidState(QString, QString&);
  TaskState setVidVals(std::vector<int>, QString&);
  TaskState setVidVals(QString, QString&);
  int getVidCh(){
    return mVid.ch;
  };
  QString getVidState(){
    return mVid.state;
  };
  std::vector<int> getVidVals(){
    return mVid.stateVal;
  };
  bool getVidFlag(){
    return mVid.flag;
  };

  TaskState setAudCh(int, QString&);
  TaskState setAudCh(QString, QString&);
  TaskState setAudState(QString, QString&);
  TaskState setAudVals(std::vector<int>, QString&);
  TaskState setAudVals(QString, QString&);
  int getAudCh(){
    return mAud.ch;
  };
  QString getAudState(){
    return mAud.state;
  };
  std::vector<int> getAudVals(){
    return mAud.stateVal;
  };
  bool getAudFlag(){
    return mAud.flag;
  };

};

class Tasks : public std::vector<TaskType>
{
public:
    Tasks();
    Tasks(QString fname);
    ~Tasks();

    void init(QString fname);
    void parseIni();
    int getReturnCode(){return returnCode;}

    bool writeIni(QString);
    bool checkTasks(QList<int> &list);
    bool checkTask(int);

    QString getGlobalSource(){return mGlobalSource;}
    QString getProgPath(){return mProgPath;}
    QString getBCI2000Path(){return mBCI2000Path;}
    QString getOperator(){return mOperatorExe;}
    TaskState setGlobalSource(QString, QString&);
    bool mGlobalSourceValid;
    TaskState setProgPath(QString, QString&);
    bool mProgPathValid;
    TaskState setWindowLeft(int, QString&);
    TaskState setWindowTop(int, QString&);
    TaskState setWindowWidth(int, QString&);
    TaskState setWindowHeight(int, QString&);
    TaskState setWindowLeft(QString, QString&);
    TaskState setWindowTop(QString, QString&);
    TaskState setWindowWidth(QString, QString&);
    TaskState setWindowHeight(QString, QString&);
    int getWindowLeft(){return mWinLeft;}
    int getWindowTop(){return mWinTop;}
    int getWindowWidth(){return mWinWidth;}
    int getWindowHeight(){return mWinHeight;}
    bool useWindowLeft(){return mUseWinLeft;}
    bool useWindowTop(){return mUseWinTop;}
    bool useWindowWidth(){return mUseWinWidth;}
    bool useWindowHeight(){return mUseWinHeight;}

    void setAutoCfg(bool s){mAutoCfg = s;}
    void setAutoStart(bool s){mAutoStart = s;}
    void setAutoQuit(bool s){mAutoQuit = s;}
    bool getAutoCfg(){return mAutoCfg;}
    bool getAutoStart(){return mAutoStart;}
    bool getAutoQuit(){return mAutoQuit;}
private:
    int returnCode;
    QString mGlobalSource;
    QString mProgPath;
    QString mBCI2000Path;
    QString mOperatorExe;
    int mWinLeft, mWinTop, mWinWidth, mWinHeight;
    bool mUseWinLeft, mUseWinTop,mUseWinWidth, mUseWinHeight;
    bool mAutoCfg, mAutoStart, mAutoQuit;
};


//---------------------------------------------------------------------------
#endif // TaskTypeH
