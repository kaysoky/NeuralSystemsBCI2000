//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The Operator module's main window.
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
#include "OSThread.h"
#include "BCIException.h"
#include "ParserToken.h"

#include <QMessageBox>
#include <QShortcut>
#include <sstream>
#include <fstream>

using namespace std;

const char* cProgramName = "Operator Module";

Preferences* gpPreferences = NULL;
extern ConnectionInfo* gpConnectionInfo;

MainWindow::MainWindow( QWidget* parent )
: QMainWindow( parent ), ui( new Ui::MainWindow ),
  mSyslog( this ),
  mHide( false ),
  mStartupIdle( false ),
  mTerminating( false ),
  mTerminated( false ),
  mUpdateTimerID( 0 )
{
  ui->setupUi(this);
  this->setWindowFlags(
          Qt::Window
          | Qt::CustomizeWindowHint
          | Qt::WindowTitleHint
          | Qt::WindowSystemMenuHint
  );
  for( size_t i = 0; i < Preferences::numButtons; ++i )
  {
    QString idx;
    idx.setNum( i + 1 );
    mButtons[i] = findChild<QPushButton*>( "pushButton_Btn" + idx );
    new QShortcut( QKeySequence( "F" + idx ), mButtons[i], SLOT(click()), SLOT(click()), Qt::ApplicationShortcut );
  }
  new QShortcut( QKeySequence( tr("Ctrl+W") ), this, SLOT(CloseTopmostWindow()), NULL, Qt::ApplicationShortcut );

  ReadCommandLine();

  if( gpPreferences == NULL )
    gpPreferences = new Preferences;

  if( gpConnectionInfo == NULL )
  {
    gpConnectionInfo = new ConnectionInfo( this );
    gpConnectionInfo->setVisible( false );
  }

  QFont font = mButtons[0]->font();
  int standardFontSize = QFontInfo( font ).pixelSize(),
      standardWeight = QFontInfo( font ).weight();
  font.setPixelSize( ( standardFontSize * 150 ) / 100 );
  font.setWeight( QFont::DemiBold );
  ui->pushButton_Config->setFont( font );
  ui->pushButton_SetConfig->setFont( font );
  ui->pushButton_RunSystem->setFont( font );
  ui->pushButton_Quit->setFont( font );
  font.setPixelSize( ( standardFontSize * 84 ) / 100 );
  font.setWeight( standardWeight );

  int nStatusLabels = sizeof( mpStatusLabels ) / sizeof( *mpStatusLabels );
  for( int i = 0; i < nStatusLabels; ++i )
  {
    QLabel* pLabel = new QLabel( ui->statusBar );
#ifndef _WIN32
    pLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    pLabel->setFont( font );
#endif
    pLabel->setText( "N/A" );
    ui->statusBar->addWidget( pLabel, 1 );
    mpStatusLabels[i] = pLabel;
  }

  OperatorUtils::RestoreWidget( this );
  BCI_Initialize();

  BCI_SetCallback( BCI_OnSetConfig, BCI_Function( SetStartTime ), this );
  BCI_SetCallback( BCI_OnStart, BCI_Function( SetStartTime ), this );
  BCI_SetCallback( BCI_OnResume, BCI_Function( SetStartTime ), this );
  BCI_SetCallback( BCI_OnSuspend, BCI_Function( SetStartTime ), this );

  BCI_SetExternalCallback( BCI_OnQuitRequest, BCI_Function( OnQuitRequest ), this );
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
  BCI_SetCallback( BCI_OnScriptHelp, BCI_Function( OnScriptHelp ), this );
  BCI_SetCallback( BCI_OnScriptError, BCI_Function( OnScriptError ), this );

  SetupScripts();

  if( mTelnet.length() )
    BCI_TelnetListen( mTelnet.toLocal8Bit().constData() );

  if( !mStartupIdle )
  {
    if( mStartup.length() )
      BCI_Startup( mStartup.toLocal8Bit().constData() );
    else
      BCI_Startup( "* SignalSource:4000 SignalProcessing:4001 Application:4002" );
  }

  if( mHide )
    this->hide();
  else
    this->show();

  mUpdateTimerID = this->startTimer( 100 );
  UpdateDisplay();
  SetFunctionButtons();
}

MainWindow::~MainWindow()
{
  OperatorUtils::SaveWidget( this );
  delete ui;
  ui = NULL;
  delete gpPreferences;
  gpPreferences = NULL;
  BCI_Dispose();
}

void
MainWindow::Terminate()
{
  bool doExecute = false;
  {
    OSMutex::Lock lock( mTerminationMutex );
    doExecute = !mTerminating;
    mTerminating = true;
  }
  if( doExecute )
  {
    BCI_Shutdown();
    // Execute the on-exit script ...
    if( !mExitScript.empty() )
    {
      mSyslog.AddEntry( "Executing OnExit script ..." );
      BCI_ExecuteScript( mExitScript.c_str() );
    }
    mSyslog.Close( true );
    mTerminated = true;
  }
}

void
MainWindow::QuitOperator()
{
  if( QMessageBox::Yes == QMessageBox::question(
       this,
       "Question", "Do you really want to quit BCI2000?",
       QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes )
    )
    Terminate();
}

void
MainWindow::ReadCommandLine()
{
  mHide = false;
  mTitle = "";
  mTelnet = "";
  mStartupIdle = false;
  mStartup = "";

  int i = 1;
  while( i < qApp->arguments().size() )
  {
    if( qApp->arguments().at( i ) == "--Title" )
    {
      if( ( i + 1 ) < qApp->arguments().size() )
        mTitle = qApp->arguments().at( ++i );
      else
        mTitle = "";
    }
    else if( qApp->arguments().at( i ) == "--Telnet" )
    {
      if( ( i + 1 ) < qApp->arguments().size() )
        mTelnet = qApp->arguments().at( ++i );
      else
        mTelnet = "localhost:3999";
    }
    else if( qApp->arguments().at( i ) == "--Hide" )
      mHide = true;
    else if( qApp->arguments().at( i ) == "--StartupIdle" )
      mStartupIdle = true;
    else if( qApp->arguments().at( i ) == "--Startup" )
    {
      if( ( i + 1 ) < qApp->arguments().size() )
        mStartup = qApp->arguments().at( ++i );
      mStartupIdle = false;
    }
    ++i;
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
  windowCaption += VersionInfo::Current[VersionInfo::VersionID].c_str();
  if( mTitle.length() > 0 )
  {
    windowCaption += " - ";
    windowCaption += mTitle;
  }

  int stateOfOperation = BCI_GetStateOfOperation();
  switch( stateOfOperation )
  {
    case BCI_StateIdle:
      statusText = "System Status: <Idle>";
      break;
    case BCI_StateStartup:
      statusText = "Waiting for connection";
      break;
    case BCI_StateInitialization:
      statusText = "Initialization Phase ...";
      break;
    case BCI_StateResting:
    case BCI_StateSuspended:
    case BCI_StateParamsModified:
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
      break;
    default:
      statusText = "System Status: <" + QString::number( stateOfOperation ) + ">";
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
  for( size_t i = 0; i < Preferences::numButtons; ++i )
  {
    QString name = gpPreferences->mButtons[i].Name,
            script = gpPreferences->mButtons[i].Cmd;
    SetFunctionButton( i, name, script );
  }
}

void
MainWindow::SetFunctionButton( size_t inIdx, const QString& inTitle, const QString& inScript )
{
  QPushButton* pButton = mButtons[inIdx];
  pButton->setEnabled( inTitle != "" && inScript != "" );
  if( inTitle != "" )
  {
    pButton->setText( inTitle );
  }
  else
  {
    QString num;
    num.setNum( inIdx + 1 );
    pButton->setText( "Function " + num );
  }
  mButtonScripts[inIdx] = inScript.toLocal8Bit().constData();
}

void
MainWindow::SetupScripts()
{
  if( !gpPreferences )
    throw bciexception( "Global preferences object does not exist" );

  static const struct { int id; const char* name; }
  events[] =
  {
    #define EVENT(x) { Preferences::x, #x },
    EVENT( OnConnect )
    EVENT( OnSetConfig )
    EVENT( OnResume )
    EVENT( OnSuspend )
    EVENT( OnStart )
    #undef EVENT
  };
  QString* scripts = gpPreferences->mScript;
  for( size_t i = 0; i < sizeof( events ) / sizeof( *events ); ++i )
  {
    string command = "SET SCRIPT " + string( events[i].name ) + " " + FormatScript( events[i].name, scripts[events[i].id] );
    BCI_ExecuteScript( command.c_str() );
  }
  mExitScript = FormatScript( "OnExit", scripts[Preferences::OnExit] );
}

string
MainWindow::FormatScript( const char* inEventName, const QString& inScript )
{
  string result = inScript.toLocal8Bit().constData();
  if( !result.empty() )
  {
    if( result[0] == '-' )
    { // Immediate script specified
      result = result.substr( 1 );
    }
    else
    { // Script file specified
      ifstream file( result.c_str() );
      if( !file.is_open() )
      {
        OnErrorMessage( this, ( "Could not open " + string( inEventName ) + " script file \"" + result + "\"" ).c_str() );
        result = "";
      }
      else
        result = "EXECUTE SCRIPT " + result;
    }
  }
  ostringstream oss;
  EncodedString( result ).WriteToStream( oss, "\";`" );
  result = oss.str();
  return result;
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

void
MainWindow::CloseTopmostWindow()
{
  QWidget* pActiveWindow = qApp->activeWindow();
  if( pActiveWindow == this )
    QuitOperator();
  else
    pActiveWindow->close();
}

////////////////////////////////////////////////////////////////////////////////
//----------    Callback functions called by the OperatorLib    --------------
////////////////////////////////////////////////////////////////////////////////
void
MainWindow::SetStartTime( void* inData )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  this_->mStarttime = QDateTime::currentDateTime();
}

void
MainWindow::OnQuitRequest( void* inData, const char** )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  this_->Terminate();
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
MainWindow::OnScriptError( void* inData, const char* s )
{
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  QString message = QString( "Script error: " ) + s;
  // If we receive an error message, add a line to the system log and bring it to front.
  QMetaObject::invokeMethod(
    &this_->mSyslog,
    "AddEntry",
    Qt::QueuedConnection,
    Q_ARG(QString, message),
    Q_ARG(int, SysLog::logEntryError));
}

void
MainWindow::OnParameter( void*, const char* s )
{
  Param param( s );
  // Update the parameter in the configuration window.
  if( gpConfig != NULL )
    gpConfig->RenderParameter( &param );
}

int
MainWindow::OnUnknownCommand( void* inData, const char* inCommand )
{
  int result = BCI_NotHandled;
  MainWindow* this_ = static_cast<MainWindow*>( inData );
  istringstream iss( inCommand );
  ParserToken verb, type, name;
  iss >> verb >> type >> name;
  enum { unknown, show, hide, set } action = unknown;
  if( !::stricmp( verb.c_str(), "Show" ) )
    action = show;
  else if( !::stricmp( verb.c_str(), "Hide" ) )
    action = hide;
  else if( !::stricmp( verb.c_str(), "Close" ) )
    action = hide;
  else if( !::stricmp( verb.c_str(), "Set" ) )
    action = set;
  if( action != unknown && !::stricmp( type.c_str(), "Window" ) )
  {
    result = BCI_Handled;
    enum { unknown, main, log, config } object = unknown;
    if( name.empty() || !::stricmp( name.c_str(), "Main" ) )
      object = main;
    else if( !::stricmp( name.c_str(), "Log" ) )
      object = log;
    else if( !::stricmp( name.c_str(), "Configuration" ) || !::stricmp( name.c_str(), "Config" ) )
      object = config;
    else
      result = BCI_NotHandled;

    switch( object )
    {
      case main:
        QMetaObject::invokeMethod(
          this_,
          "setVisible",
          Qt::QueuedConnection,
          Q_ARG(bool, action == show ));
        break;
      case log:
        QMetaObject::invokeMethod(
          &this_->mSyslog,
          "setVisible",
          Qt::QueuedConnection,
          Q_ARG(bool, action == show ));
        break;
      case config:
      {
        switch( action )
        {
          case show:
            QMetaObject::invokeMethod(
              this_,
              "on_pushButton_Config_clicked",
              Qt::QueuedConnection);
            break;
          case hide:
            QMetaObject::invokeMethod(
              gpConfig,
              "close",
              Qt::QueuedConnection);
            break;
            break;
          default:
            result = BCI_NotHandled;
         }
      }
    }
  }
  else if( action == set && !::stricmp( type.c_str(), "Title" ) )
  {
    result = BCI_Handled;
    this_->mTitle = QString::fromLocal8Bit( name.c_str() );
  }
  else if( action == set && !::stricmp( type.c_str(), "Button" ) )
  {
    int idx = ::atoi( name.c_str() ) - 1;
    if( idx >= 0 && idx < Preferences::numButtons )
    {
      result = BCI_Handled;
      ParserToken caption, script;
      iss >> caption >> script;
      QMetaObject::invokeMethod(
        this_,
        "SetFunctionButton",
        Qt::QueuedConnection,
        Q_ARG(int, idx),
        Q_ARG(QString, QString::fromLocal8Bit( caption.c_str() )),
        Q_ARG(QString, QString::fromLocal8Bit( script.c_str() ))
      );
      this_->mButtonScripts[idx] = script;
    }
  }
  return result;
}

void
MainWindow::OnScriptHelp( void*, const char** outHelp )
{
  *outHelp = "Show Window <name>, Hide Window <name>, Set Title <window title>, "
             "names are: Main, Log, Configuration";
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
MainWindow::on_actionQuit_triggered()
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
  BCI_ExecuteScript( mButtonScripts[0].c_str() );
}

void
MainWindow::on_pushButton_Btn2_clicked()
{
  BCI_ExecuteScript( mButtonScripts[1].c_str() );
}

void
MainWindow::on_pushButton_Btn3_clicked()
{
  BCI_ExecuteScript( mButtonScripts[2].c_str() );
}

void
MainWindow::on_pushButton_Btn4_clicked()
{
  BCI_ExecuteScript( mButtonScripts[3].c_str() );
}

