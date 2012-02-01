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
#ifndef CERTIFICATIONCLASS_H
#define CERTIFICATIONCLASS_H
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <QtCore>
#include "TaskType.h"
#include "analysisClass.h"
#include "progressClass.h"

#ifdef WIN32
  #define DIRSEP "\\"
#else
  #define DIRSEP "/"
#endif


class certificationClass : public QThread
{
public:
  certificationClass();
  ~certificationClass();
  bool init(QStringList fileNames, QString iniFile, QString cfgFile, QString outputDir, progressClass *progress);
  void run();
  QString lastError(){return mError;}
  QString getOutputFilePath(){return mOutFilePath;}
private:
  QStringList mFileNames;
  progressClass *mProgress;
  QString mIniFile;
  QString mError;
  QString mCfgFile;
  QString mOutputDir;
  QString mOutFilePath;
  QString mDateTime;
  double mThresh;
  Tasks mTaskTypes;
  FILE *mResOut;
  std::vector<basicStats*> mMinReqs;
  std::vector< analysis*> mAnalyses;

  void genResultsFile();
  QString getCurDateTime();
  QString getShortFileName(QString);
  bool parseCfg();
};

#endif // CERTIFICATIONCLASS_H
