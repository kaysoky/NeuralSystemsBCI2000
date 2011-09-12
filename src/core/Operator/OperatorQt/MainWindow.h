//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The Operator module's main window.
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
///////////////////////////////////////////////////////////////////////
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QLabel>
#include <QDateTime>
#include "VersionInfo.h"
#include "SysLog.h"
#include "ParamList.h"

#define TXT_WINDOW_CAPTION      "BCI2000/Operator"
#define TXT_OPERATOR_SUSPENDED  "Suspended"
#define TXT_OPERATOR_RUNNING    "Running"

namespace Ui
{
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

 private:
  Ui::MainWindow *ui;

 public:
  bool Terminating()
       { return mTerminating; }
  bool Terminated()
       { return mTerminated; }

 private slots:
  void on_actionStates_triggered();
  void on_actionAbout_triggered();
  void on_actionBCI2000_Help_triggered();
  void on_actionOperator_Log_toggled(bool );
  void on_actionConnection_Info_toggled(bool );
  void on_actionPreferences_triggered();
  void on_pushButton_Config_clicked();
  void on_pushButton_SetConfig_clicked();
  void on_pushButton_RunSystem_clicked();
  void on_actionExit_triggered();
  void on_pushButton_Quit_clicked();
  void on_pushButton_Btn1_clicked();
  void on_pushButton_Btn2_clicked();
  void on_pushButton_Btn3_clicked();
  void on_pushButton_Btn4_clicked();

 private:
  void QuitOperator();
  void Terminate();

  void ReadCommandLine();
  void UpdateDisplay();
  void SetFunctionButtons();
  void ExecuteScript( const QString& );

  void GetParameters();

 public slots:
  void PutParameters();

 private:
  virtual void timerEvent( QTimerEvent* );

  static void OnConnect( void* );
  static void OnSetConfig( void* );
  static void OnStart( void* );
  static void OnResume( void* );
  static void OnSuspend( void* );
  static void OnCoreInput( void* );
  static void OnShutdown( void* );
  static void OnDebugMessage( void*, const char* );
  static void OnLogMessage( void*, const char* );
  static void OnWarningMessage( void*, const char* );
  static void OnErrorMessage( void*, const char* );
  static void OnUnknownCommand( void*, const char* );
  static void OnParameter( void*, const char* );
  static void OnVisSignal( void*, const char*, int, int, float* );
  static void OnVisMemo( void*, const char*, const char* );
  static void OnVisBitmap( void*, const char*, int, int, unsigned short* );
  static void OnVisPropertyMessage( void*, const char*, int, const char* );
  static void OnVisProperty( void*, const char*, int, const char* );
  static void OnInitializeVis( void*, const char*, const char* );

 private:
  QLabel*        mpStatusLabels[4];
  SysLog         mSyslog;
  ParamList      mParameters;
  volatile bool  mTerminating,
                 mTerminated;
  QDateTime      mStarttime;
  QString        mTitle;
  VersionInfo    mVersionInfo;
  int            mUpdateTimerID;
};

#endif // MAINWINDOW_H
