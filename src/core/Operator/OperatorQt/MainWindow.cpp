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
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "../OperatorLib/BCI_OperatorLib.h"
#include "OperatorUtils.h"
#include "PrefDialog.h"
#include "ConnectionInfo.h"
#include "Preferences.h"
#include "ConfigWindow.h"
#include "ShowStates.h"
#include "ExecutableHelp.h"
#include "AboutBox.h"
#include "GenericSignal.h"
#include "VisDisplay.h"
#include "BitmapImage.h"
#include "Version.h"
#include "OSThread.h"

#include <QMessageBox>
#include <sstream>
#include <fstream>

using namespace std;

const char* cProgramName = "Operator Module";

Preferences* gpPreferences = NULL;
extern ConnectionInfo* gpConnectionInfo;

MainWindow::MainWindow( QWidget* parent )
: QMainWindow( parent ), ui( new Ui::MainWindow ),
  mSyslog( this ),
  mTerminating( false ),
  mTerminated( false ),
  mUpdateTimerID( 0 )
{
  istringstream iss( BCI2000_VERSION );
  iss >> mVersionInfo;

  ReadCommandLine();

  if( gpPreferences == NULL )
    gpPreferences = new Preferences;

  if( gpConnectionInfo == NULL )
  {
    gpConnectionInfo = new ConnectionInfo( this );
    gpConnectionInfo->setVisible( false );
  }

  ui->setupUi(this);
  this->setWindowFlags(
          Qt::Window
          | Qt::CustomizeWindowHint
          | Qt::WindowTitleHint
          | Qt::WindowSystemMenuHint
          | Qt::MSWindowsFixedSizeDialogHint
  );

  int nStatusLabels = sizeof( mpStatusLabels ) / sizeof( *mpStatusLabels );
  for( int i = 0; i < nStatusLabels; ++i )
  {
    mpStatusLabels[i] = new QLabel;
    mpStatusLabels[i]->setText( "N/A" );
    ui->statusBar->addWidget( mpStatusLabels[i], 1 );
  }
  ui->statusBar->addWidget( new QLabel, 0 );

  OperatorUtils::RestoreWidget( this );
  BCI_Initialize();

  BCI_SetExternalCallback( BCI_OnConnect, BCI_Function( OnConnect ), this );
  BCI_SetExternalCallback( BCI_OnSetConfig, BCI_Function( OnSetConfig ), this );
  BCI_SetExternalCallback( BCI_OnStart, BCI_Function( OnStart ), this );
  BCI_SetExternalCallback( BCI_OnResume, BCI_Function( OnResume ), this );
  BCI_SetExternalCallback( BCI_OnSuspend, BCI_Function( OnSuspend ), this );
  BCI_SetExternalCallback( BCI_OnShutdown, BCI_Function( OnShutdown ), this );
  BCI_SetExternalCallback( BCI_OnInitializeVis, BCI_Function( OnInitializeVis ), this );

  BCI_SetCallback( BCI_OnCoreInput, BCI_Function( OnCoreInput ), this );
  BCI_SetCallback( BCI_OnDebugMessage, BCI_Function( OnDebugMessage ), this );
  BCI_SetCallback( BCI_OnLogMessage, BCI_Function( OnLogMessage ), this );
  BCI_SetCallback( BCI_OnWarningMessage, BCI_Function( OnWarningMessage ), this );
  BCI_SetCallback( BCI_OnErrorMessage, BCI_Function( OnErrorMessage ), this );

  BCI_SetCallback( BCI_OnParameter, BCI_Function( OnParameter ), this );
  BCI_SetCallback( BCI_OnVisSignal, BCI_Function( OnVisSignal ), this );
  BCI_SetCallback( BCI_OnVisMemo, BCI_Function( OnVisMemo ), this );
  BCI_SetCallback( BCI_OnVisBitmap, BCI_Function( OnVisBitmap ), this );
  BCI_SetExternalCallback( BCI_OnVisPropertyMessage, BCI_Function( OnVisPropertyMessage ), this );
  BCI_SetExternalCallback( BCI_OnVisProperty, BCI_Function( OnVisProperty ), this );

  BCI_SetCallback( BCI_OnUnknownCommand, BCI_Function( OnUnknownCommand ), this );
  BCI_SetCallback( BCI_OnScriptError, BCI_Function( OnErrorMessage ), this );

  BCI_Startup( "SignalSource:4000 SignalProcessing:4001 Application:4002" );

  mUpdateTimerID = this->startTimer( 100 );
  UpdateDisplay();
  SetFunctionButtons();
}

MainWindow::~MainWindow()
{
  OperatorUtils::SaveWidget( this );
  delete ui;
  delete gpPreferences;
  BCI_Dispose();
}

void
MainWindow::Terminate()
{
  mTerminating = true;
  BCI_Shutdown();
  mTerminated = true;
}

void
MainWindow::QuitOperator()
{
  if( QMessageBox::Yes == QMessageBox::question(
       this,
       "Question", "Do you really want to quit BCI2000?",
       QMessageBox::No, QMessageBox::Yes | QMessageBox::Default )
    )
    this->Terminate();
}

void
MainWindow::ReadCommandLine()
{
  for( int i = 1; i < qApp->arguments().size(); ++i )
  {
    if( qApp->arguments().at( i ) == "--Title" && ( i + 1 ) < qApp->arguments().size() )
      mTitle = qApp->arguments().at( i + 1 );
  }
}

void
MainWindow::timerEvent( QTimerEvent* inEvent )
{
  if( inEvent->timerId() == mUpdateTimerID )
    UpdateDisplay();
}

void
MainWindow::UpdateDisplay()
{
  if( mTerminating )
    return;

  int t = mStarttime.secsTo( QDateTime::currentDateTime() );
  QTime timeElapsed( t / 3600, ( t / 60 ) % 60, t % 60 );
  QString windowCaption = TXT_WINDOW_CAPTION " ",
          statusText = "N/A";
  windowCaption += mVersionInfo[ "Version" ].c_str();
  if( mTitle.length() > 0 )
  {
    windowCaption += " - ";
    windowCaption += mTitle;
  }

  switch( BCI_GetStateOfOperation() )
  {
    case BCI_StateStartup:
      statusText = "System Status: <Idle>";
      break;
    case BCI_StateInitialization:
      statusText = "Initialization Phase ...";
      break;
    case BCI_StateResting:
    case BCI_StateSuspended:
      windowCaption += " - " TXT_OPERATOR_SUSPENDED " " + timeElapsed.toString( "mm:ss" ) + " s";
      statusText = TXT_OPERATOR_SUSPENDED;
      break;
    case BCI_StateRunning:
      windowCaption += " - " TXT_OPERATOR_RUNNING " " + timeElapsed.toString( "mm:ss" ) + " s";
      statusText = TXT_OPERATOR_RUNNING;
      break;
    case BCI_StateBusy:
      statusText = "Waiting...";
      break;
    case BCI_StateUnavailable:
      statusText = "Fatal Error ...";
  }

  int nStatusLabels = sizeof( mpStatusLabels ) / sizeof( *mpStatusLabels );
  if( this->windowTitle() != windowCaption )
    this->setWindowTitle( windowCaption );
  if( mpStatusLabels[ 0 ]->text() != statusText )
    mpStatusLabels[ 0 ]->setText( statusText );

  for( int i = 0; i < nStatusLabels - 1; ++i )
  {
    const char* p = BCI_GetCoreModuleStatus( i );
    if( mpStatusLabels[ i + 1 ]->text() != p )
      mpStatusLabels[ i + 1 ]->setText( p );
    BCI_ReleaseObject( p );
  }

  QString runSystemCaption = "Start";
  bool    configEnabled = false,
          setConfigEnabled = false,
          runSystemEnabled = false,
          quitEnabled = false;

  switch( BCI_GetStateOfOperation() )
  {
    case BCI_StateStartup:
      quitEnabled = true;
      break;
    case BCI_StateInitialization:
      configEnabled = true;
      setConfigEnabled = true;
      quitEnabled = true;
      break;
    case BCI_StateSuspended:
      runSystemCaption = "Resume";
      /* fall through */
    case BCI_StateResting:
      runSystemEnabled = true;
      configEnabled = true;
      setConfigEnabled = true;
      quitEnabled = true;
      break;
    case BCI_StateParamsModified:
      configEnabled = true;
      setConfigEnabled = true;
      quitEnabled = true;
      break;
    case BCI_StateBusy:
      runSystemCaption = "Resume";
      configEnabled = true;
      quitEnabled = true;
      break;
    case BCI_StateRunning:
      runSystemCaption = "Suspend";
      runSystemEnabled = true;
      break;
    case BCI_StateUnavailable:
      quitEnabled = true;
      break;
  }
  // To avoid unnecessary redraws of controls (flicker), we check for changed
  // captions before actually assigning them.
  if( ui->pushButton_RunSystem->text() != runSystemCaption )
      ui->pushButton_RunSystem->setText( runSystemCaption );
  if( ui->pushButton_Config->isEnabled() != configEnabled )
      ui->pushButton_Config->setEnabled( configEnabled );
  if( ui->pushButton_SetConfig->isEnabled() != setConfigEnabled )
      ui->pushButton_SetConfig->setEnabled( setConfigEnabled );
  if( ui->pushButton_RunSystem->isEnabled() != runSystemEnabled )
      ui->pushButton_RunSystem->setEnabled( runSystemEnabled );
  if( ui->pushButton_Quit->isEnabled() != quitEnabled )
      ui->pushButton_Quit->setEnabled( quitEnabled );

  ui->actionConnection_Info->setChecked( gpConnectionInfo && gpConnectionInfo->isVisible() );
  ui->actionOperator_Log->setChecked( mSyslog.isVisible() );
}

void
MainWindow::SetFunctionButtons()
{
  #define SET_BUTTON( number ) \
  if( gpPreferences->mButtons[ number - 1 ].Name != ""                                 \
      && gpPreferences->mButtons[ number - 1 ].Cmd != "" )                             \
  {                                                                                    \
    ui->pushButton_Btn##number->setEnabled( true );                                    \
    ui->pushButton_Btn##number->setText( gpPreferences->mButtons[ number - 1 ].Name ); \
  }                                                                                    \
  else                                                                                 \
  {                                                                                    \
    ui->pushButton_Btn##number->setEnabled( false );                                   \
    ui->pushButton_Btn##number->setText( "Function " #number );                        \
  }
  SET_BUTTON( 1 );
  SET_BUTTON( 2 );
  SET_BUTTON( 3 );
  SET_BUTTON( 4 );
}

void
MainWindow::ExecuteScript( const QString& inScript )
{
  string s = inScript.toLocal8Bit().constData();
  if( !s.empty() )
  {
    if( s[ 0 ] == '-' )
    {
      s = s.substr( 1 );
      BCI_ExecuteScript( s.c_str() );
    }
    else
    {
      ifstream file( s.c_str() );
      if( !file.is_open() )
      {
        string err = "Could not open script file ";
        err += s;
        OnErrorMessage( this, err.c_str() );
      }
      else
      {
        getline( file, s, '\0' );
        BCI_ExecuteScript( s.c_str() );
      }
    }
  }
}

void
MainWindow::GetParameters()
{
  mParameters.Clear();
  int i = 0;
  const char* p = NULL;
  do
  {
    p = BCI_GetParameter( i++ );
    if( p != NULL )
    {
      Param param( p );
      mParameters[param.Name()] = param;
    }
    BCI_ReleaseObject( p );
  } while( p != NULL );
}

void
MainWindow::PutParameters()
{
  for( int i = 0; i < mParameters.Size(); ++i )
  {
    ostringstream oss;
    oss << mParameters[i];
    BCI_PutParameter( oss.str().c_str() );
  }
}


////////////////////////////////////////////////////////////////////////////////
//----------    Callback functions called by the OperatorLib    --------------
////////////////////////////////////////////////////////////////////////////////
void
MainWindow::OnConnect( void* inData )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  // Execute the script after all modules are connected ...
  if( gpPreferences && gpPreferences->mScript[ Preferences::AfterModulesConnected ] != "" )
  {
    this_->mSyslog.AddEntry( "Executing script after all modules connected ..." );
    this_->ExecuteScript( gpPreferences->mScript[ Preferences::AfterModulesConnected ] );
  }
}

void
MainWindow::OnSetConfig( void* inData )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  if( gpPreferences && gpPreferences->mScript[ Preferences::OnSetConfig ] != "" )
  {
    this_->mSyslog.AddEntry( "Executing OnSetConfig script ..." );
    this_->ExecuteScript( gpPreferences->mScript[ Preferences::OnSetConfig ] );
  }
  this_->mStarttime = QDateTime::currentDateTime();
}

void
MainWindow::OnStart( void* inData )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  // Execute the on-start script ...
  if( gpPreferences && gpPreferences->mScript[ Preferences::OnStart ] != "" )
  {
    this_->mSyslog.AddEntry( "Executing OnStart script ..." );
    this_->ExecuteScript( gpPreferences->mScript[ Preferences::OnStart ] );
  }
  this_->mStarttime = QDateTime::currentDateTime();
}

void
MainWindow::OnResume( void* inData )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  // Execute the on-resume script ...
  if( gpPreferences && gpPreferences->mScript[ Preferences::OnResume ] != "" )
  {
    this_->mSyslog.AddEntry( "Executing OnResume script ..." );
    this_->ExecuteScript( gpPreferences->mScript[ Preferences::OnResume ] );
  }
  this_->mStarttime = QDateTime::currentDateTime();
}

void
MainWindow::OnSuspend( void* inData )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  // Execute the on-suspend script ...
  if( gpPreferences && gpPreferences->mScript[ Preferences::OnSuspend ] != "" )
  {
    this_->mSyslog.AddEntry( "Executing OnSuspend script ..." );
    this_->ExecuteScript( gpPreferences->mScript[ Preferences::OnSuspend ] );
  }
  this_->mStarttime = QDateTime::currentDateTime();
}

void
MainWindow::OnShutdown( void* inData )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  // Execute the on-exit script ...
  if( gpPreferences && gpPreferences->mScript[ Preferences::OnExit ] != "" )
  {
    this_->mSyslog.AddEntry( "Executing OnExit script ..." );
    this_->ExecuteScript( gpPreferences->mScript[ Preferences::OnExit ] );
  }
  if( !this_->Terminating() )
  {
    this_->mSyslog.Close( true );
    this_->Terminate();
  }
}

void
MainWindow::OnCoreInput( void* inData )
{
  if( gpConnectionInfo != NULL )
    QMetaObject::invokeMethod(
      gpConnectionInfo,
      "UpdateDisplay",
      Qt::QueuedConnection );
}

void
MainWindow::OnDebugMessage( void* inData, const char* s )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  QMetaObject::invokeMethod(
    &this_->mSyslog,
    "AddEntry",
    Qt::QueuedConnection,
    Q_ARG(QString, s),
    Q_ARG(int, SysLog::logEntryNormal));
}

void
MainWindow::OnLogMessage( void* inData, const char* s )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  QMetaObject::invokeMethod(
    &this_->mSyslog,
    "AddEntry",
    Qt::QueuedConnection,
    Q_ARG(QString, s),
    Q_ARG(int, SysLog::logEntryNormal));
}

void
MainWindow::OnWarningMessage( void* inData, const char* s )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  // If we receive a warning message, add a line to the system log and bring it to front.
  QMetaObject::invokeMethod(
    &this_->mSyslog,
    "AddEntry",
    Qt::QueuedConnection,
    Q_ARG(QString, s),
    Q_ARG(int, SysLog::logEntryWarning));
}

void
MainWindow::OnErrorMessage( void* inData, const char* s )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  // If we receive an error message, add a line to the system log and bring it to front.
  QMetaObject::invokeMethod(
    &this_->mSyslog,
    "AddEntry",
    Qt::QueuedConnection,
    Q_ARG(QString, s),
    Q_ARG(int, SysLog::logEntryError));
}

void
MainWindow::OnUnknownCommand( void* inData, const char* inCommand )
{
  string s = "Unknown command: \"";
  s += inCommand;
  s += "\"";
  OnErrorMessage( inData, s.c_str() );
}

void
MainWindow::OnParameter( void*, const char* s )
{
  Param param( s );
  // Update the parameter in the configuration window.
  if( gpConfig != NULL )
    gpConfig->RenderParameter( &param );
}

void
MainWindow::OnVisSignal( void*, const char* visID, int ch, int el, float* data )
{
  GenericSignal signal( ch, el );
  for( int i = 0; i < ch; ++i )
    for( int j = 0; j < el; ++j )
      signal( i, j ) = data[ i*el + j ];
  VisDisplay::HandleSignal( visID, signal );
}

void
MainWindow::OnVisMemo( void*, const char* visID, const char* s )
{
  VisDisplay::HandleMemo( visID, s );
}

void
MainWindow::OnVisBitmap( void*, const char* visID, int width, int height, unsigned short* data )
{
  BitmapImage bitmap( width, height, data );
  VisDisplay::HandleBitmap( visID, bitmap );
}

void
MainWindow::OnVisPropertyMessage( void*, const char* visID, int cfgID, const char* value )
{
  VisDisplay::HandlePropertyMessage( visID, cfgID, value );
}

void
MainWindow::OnVisProperty( void*, const char* visID, int cfgID, const char* value )
{
  VisDisplay::HandleProperty( visID, cfgID, value );
}

void
MainWindow::OnInitializeVis( void*, const char* inVisID, const char* inKind )
{
  string kind = inKind;
  if( kind == "Memo" )
    VisDisplay::CreateMemo( inVisID );
  else if( kind == "Graph" )
    VisDisplay::CreateGraph( inVisID );
  else if( kind == "Bitmap" )
    VisDisplay::CreateBitmap( inVisID );
}

////////////////////////////////////////////////////////////////////////////////
//----------------------   IDE-managed Qt slots    ---------------------------
////////////////////////////////////////////////////////////////////////////////

void
MainWindow::on_pushButton_Quit_clicked()
{
  QuitOperator();
}

void
MainWindow::on_actionExit_triggered()
{
  QuitOperator();
}

void
MainWindow::on_pushButton_RunSystem_clicked()
{
  if( BCI_GetStateOfOperation() == BCI_StateRunning )
    BCI_StopRun();
  else
  {
    if( gpConfig && gpConfig->isVisible() )
      gpConfig->close();
    BCI_StartRun();
  }
}

void
MainWindow::on_pushButton_SetConfig_clicked()
{
  ui->pushButton_SetConfig->setEnabled( false );
  if( gpConfig && gpConfig->isVisible() )
    gpConfig->close();
  BCI_SetConfig();
}

void
MainWindow::on_pushButton_Config_clicked()
{
  if( gpConfig == NULL )
  {
    gpConfig = new ConfigWindow( this );
    connect( gpConfig, SIGNAL(finished(int)), this, SLOT(PutParameters()) );
  }
  GetParameters();
  gpConfig->Initialize( &mParameters, gpPreferences );
  gpConfig->show();
}

void
MainWindow::on_actionPreferences_triggered()
{
  PrefDialog dialog( this );
  dialog.exec();
  SetFunctionButtons();
}

void
MainWindow::on_actionConnection_Info_toggled( bool inOnOff )
{
  gpConnectionInfo->setVisible( inOnOff );
}

void
MainWindow::on_actionOperator_Log_toggled( bool inOnOff )
{
  if( inOnOff )
    mSyslog.show();
  else
    mSyslog.hide();
}

void
MainWindow::on_actionBCI2000_Help_triggered()
{
  ExecutableHelp()
  .Display();
}

void
MainWindow::on_actionAbout_triggered()
{
  AboutBox()
  .SetApplicationName( cProgramName )
  .Display();
}

void
MainWindow::on_actionStates_triggered()
{
  ShowStates s( this );
  s.exec();
}

void
MainWindow::on_pushButton_Btn1_clicked()
{
  BCI_ExecuteScript( gpPreferences->mButtons[0].Cmd.toLocal8Bit().constData() );
}

void
MainWindow::on_pushButton_Btn2_clicked()
{
  BCI_ExecuteScript( gpPreferences->mButtons[1].Cmd.toLocal8Bit().constData() );
}

void
MainWindow::on_pushButton_Btn3_clicked()
{
  BCI_ExecuteScript( gpPreferences->mButtons[2].Cmd.toLocal8Bit().constData() );
}

void
MainWindow::on_pushButton_Btn4_clicked()
{
  BCI_ExecuteScript( gpPreferences->mButtons[3].Cmd.toLocal8Bit().constData() );
}
