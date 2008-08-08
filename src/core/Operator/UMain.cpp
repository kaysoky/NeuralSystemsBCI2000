/******************************************************************************
 * Program:   OPERAT.EXE                                                      *
 * Module:    UMAIN.CPP                                                       *
 * Comment:   The main module of the operator program in BCI2000              *
 * Version:   0.25                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 02/10/2000 - First start                                           *
 * V0.02 - 02/16/2000 - Keepin on workin                                      *
 * V0.03 - 02/21/2000 - Assembling lists of parameters and states             *
 * V0.04 - 02/23/2000 - Sending all parameters and states back                *
 * V0.05 - 02/24/2000 - Constructing and publishing of the initial state vec. *
 * V0.06 - 03/02/2000 - Added the supplemental byte after the content descr.  *
 *         03/04/2000 - Removed a couple bugs related to the state vector     *
 * V0.07 - 03/04/2000 - Created signal processing and application modules     *
 * V0.08 - 03/09/2000 - Receiving status messages from core                   *
 * V0.09 - 03/16/2000 - Complete communication in between all core modules    *
 * V0.10 - 03/20/2000 - Updated to C++ Builder 5.0                            *
 * V0.11 - 03/24/2000 - made the socket connections blocking inst. of non-bl. *
 * V0.12 - 03/29/2000 - stable and commented version                          *
 * V0.13 - 04/19/2000 - test version                                          *
 * V0.14 - 05/16/2000 - added ability to set state 'Running'                  *
 * V0.15 - 06/09/2000 - implementing parameter visualization (for config)     *
 *         06/13/2000 - implemented data visualization                        *
 * V0.16 - 07/25/2000 - removed the main chart                                *
 * V0.17 - 08/08/2000 - being able to display visualization of float          *
 * V0.18 - 09/21/2000 - removed bug in visualization                          *
 *                      added parameter saving of visualization parameters    *
 * V0.19 - 12/15/2000 - added editable matrices                               *
 * V0.20 - 04/16/2001 - minor changes to work with coremodules V0.20          *
 *         04/30/2001 - allows for changes to parameters while cfg wind. open *
 * V0.21 - 05/11/2001 - timer at operator; introduced user levels             *
 * V0.211- 05/24/2001 - filtering capabilities for loading/saving parameters  *
 *         06/01/2001 - added system log                                      *
 *         06/05/2001 - added scripting capability                            *
 * V0.22 - 06/07/2001 - upgraded to shared libraries V0.17                    *
 *         06/26/2001 - added function buttons                                *
 *         06/28/2001 - dramatically sped up charting by replacing            *
 *                      TeeChart with our own code                            *
 * V0.23 - 08/15/2001 - added color scheme to output graph                    *
 * V0.24 - 01/03/2002 - minor stability improvements                          *
 * V0.25 - 01/11/2002 - fixed concurrency issues (i.e., can't send or receive *
 *                      to core modules at the same time)                     *
 * V0.26 - 04/11/2002 - Updated to Borland C++ Builder 6.0                    *
 *                      improved error handling                               *
 * V0.27 - 06/11/2004 juergen.mellinger@uni-tuebingen.de:                     *
 *                    - replaced CORECOMM and COREMESSAGE with TCPStream and  *
 *                      MessageHandler classes,                               *
 *                    - cleared up management of system state,                *
 *                    - reduced number of receiving threads to one            *
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "UMain.h"

#include "defines.h"
#include "Operator.h"
#include "USysStatus.h"
#include "USysLog.h"
#include "UScript.h"
#include "UShowStates.h"
#include "UOperatorCfg.h"
#include "UPreferences.h"
#include "UConnectionInfo.h"
#include "UOperatorUtils.h"
#include "BCIError.h"
#include "ProtocolVersion.h"
#include "Status.h"
#include "SysCommand.h"
#include "PrecisionTime.h"
#include "StateVector.h"
#include "ExecutableHelp.h"
#include "AboutBox.h"
#include "Version.h"
#include "BCIDirectory.h"

#include <sstream>

#pragma package(smart_init)
#pragma resource "*.dfm"

using namespace std;

const char* cProgramName = "Operator Module";

TfMain *fMain;

__fastcall
TfMain::TfMain( TComponent* Owner )
: TForm( Owner ),
  mTerminated( false ),
  mScript( &mParameters, &mStates, &mSyslog, this ),
  mpReceivingThread( new ReceivingThread( this ) ),
  mEEGSource( *this, EEGSource, EEGSourcePort ),
  mSigProc( *this, SigProc, SigProcPort ),
  mApp( *this, App, AppPort )
{
  istringstream iss( BCI2000_VERSION );
  iss >> mVersionInfo;
  mParameters.Add(
    "System:Configuration matrix OperatorVersion= { Framework Revision Build } 1 Operator % %"
    " % % % // operator module version information" );
  mParameters[ "OperatorVersion" ].Value( "Framework" )
    = mVersionInfo[ VersionInfo::VersionID ];
  if( mVersionInfo[ VersionInfo::Revision ].empty() )
    mParameters[ "OperatorVersion" ].Value( "Revision" )
      = mVersionInfo[ VersionInfo::SourceDate ];
  else
    mParameters[ "OperatorVersion" ].Value( "Revision" )
      = mVersionInfo[ VersionInfo::Revision ] + ", " +  mVersionInfo[ VersionInfo::SourceDate ];
  mParameters[ "OperatorVersion" ].Value( "Build" )
    = mVersionInfo[ VersionInfo::BuildDate ];
  if( bcierr__.Flushes() == 0 )
  {
    OperatorUtils::RestoreControl( this, KEY_OPERATOR );
    mPreferences.GetDefaultSettings();
    Application->OnIdle = ApplicationIdleHandler;
  }
  else
    EnterState( SYSSTATUS::Fatal );
}

TfMain::CoreConnection::CoreConnection( TfMain& inParent, MessageOrigin inOrigin, short inPort )
: mParent( inParent ),
  mOrigin( inOrigin ),
  mPort( inPort ),
  mConnected( false )
{
  mParent.mCoreConnections.insert( this );
  mParent.mSockets.insert( &mSocket );
  mSocket.set_tcpnodelay( true );
  mSocket.open( "*", mPort );
  if( !mSocket.is_open() )
    bcierr__ << "Operator: Could not open socket for listening on port "
             << mPort
             << endl;
}


TfMain::CoreConnection::~CoreConnection()
{
  mParent.mCoreConnections.erase( this );
  mParent.mSockets.erase( &mSocket );
}


__fastcall TfMain::~TfMain()
{
  mTerminated = true;
  OperatorUtils::SaveControl( this, KEY_OPERATOR );
  delete mpReceivingThread;
}

// Send a state message containing a certain state value to the EEG source module.
// This function is public because it is called from the SCRIPT class.
int
TfMain::UpdateState( const char* inName, unsigned short inValue )
{
  // We call EnterState() from here to have a consistent behavior if
  // UpdateState() is called for "Running" from a script or a button.
  if( string( "Running" ) == inName )
  {
    switch( mSysstatus.SystemState )
    {
      case SYSSTATUS::Resting:
      case SYSSTATUS::Suspended:
        if( inValue )
          EnterState( SYSSTATUS::RunningInitiated );
        break;
      case SYSSTATUS::Running:
        if( !inValue )
          EnterState( SYSSTATUS::SuspendInitiated );
        break;
      default:
        return ERR_STATENOTFOUND;
    }
  }

  if( !mStates.Exists( inName ) )
    return ERR_STATENOTFOUND;
  else
  {
    State& s = mStates[ inName ];
    s.SetValue( inValue );
    if( !mEEGSource.PutMessage( s ) )
      return ERR_SOURCENOTCONNECTED;
  }
  return ERR_NOERR;
}

bool
TfMain::SetConfig()
{
  bool result = false;
  switch( mSysstatus.SystemState )
  {
    case SYSSTATUS::Information:
    case SYSSTATUS::Initialization:
    case SYSSTATUS::Resting:
    case SYSSTATUS::RestingParamsModified:
    case SYSSTATUS::Suspended:
    case SYSSTATUS::SuspendedParamsModified:
      EnterState( SYSSTATUS::Initialization );
      result = true;
      break;
  }
  return result;
}

bool
TfMain::Run()
{
  bool result = bRunSystem->Enabled;
  if( bRunSystem->Enabled )
    UpdateState( "Running", mSysstatus.SystemState != SYSSTATUS::Running );
  return result;
}

bool
TfMain::Quit()
{
  bool result = bQuit->Enabled;
  if( bQuit->Enabled )
    QuitOperator( false );
  return result;
}

void
TfMain::UserChangedParameters()
{
  switch( fMain->mSysstatus.SystemState )
  {
    case SYSSTATUS::Resting:
      fMain->EnterState( SYSSTATUS::RestingParamsModified );
      break;

    case SYSSTATUS::Suspended:
      fMain->EnterState( SYSSTATUS::SuspendedParamsModified );
      break;
  }
}

// Some initialization code requires a fully initialized application on the VCL
// level, and thus cannot go into the TfMain constructor.
// Instead, it is performed inside an idle handler that detaches itself from
// the idle event as soon as it is called for the first time.
void
__fastcall
TfMain::ApplicationIdleHandler( TObject*, bool& )
{
  Application->OnIdle = NULL;
  mSyslog.AddSysLogEntry( "BCI2000 started" );
  SetFunctionButtons();
  UpdateDisplay();
  mpReceivingThread->Resume();
}

// Initiate program termination, and perform de-initialization that must take
// place before the destructor.
void
TfMain::Terminate()
{
  mTerminated = true;
  VisDisplay::Clear();
  mpReceivingThread->Terminate();
  Application->Terminate();
}

// Get all available BCI messages from the streams, and call the appropriate handlers.
// This function is called from the ReceivingThread inside a Synchronize() call.
void
__fastcall
TfMain::ProcessBCIMessages()
{
  // This function might be called from the VCL Synchronize() queue even after
  // destruction of the other windows.
  if( !mTerminated )
  {
    for( SetOfConnections::iterator i = mCoreConnections.begin();
                                               i != mCoreConnections.end(); ++i )
      ( *i )->ProcessBCIMessages();
    UpdateDisplay();
  }
}


void
TfMain::CoreConnection::ProcessBCIMessages()
{
  if( mSocket.connected() && !mConnected )
    OnAccept();
  else if( !mSocket.connected() && mConnected )
    OnDisconnect();
  mConnected = mSocket.connected();
  while( mStream && mStream.rdbuf()->in_avail() && !mParent.mTerminated )
  {
    HandleMessage( mStream );
#ifdef TODO
# error Move this kind of counter from SYSSTATUS into CoreConnection class.
#endif // TODO
    ++mParent.mSysstatus.NumMessagesRecv[ mOrigin ];
    if( !( mStream && mStream.is_open() ) || ( bcierr__.Flushes() > 0 ) )
      mParent.EnterState( SYSSTATUS::Fatal );
  }
}

//----------------------- Connection householding ------------------------------

// When a connection is accepted by an open socket, open the associated stream,
// and enter the appropriate information into its SYSSTATUS::Address[] entry.
void
TfMain::CoreConnection::OnAccept()
{
  mStream.clear();
  mStream.open( mSocket );
  if( mStream.is_open() )
    mParent.mSysstatus.Address[ mOrigin ]                                                \
      = AnsiString( mSocket.ip().c_str() ) + ":"
        + AnsiString( mSocket.port() );
}

// When a connection is closed, close the associated stream, and update
// the information in its SYSSTATUS::Address[] entry.
void
TfMain::CoreConnection::OnDisconnect()
{
  mStream.close();
  mSocket.open( "*", mPort );
  mParent.mSysstatus.Address[ mOrigin ] = "";
}


void
TfMain::BroadcastParameters()
{
  int numParams = mParameters.Size();
  for( SetOfConnections::iterator i = mCoreConnections.begin();
                                               i != mCoreConnections.end(); ++i )
    if( ( *i )->PutMessage( mParameters ) )
    {
      ::Sleep( 0 );
      mSysstatus.NumMessagesSent[ ( *i )->Origin() ] += numParams;
      mSysstatus.NumParametersSent[ ( *i )->Origin() ] += numParams;
    }
}

void
TfMain::BroadcastEndOfParameter()
{
  for( SetOfConnections::iterator i = mCoreConnections.begin();
                                               i != mCoreConnections.end(); ++i )
  {
    mSysstatus.INI[ ( *i )->Origin() ] = false;
    if( ( *i )->PutMessage( SysCommand::EndOfParameter ) )
      ++mSysstatus.NumMessagesSent[ ( *i )->Origin() ];
  }
}

void
TfMain::BroadcastStates()
{
  int numStates = mStates.Size();
  for( SetOfConnections::iterator i = mCoreConnections.begin();
                                               i != mCoreConnections.end(); ++i )
  {
    if( ( *i )->PutMessage( mStates ) )
    {
      ::Sleep( 0 );
      mSysstatus.NumMessagesSent[ ( *i )->Origin() ] += numStates + 1;
      mSysstatus.NumStatesSent[ ( *i )->Origin() ] += numStates;
    }
  }
}

void
TfMain::BroadcastEndOfState()
{
  for( SetOfConnections::iterator i = mCoreConnections.begin();
                                               i != mCoreConnections.end(); ++i )
    if( ( *i )->PutMessage( SysCommand::EndOfState ) )
      ++mSysstatus.NumMessagesSent[ ( *i )->Origin() ];
}

void
TfMain::InitializeModules()
{
  if( mEEGSource.PutMessage( SignalProperties( 0, 0 ) ) )
    ++mSysstatus.NumMessagesSent[ EEGSource ];
}

// Here we list the actions to be taken for allowed state transitions.
#define TRANSITION( a, b )  ( ( int( a ) << 8 ) | int( b ) )
void
TfMain::EnterState( SYSSTATUS::State inState )
{
  int transition = TRANSITION( mSysstatus.SystemState, inState );
  SYSSTATUS::State prevState = mSysstatus.SystemState;
  mSysstatus.SystemState = inState;
  switch( transition )
  {
    case TRANSITION( SYSSTATUS::Idle, SYSSTATUS::Publishing ):
      mStarttime = TDateTime::CurrentDateTime();
      break;

    case TRANSITION( SYSSTATUS::Publishing, SYSSTATUS::Publishing ):
      break;

    case TRANSITION( SYSSTATUS::Publishing, SYSSTATUS::Information ):
      // Execute the script after all modules are connected ...
      if( mPreferences.Script[ PREFERENCES::AfterModulesConnected ] != "" )
      {
        mSyslog.AddSysLogEntry( "Executing script after all modules connected ..." );
        mScript.ExecuteScript( mPreferences.Script[ PREFERENCES::AfterModulesConnected ].c_str() );
      }
      // Add the state vector's length to the system parameters.
      {
        mParameters.Add(
          "System:State%20Vector"
          " int StateVectorLength= 0 16 1 30 "
          "// length of the state vector in bytes" );
        mStates.AssignPositions();
        AnsiString length = StateVector( mStates ).Length();
        mParameters[ "StateVectorLength" ].Value() = length.c_str();
      }
      break;

    case TRANSITION( SYSSTATUS::Information, SYSSTATUS::Information ):
    case TRANSITION( SYSSTATUS::Information, SYSSTATUS::Initialization ):
      MaintainDebugLog();
      BroadcastParameters();
      BroadcastEndOfParameter();
      BroadcastStates();
      BroadcastEndOfState();
      InitializeModules();
      // Send a system command 'Start' to the EEGsource (currently, it will be ignored).
      if( mEEGSource.PutMessage( SysCommand::Start ) )
      {
        ++mSysstatus.NumMessagesSent[ EEGSource ];
        ++mSysstatus.NumStatesSent[ EEGSource ];
      }
      mSyslog.AddSysLogEntry( "Operator set configuration" );
      break;

    case TRANSITION( SYSSTATUS::Initialization, SYSSTATUS::Resting ):
      if( mPreferences.Script[ PREFERENCES::OnSetConfig ] != "" )
      {
        mSyslog.AddSysLogEntry( "Executing OnSetConfig script ..." );
        mScript.ExecuteScript( mPreferences.Script[ PREFERENCES::OnSetConfig ].c_str() );
      }
      break;

    case TRANSITION( SYSSTATUS::Initialization, SYSSTATUS::Initialization ):
    case TRANSITION( SYSSTATUS::Resting, SYSSTATUS::Initialization ):
    case TRANSITION( SYSSTATUS::Suspended, SYSSTATUS::Initialization ):
    case TRANSITION( SYSSTATUS::SuspendedParamsModified, SYSSTATUS::Initialization ):
    case TRANSITION( SYSSTATUS::RestingParamsModified, SYSSTATUS::Initialization ):
      MaintainDebugLog();
      BroadcastParameters();
      BroadcastEndOfParameter();
      InitializeModules();
      mSyslog.AddSysLogEntry( "Operator set configuration" );
      break;

    case TRANSITION( SYSSTATUS::Resting, SYSSTATUS::RunningInitiated ):
      MaintainDebugLog();
      // Execute the on-start script ...
      if( mPreferences.Script[ PREFERENCES::OnStart ] != "" )
      {
        mSyslog.AddSysLogEntry( "Executing OnStart script ..." );
        mScript.ExecuteScript( mPreferences.Script[ PREFERENCES::OnStart ].c_str() );
      }
      mStarttime = TDateTime::CurrentDateTime();
      mSyslog.AddSysLogEntry( "Operator started operation" );
      break;

    case TRANSITION( SYSSTATUS::Suspended, SYSSTATUS::RunningInitiated ):
      MaintainDebugLog();
      // Execute the on-resume script ...
      if( mPreferences.Script[ PREFERENCES::OnResume ] != "" )
      {
        mSyslog.AddSysLogEntry( "Executing OnResume script ..." );
        mScript.ExecuteScript( mPreferences.Script[ PREFERENCES::OnResume ].c_str() );
      }
      mStarttime = TDateTime::CurrentDateTime();
      mSyslog.AddSysLogEntry( "Operator resumed operation" );
      break;

    case TRANSITION( SYSSTATUS::RunningInitiated, SYSSTATUS::Running ):
      break;

    case TRANSITION( SYSSTATUS::Running, SYSSTATUS::SuspendInitiated ):
      mStarttime = TDateTime::CurrentDateTime();
      mSyslog.AddSysLogEntry( "Operation suspended" );
      break;

    case TRANSITION( SYSSTATUS::SuspendInitiated, SYSSTATUS::SuspendInitiated ):
      break;

    case TRANSITION( SYSSTATUS::SuspendInitiated, SYSSTATUS::Suspended ):
      BroadcastParameters(); // no EndOfParameter

      // Execute the on-suspend script ...
      if( mPreferences.Script[ PREFERENCES::OnSuspend ] != "" )
      {
        mSyslog.AddSysLogEntry( "Executing OnSuspend script ..." );
        mScript.ExecuteScript( mPreferences.Script[ PREFERENCES::OnSuspend ].c_str() );
      }
      break;

    case TRANSITION( SYSSTATUS::Suspended, SYSSTATUS::SuspendedParamsModified ):
    case TRANSITION( SYSSTATUS::Resting, SYSSTATUS::RestingParamsModified ):
      break;

#if 0
    case TRANSITION( SYSSTATUS::Suspended, SYSSTATUS::Suspended ):
      break;
#endif

    case TRANSITION( SYSSTATUS::Idle, SYSSTATUS::Fatal ):
    case TRANSITION( SYSSTATUS::Publishing, SYSSTATUS::Fatal ):
    case TRANSITION( SYSSTATUS::Information, SYSSTATUS::Fatal ):
    case TRANSITION( SYSSTATUS::Initialization, SYSSTATUS::Fatal ):
    case TRANSITION( SYSSTATUS::Resting, SYSSTATUS::Fatal ):
    case TRANSITION( SYSSTATUS::RestingParamsModified, SYSSTATUS::Fatal ):
    case TRANSITION( SYSSTATUS::RunningInitiated, SYSSTATUS::Fatal ):
    case TRANSITION( SYSSTATUS::Running, SYSSTATUS::Fatal ):
    case TRANSITION( SYSSTATUS::SuspendInitiated, SYSSTATUS::Fatal ):
    case TRANSITION( SYSSTATUS::Suspended, SYSSTATUS::Fatal ):
    case TRANSITION( SYSSTATUS::SuspendedParamsModified, SYSSTATUS::Fatal ):
    case TRANSITION( SYSSTATUS::Fatal, SYSSTATUS::Fatal ):
      break;

    default:
      bcierr << "Unexpected system state transition: "
             << prevState << " -> " << inState
             << endl;
  }
  UpdateDisplay();
}

void
TfMain::QuitOperator( bool inConfirm )
{
  if( !inConfirm || ID_YES == Application->MessageBox(
    "Do you really want to quit BCI2000?", "Question", MB_YESNO ) )
  {
    // Execute the on-exit script ...
    if( mPreferences.Script[ PREFERENCES::OnExit ] != "" )
    {
      mSyslog.AddSysLogEntry( "Executing OnExit script ..." );
      mScript.ExecuteScript( mPreferences.Script[ PREFERENCES::OnExit ].c_str() );
    }
    mSyslog.Close( true );
    // Store the settings in the ini file.
    mPreferences.SetDefaultSettings();
    // Send a system command 'Reset' to the EEGsource.
    if( mEEGSource.PutMessage( SysCommand::Reset ) )
      ++mSysstatus.NumMessagesSent[ EEGSource ];
    // Quit the operator.
    Terminate();
  }
}

// This function is called after each received message has been processed.
void
TfMain::UpdateDisplay()
{
  TDateTime  timeElapsed = TDateTime::CurrentDateTime() - mStarttime;
  AnsiString windowCaption = TXT_WINDOW_CAPTION " ",
             statusText = "N/A";
  windowCaption += mVersionInfo[ "Version" ].c_str();

  switch( mSysstatus.SystemState )
  {
    case SYSSTATUS::Idle:
      statusText = "System Status: < idle >";
      break;
    case SYSSTATUS::Publishing:
      statusText = "Publishing Phase ...";
      break;
    case SYSSTATUS::Information:
      statusText = "Information Phase ...";
      break;
    case SYSSTATUS::Initialization:
      statusText = "Initialization Phase ...";
      break;
    case SYSSTATUS::Resting:
    case SYSSTATUS::RestingParamsModified:
    case SYSSTATUS::SuspendInitiated:
    case SYSSTATUS::Suspended:
    case SYSSTATUS::SuspendedParamsModified:
      windowCaption += " - " TXT_OPERATOR_SUSPENDED " " + timeElapsed.FormatString( "nn:ss" ) + " s";
      statusText = TXT_OPERATOR_SUSPENDED;
      break;
    case SYSSTATUS::RunningInitiated:
    case SYSSTATUS::Running:
      windowCaption += " - " TXT_OPERATOR_RUNNING " " + timeElapsed.FormatString( "nn:ss" ) + " s";
      statusText = TXT_OPERATOR_RUNNING;
      break;
    case SYSSTATUS::Fatal:
      statusText = "Fatal Error ...";
  }

  if( Caption != windowCaption )
    Caption = windowCaption;
  mSysstatus.Status[ Operator ] = statusText;
  for( int i = 0; i < numMessageOrigins; ++i )
    if( StatusBar->Panels->Items[ i ]->Text != mSysstatus.Status[ i ] )
      StatusBar->Panels->Items[ i ]->Text = mSysstatus.Status[ i ];

  AnsiString runSystemCaption = "Start";
  bool       configEnabled = false,
             setConfigEnabled = false,
             runSystemEnabled = false,
             quitEnabled = false;

  switch( mSysstatus.SystemState )
  {
    case SYSSTATUS::Idle:
      quitEnabled = true;
      break;
    case SYSSTATUS::Publishing:
      quitEnabled = true;
      break;
    case SYSSTATUS::Information:
      configEnabled = true;
      setConfigEnabled = true;
      quitEnabled = true;
      break;
    case SYSSTATUS::Initialization:
      configEnabled = true;
      setConfigEnabled = true;
      quitEnabled = true;
      break;
    case SYSSTATUS::Resting:
      runSystemEnabled = true;
      /* no break */
    case SYSSTATUS::RestingParamsModified:
      configEnabled = true;
      setConfigEnabled = true;
      quitEnabled = true;
      break;
    case SYSSTATUS::SuspendInitiated:
      runSystemCaption = "Resume";
      configEnabled = true;
      quitEnabled = true;
      break;
    case SYSSTATUS::Suspended:
      runSystemEnabled = true;
      /* no break */
    case SYSSTATUS::SuspendedParamsModified:
      runSystemCaption = "Resume";
      configEnabled = true;
      setConfigEnabled = true;
      quitEnabled = true;
      break;
    case SYSSTATUS::RunningInitiated:
      runSystemCaption = "Suspend";
      break;
    case SYSSTATUS::Running:
      runSystemCaption = "Suspend";
      runSystemEnabled = true;
      break;
    case SYSSTATUS::Fatal:
      quitEnabled = true;
      break;
  }
  // To avoid unnecessary redraws of controls (flicker), we check for changed
  // captions before actually assigning them.
  if( bRunSystem->Caption != runSystemCaption )
    bRunSystem->Caption = runSystemCaption;
  if( bConfig->Enabled != configEnabled )
    bConfig->Enabled = configEnabled;
  if( bSetConfig->Enabled != setConfigEnabled )
    bSetConfig->Enabled = setConfigEnabled;
  if( bRunSystem->Enabled != runSystemEnabled )
    bRunSystem->Enabled = runSystemEnabled;
  if( bQuit->Enabled != quitEnabled )
    bQuit->Enabled = quitEnabled;

  fConnectionInfo->UpdateDisplay( mSysstatus );
}

void TfMain::SetFunctionButtons()
{
  #define SET_BUTTON( number ) \
  if( mPreferences.Buttons[ number ].Name != ""                       \
      && mPreferences.Buttons[ number ].Cmd != "" )                   \
  {                                                                   \
    bFunction##number->Enabled = true;                                \
    bFunction##number->Caption = mPreferences.Buttons[ number ].Name; \
  }                                                                   \
  else                                                                \
  {                                                                   \
    bFunction##number->Enabled = false;                               \
    bFunction##number->Caption = "Function " #number;                 \
  }
  SET_BUTTON( 1 );
  SET_BUTTON( 2 );
  SET_BUTTON( 3 );
  SET_BUTTON( 4 );
}

void
__fastcall
TfMain::ReceivingThread::Execute()
{
  const socketTimeout = 500; // ms
  while( !TThread::Terminated )
    if( streamsock::wait_for_read( mParent.mSockets, socketTimeout, true ) )
      TThread::Synchronize( &mParent.ProcessBCIMessages );
}

//--------------------- Handlers for BCI messages ------------------------------
bool
TfMain::CoreConnection::HandleProtocolVersion( istream& is )
{
  ProtocolVersion version;
  if( version.ReadBinary( is ) )
    mParent.mSysstatus.Version[ mOrigin ] = version;
  return true;
}

bool
TfMain::CoreConnection::HandleStatus( istream& is )
{
  Status status;
  if( status.ReadBinary( is ) )
  {
    mParent.mSysstatus.Status[ mOrigin ] = status.Message().c_str();
    switch( status.Content() )
    {
      case Status::debug:
        mParent.mSyslog.AddSysLogEntry( status.Message().c_str() );
        mParent.mDebugLog << status.Message() << endl;
        break;
      case Status::warning:
        // If we receive a warning message, add a line to the system log and bring it to front.
        mParent.mSyslog.AddSysLogEntry( status.Message().c_str(), SYSLOG::logEntryWarning );
        mParent.mSyslog.ShowSysLog();
        break;
      case Status::error:
        // If we receive an error message, add a line to the system log and bring it to front.
        mParent.mSyslog.AddSysLogEntry( status.Message().c_str(), SYSLOG::logEntryError );
        mParent.mSyslog.ShowSysLog();
        break;
      case Status::initialized:
        // If the operator received successful status messages from
        // all core modules, then this is the end of the initialization phase.
        if( mParent.mSysstatus.SystemState == SYSSTATUS::Initialization )
        {
          mParent.mSysstatus.INI[ mOrigin ] = true;
          switch( mOrigin )
          {
            case EEGSource:
              mParent.mSyslog.AddSysLogEntry( "Source confirmed new parameters ..." );
              break;
            case SigProc:
              mParent.mSyslog.AddSysLogEntry( "Signal Processing confirmed new parameters ..." );
              break;
            case App:
              mParent.mSyslog.AddSysLogEntry( "User Application confirmed new parameters ..." );
              break;
          }
        }
        break;
      case Status::running:
        mParent.mSysstatus.runningConfirmed[ mOrigin ] = true;
        break;
      case Status::suspended:
        mParent.mSysstatus.suspendConfirmed[ mOrigin ] = true;
        break;
      default:
        mParent.mSyslog.AddSysLogEntry( status.Message().c_str() );
    }

    if( mParent.mSysstatus.INI[ EEGSource ]
        && mParent.mSysstatus.INI[ SigProc ]
        && mParent.mSysstatus.INI[ App ]
        && mParent.mSysstatus.SystemState == SYSSTATUS::Initialization )
    {
      mParent.EnterState( SYSSTATUS::Resting );
    }

    if( mParent.mSysstatus.suspendConfirmed[ EEGSource ]
       && mParent.mSysstatus.suspendConfirmed[ SigProc ]
       && mParent.mSysstatus.suspendConfirmed[ App ]
        && mParent.mSysstatus.SystemState == SYSSTATUS::SuspendInitiated )
    {
      mParent.mSysstatus.suspendConfirmed[ EEGSource ] = false;
      mParent.mSysstatus.suspendConfirmed[ SigProc ] = false;
      mParent.mSysstatus.suspendConfirmed[ App ] = false;
      mParent.EnterState( SYSSTATUS::Suspended );
    }

    if( mParent.mSysstatus.runningConfirmed[ EEGSource ]
       && mParent.mSysstatus.runningConfirmed[ SigProc ]
       && mParent.mSysstatus.runningConfirmed[ App ]
        && mParent.mSysstatus.SystemState == SYSSTATUS::RunningInitiated )
    {
      mParent.mSysstatus.runningConfirmed[ EEGSource ] = false;
      mParent.mSysstatus.runningConfirmed[ SigProc ] = false;
      mParent.mSysstatus.runningConfirmed[ App ] = false;
      mParent.EnterState( SYSSTATUS::Running );
    }
  }
  return true;
}

bool
TfMain::CoreConnection::HandleSysCommand( istream& is )
{
  SysCommand syscmd;
  if( syscmd.ReadBinary( is ) )
  {
    if( syscmd == SysCommand::Reset )
    {
      mParent.Terminate();
    }
    else if( syscmd == SysCommand::Suspend )
    {
      mParent.EnterState( SYSSTATUS::SuspendInitiated );
    }
    else if( syscmd == SysCommand::EndOfParameter )
    {
      /* do nothing */
    }
    // The operator receiving 'EndOfState' marks the end of the publishing phase.
    else if( syscmd == SysCommand::EndOfState )
    {
      mParent.mSysstatus.EOS[ mOrigin ] = true;
      if( !mParent.mSysstatus.Version[ mOrigin ].Matches( ProtocolVersion::Current() ) )
      {
        string name = "N/A";
        switch( mOrigin )
        {
          case EEGSource:
            name = "Source";
            break;
          case SigProc:
            name = "Signal Processing";
            break;
          case App:
            name = "Application";
            break;
        }
        string older = name,
               newer = "Operator";
        if( mParent.mSysstatus.Version[ mOrigin ].MoreRecentThan( ProtocolVersion::Current() ) )
          swap( older, newer );

        bcierr__ << "Protocol version mismatch between Operator and "
                 << name << " module. \n"
                 << "The " << newer << " module appears to be more recent than the "
                 << older << " module. \n\n"
                 << "Please make sure that all modules share the same"
                 << " BCI2000 major version number. \n"
                 << "The operator module's version number is "
                 << mParent.mParameters[ "OperatorVersion" ].Value( "Framework" )
                 << ". \n\n"
                 << "BCI2000 will now quit."
                 << endl;
        mParent.Terminate();
      }
    }
    // If we received EndOfStates from all the modules, then make the transition
    // to the next system state.
    if( mParent.mSysstatus.EOS[ EEGSource ]
        && mParent.mSysstatus.EOS[ SigProc ]
        && mParent.mSysstatus.EOS[ App ]
        && mParent.mSysstatus.SystemState == SYSSTATUS::Publishing )
    {
      mParent.EnterState( SYSSTATUS::Information );
    }
  }
  return true;
}

bool
TfMain::CoreConnection::HandleParam( istream& is )
{
  Param param;
  if( param.ReadBinary( is ) )
  {
    ++mParent.mSysstatus.NumParametersRecv[ mOrigin ];
    mParent.mParameters.Add( param, mOrigin );
    // Update the parameter in the configuration window.
    fConfig->RenderParameter( &mParent.mParameters[ param.Name() ] );
  }
  return true;
}

bool
TfMain::CoreConnection::HandleState( istream& is )
{
  State state;
  if( state.ReadBinary( is ) )
  {
    ++mParent.mSysstatus.NumStatesRecv[ mOrigin ];
    mParent.mStates.Delete( state.Name() );
    mParent.mStates.Add( state );
    mParent.EnterState( SYSSTATUS::Publishing );
  }
  return true;
}

bool
TfMain::CoreConnection::HandleVisSignal( istream& is )
{
  VisSignal visSignal;
  if( visSignal.ReadBinary( is ) )
  {
    ++mParent.mSysstatus.NumDataRecv[ mOrigin ];
    VisDisplay::HandleMessage( visSignal );
  }
  return true;
}

bool
TfMain::CoreConnection::HandleVisSignalProperties( istream& is )
{
  VisSignalProperties visSignalProperties;
  if( visSignalProperties.ReadBinary( is ) )
  {
    ++mParent.mSysstatus.NumDataRecv[ mOrigin ];
    VisDisplay::HandleMessage( visSignalProperties );
  }
  return true;
}

bool
TfMain::CoreConnection::HandleVisMemo( istream& is )
{
  VisMemo visMemo;
  if( visMemo.ReadBinary( is ) )
  {
    ++mParent.mSysstatus.NumDataRecv[ mOrigin ];
    VisDisplay::HandleMessage( visMemo );
  }
  return true;
}

bool
TfMain::CoreConnection::HandleVisBitmap( istream& is )
{
  VisBitmap visBitmap;
  if( visBitmap.ReadBinary( is ) )
  {
    ++mParent.mSysstatus.NumDataRecv[ mOrigin ];
    VisDisplay::HandleMessage( visBitmap );
  }
  return true;
}

bool
TfMain::CoreConnection::HandleVisCfg( istream& is )
{
  VisCfg visCfg;
  if( visCfg.ReadBinary( is ) )
  {
    ++mParent.mSysstatus.NumDataRecv[ mOrigin ];
    VisDisplay::HandleMessage( visCfg );
  }
  return true;
}

//------------------ IDE-managed VCL event handlers -------------------------

void __fastcall TfMain::bQuitClick( TObject* )
{
  QuitOperator();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::FormClose( TObject*, TCloseAction &outAction )
{
  if( !mSyslog.Close() )
    outAction = caNone;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bRunSystemClick( TObject* )
{
  Run();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bConfigClick( TObject* )
{
  fPreferences->preferences = &mPreferences;
  fConfig->OnParameterChange( UserChangedParameters );
  fConfig->Initialize( &mParameters, &mPreferences );
  fConfig->Show();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bSetConfigClick( TObject* )
{
  if( fConfig->Visible )
    fConfig->Close();

  SetConfig();
}

//---------------------------------------------------------------------------

void __fastcall TfMain::bShowConnectionInfoClick( TObject* )
{
  fConnectionInfo->Show();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::Exit1Click( TObject* )
{
  QuitOperator();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::N1Click( TObject* )
{
  fPreferences->preferences = &mPreferences;
  fPreferences->ShowModal();
  SetFunctionButtons();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::HelpClick(TObject *Sender)
{
  ExecutableHelp()
  .Display();
}

//---------------------------------------------------------------------------
void __fastcall TfMain::About1Click( TObject* )
{
  AboutBox()
  .SetApplicationName( cProgramName )
  .Display();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::States1Click( TObject* )
{
  fShowStates->statelist = &mStates;
  fShowStates->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::OperatorLog1Click( TObject* )
{
  if( mSyslog.Visible() )
    mSyslog.HideSysLog();
  else
    mSyslog.ShowSysLog();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::ConnectionInfo1Click( TObject* )
{
  fConnectionInfo->Visible = !fConnectionInfo->Visible;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::View1Click( TObject* )
{
  ConnectionInfo1->Checked = fConnectionInfo->Visible;
  OperatorLog1->Checked = mSyslog.Visible();
}
//---------------------------------------------------------------------------
#define BUTTON_CLICK( number ) \
void __fastcall TfMain::bFunction##number##Click( TObject* )            \
{                                                                       \
  mScript.ExecuteCommand( mPreferences.Buttons[ number ].Cmd.c_str() ); \
}
BUTTON_CLICK( 1 )
BUTTON_CLICK( 2 )
BUTTON_CLICK( 3 )
BUTTON_CLICK( 4 )

//---------------------------------------------------------------------------
void
TfMain::MaintainDebugLog()
{
  // Enable debug logging if requested.
  // The method to obtain the debugging file's name is rather crude but difficult
  // to get right in the presence of auto-incrementing run numbers.
  if( mParameters.Exists( "DebugLog" ) && ::atoi( mParameters[ "DebugLog" ].Value().c_str() ) != 0 )
  {
    BCIDirectory bciDir;
    bciDir.SetDataDirectory( mParameters[ "DataDirectory" ].Value() )
          .SetSubjectName( mParameters[ "SubjectName" ].Value() )
          .SetSessionNumber( ::atoi( mParameters[ "SubjectSession" ].Value().c_str() ) )
          .SetRunNumber( ::atoi( mParameters[ "SubjectRun" ].Value().c_str() ) );

    string extension = ".";
    if( mParameters.Exists( "FileFormat" ) )
    {
      extension += mParameters[ "FileFormat" ].Value();
      bciDir.SetFileExtension( extension );
    }
    string filePath = bciDir.CreatePath().FilePath() + ".dbg";
    mDebugLog.close();
    mDebugLog.clear();
    mDebugLog.open( filePath.c_str(), ios_base::out | ios_base::app );
  }
  else
    mDebugLog.close();
}

