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

#include "..\shared\defines.h"
#include "Operator.h"
#include "USysStatus.h"
#include "USysLog.h"
#include "UScript.h"
#include "UAbout.h"
#include "UShowStates.h"
#include "UOperatorCfg.h"
#include "UPreferences.h"
#include "UConnectionInfo.h"
#include "UOperatorUtils.h"
#include "UBCIError.h"
#include "UStatus.h"
#include "USysCommand.h"
#include "UBCITime.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

using namespace std;

TfMain *fMain;

__fastcall
TfMain::TfMain( TComponent* Owner )
: TForm( Owner ),
  mTerminated( false ),
  mScript( &mParameters, &mStates, &mSyslog, this ),
  mpReceivingThread( new ReceivingThread( this ) ),
  mMessageHandler( *this )
{
  const struct
  {
    tcpsocket*    socket;
    int           port;
    tcpstream*    stream;
    SocketHandler acceptHandler,
                  disconnectHandler;
  } socketInfo[] =
  {
    #define INFO_ENTRY( module ) \
    {                                                                           \
      &m##module##Socket,                                                       \
      module##Port,                                                             \
      &m##module,                                                               \
      &TfMain::module##SocketAccept,                                            \
      &TfMain::module##SocketDisconnect                                         \
    },
    INFO_ENTRY( EEGSource )
    INFO_ENTRY( SigProc )
    INFO_ENTRY( App )
  };
  for( int i = 0; i < sizeof( socketInfo ) / sizeof( *socketInfo ); ++i )
  {
    tcpsocket* socket = socketInfo[ i ].socket;
    socket->open( "*", socketInfo[ i ].port );
    if( !socket->is_open() )
      __bcierr << "Operator: Could not open socket for listening on port "
               << socketInfo[ i ].port
               << endl;
    mSockets.insert( socket );
    mAcceptHandlers[ socket ] = socketInfo[ i ].acceptHandler;
    mDisconnectHandlers[ socket ] = socketInfo[ i ].disconnectHandler;
    mStreams[ socket ] = socketInfo[ i ].stream;
    mConnectionStates[ socket ] = false;
  }

  if( __bcierr.flushes() == 0 )
  {
    OperatorUtils::RestoreControl( this );
    mPreferences.GetDefaultSettings();
    Application->OnIdle = ApplicationIdleHandler;
  }
  else
    Terminate();
}

__fastcall TfMain::~TfMain()
{
  OperatorUtils::SaveControl( this );
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
          EnterState( SYSSTATUS::Running );
        break;
      case SYSSTATUS::Running:
        if( !inValue )
          EnterState( SYSSTATUS::Suspended );
        break;
      default:
        return ERR_STATENOTFOUND;
    }
  }

  STATE* s = mStates.GetStatePtr( inName );
  if( s == NULL )
    return ERR_STATENOTFOUND;
  s->SetValue( inValue );
  if( !MessageHandler::PutMessage( mEEGSource, *s ) )
    return ERR_SOURCENOTCONNECTED;
  return ERR_NOERR;
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
  mpReceivingThread->Terminate();
  Application->Terminate();
}

// Get all available BCI messages from the streams, and call the appropriate handlers.
// This function is called from the ReceivingThread inside a Synchronize() call.
void
__fastcall
TfMain::ProcessBCIMessages()
{
  for( tcpsocket::set_of_instances::iterator i = mSockets.begin(); i != mSockets.end(); ++i )
  {
    if( ( *i )->connected() && !mConnectionStates[ *i ] )
      ( this->*mAcceptHandlers[ *i ] )();
    else if( !( *i )->connected() && mConnectionStates[ *i ] )
      ( this->*mDisconnectHandlers[ *i ] )();
    mConnectionStates[ *i ] = ( *i )->connected();
    while( *mStreams[ *i ] && mStreams[ *i ]->rdbuf()->in_avail() && !mTerminated )
    {
      mMessageHandler.HandleMessage( *mStreams[ *i ] );
      ++mSysstatus.NumMessagesRecv[ Origin( *mStreams[ *i ] ) ];
      if( !( *mStreams[ *i ] && mStreams[ *i ]->is_open() ) || ( __bcierr.flushes() > 0 ) )
        Terminate();
    }
  }
  UpdateDisplay();
}

//----------------------- Connection householding ------------------------------

// When a connection is accepted by an open socket, open the associated stream,
// and enter the appropriate information into its SYSSTATUS::Address[] entry.
#define SOCKET_ACCEPT_DEF( module ) \
void                                                                            \
TfMain::module##SocketAccept()                                                  \
{                                                                               \
  m##module.clear();                                                            \
  m##module.open( m##module##Socket );                                          \
  if( m##module.is_open() )                                                     \
    mSysstatus.Address[ module ]                                                \
      = AnsiString( m##module##Socket.ip().c_str() ) + ":"                      \
        + AnsiString( m##module##Socket.port() );                               \
}
SOCKET_ACCEPT_DEF( EEGSource )
SOCKET_ACCEPT_DEF( SigProc )
SOCKET_ACCEPT_DEF( App )

// When a connection is closed, close the associated stream, and update the
// the information in its SYSSTATUS::Address[] entry.
#define SOCKET_DISCONNECT_DEF( module ) \
void                                                                            \
TfMain::module##SocketDisconnect()                                              \
{                                                                               \
  m##module.close();                                                            \
  m##module##Socket.open( "*", module##Port );                                  \
  mSysstatus.Address[ module ] = "";                                            \
}
SOCKET_DISCONNECT_DEF( EEGSource )
SOCKET_DISCONNECT_DEF( SigProc )
SOCKET_DISCONNECT_DEF( App )

// Given a stream, return the index of its associated module.
MessageOrigin
TfMain::Origin( istream& is )
{
  if( &is == &mEEGSource )
    return EEGSource;
  if( &is == &mSigProc )
    return SigProc;
  if( &is == &mApp )
    return App;
  return Operator;
}

//------------------ Medium level aggregate functions --------------------------

void
TfMain::BroadcastParameters()
{
  if( mEEGSource.is_open() && mSigProc.is_open() && mApp.is_open() )
  {
    int numParams = mParameters.GetNumParameters();
    if( MessageHandler::PutMessage( mEEGSource, mParameters ) )
    {
      mEEGSource.flush();
      ::Sleep( 0 );
      mSysstatus.NumMessagesSent[ EEGSource ] += numParams + 1;
      mSysstatus.NumParametersSent[ EEGSource ] += numParams;
    }
    if( MessageHandler::PutMessage( mSigProc, mParameters ) )
    {
      mSigProc.flush();
      ::Sleep( 0 );
      mSysstatus.NumMessagesSent[ SigProc ] += numParams + 1;
      mSysstatus.NumParametersSent[ SigProc ] += numParams;
    }
    if( MessageHandler::PutMessage( mApp, mParameters ) )
    {
      mApp.flush();
      ::Sleep( 0 );
      mSysstatus.NumMessagesSent[ App ] += numParams + 1;
      mSysstatus.NumParametersSent[ App ] += numParams;
    }
  }
}

void
TfMain::BroadcastStates()
{
  if( mEEGSource.is_open() && mSigProc.is_open() && mApp.is_open() )
  {
    int numStates = mStates.GetNumStates();
    if( MessageHandler::PutMessage( mEEGSource, mStates ) )
    {
      mEEGSource.flush();
      ::Sleep( 0 );
      mSysstatus.NumMessagesSent[ EEGSource ] += numStates + 1;
      mSysstatus.NumStatesSent[ EEGSource ] += numStates;
    }
    if( MessageHandler::PutMessage( mSigProc, mStates ) )
    {
      mSigProc.flush();
      ::Sleep( 0 );
      mSysstatus.NumMessagesSent[ SigProc ] += numStates + 1;
      mSysstatus.NumStatesSent[ SigProc ] += numStates;
    }
    if( MessageHandler::PutMessage( mApp, mStates ) )
    {
      mApp.flush();
      ::Sleep( 0 );
      mSysstatus.NumMessagesSent[ App ] += numStates + 1;
      mSysstatus.NumStatesSent[ App ] += numStates;
    }
  }
}

// Here we list the actions to be taken for the allowed state transitions.
#define TRANSITION( a, b )  ( ( int( a ) << 8 ) | int( b ) )
void
TfMain::EnterState( SYSSTATUS::State inState )
{
  int transition = TRANSITION( mSysstatus.SystemState, inState );
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
        mParameters.AddParameter2List(
          "System int StateVectorLength= 0 16 1 30 // length of the state vector in bytes" );
        mParameters.Sort();
        AnsiString value = STATEVECTOR( &mStates ).GetStateVectorLength();
        mParameters[ "StateVectorLength" ].SetValue( value.c_str() );
      }
      break;

    case TRANSITION( SYSSTATUS::Information, SYSSTATUS::Initialization ):
      BroadcastParameters();
      BroadcastStates();
      // Send a system command 'Start' to the EEGsource (currently, it will be ignored).
      if( MessageHandler::PutMessage( mEEGSource, SYSCMD::Start ) )
      {
        ++mSysstatus.NumMessagesSent[ EEGSource ];
        ++mSysstatus.NumStatesSent[ EEGSource ];
      }
      mSyslog.AddSysLogEntry( "Operator set configuration" );
      break;

    case TRANSITION( SYSSTATUS::Initialization, SYSSTATUS::Resting ):
      break;

    case TRANSITION( SYSSTATUS::Initialization, SYSSTATUS::Initialization ):
    case TRANSITION( SYSSTATUS::Resting, SYSSTATUS::Resting ):
    case TRANSITION( SYSSTATUS::Suspended, SYSSTATUS::Resting ):
      BroadcastParameters();
      mSyslog.AddSysLogEntry( "Operator set configuration" );
      break;

    case TRANSITION( SYSSTATUS::Resting, SYSSTATUS::Running ):
      // Execute the on-start script ...
      if( mPreferences.Script[ PREFERENCES::OnStart ] != "" )
      {
        mSyslog.AddSysLogEntry( "Executing on-start script ..." );
        mScript.ExecuteScript( mPreferences.Script[ PREFERENCES::OnStart ].c_str() );
      }
      mStarttime = TDateTime::CurrentDateTime();
      mSyslog.AddSysLogEntry( "Operator started operation" );
      break;

    case TRANSITION( SYSSTATUS::Suspended, SYSSTATUS::Running ):
      mSyslog.AddSysLogEntry( "Operator resumed operation" );
      // Execute the on-resume script ...
      if( mPreferences.Script[ PREFERENCES::OnResume ] != "" )
      {
        mSyslog.AddSysLogEntry( "Executing on-resume script ..." );
        mScript.ExecuteScript( mPreferences.Script[ PREFERENCES::OnResume ].c_str() );
      }
      mStarttime = TDateTime::CurrentDateTime();
      break;

    case TRANSITION( SYSSTATUS::Running, SYSSTATUS::Suspended ):
      mSyslog.AddSysLogEntry( "Operator suspended operation" );
      // Execute the on-suspend script ...
      if( mPreferences.Script[ PREFERENCES::OnSuspend ] != "" )
      {
        mSyslog.AddSysLogEntry( "Executing on-suspend script ..." );
        mScript.ExecuteScript( mPreferences.Script[ PREFERENCES::OnSuspend ].c_str() );
      }
      mStarttime = TDateTime::CurrentDateTime();
      break;

    case TRANSITION( SYSSTATUS::Suspended, SYSSTATUS::Suspended ):
      break;

    default:
      bcierr << "Unexpected system state transition: "
             << mSysstatus.SystemState << " -> " << inState
             << endl;
  }
  UpdateDisplay();
}

void
TfMain::QuitOperator()
{
  if( ID_YES == Application->MessageBox(
    "Do you really want to quit BCI2000?", "Question", MB_YESNO ) )
  {
    // Execute the on-exit script ...
    if( mPreferences.Script[ PREFERENCES::OnExit ] != "" )
    {
      mSyslog.AddSysLogEntry( "Executing on-exit script ..." );
      mScript.ExecuteScript( mPreferences.Script[ PREFERENCES::OnExit ].c_str() );
    }
    mSyslog.Close( true );
    // Store the settings in the ini file.
    mPreferences.SetDefaultSettings();
    // Send a system command 'Reset' to the EEGsource.
    if( mEEGSource.is_open() && MessageHandler::PutMessage( mEEGSource, SYSCMD::Reset ) )
      ++mSysstatus.NumMessagesSent[ EEGSource ];
    // Quit the operator.
    Terminate();
  }
}

// This function is called after each received message has been processed.
// To avoid unnecessary redraws of controls (flicker), we check for changed
// captions before actually assigning them.
void
TfMain::UpdateDisplay()
{
  TDateTime  timeElapsed = TDateTime::CurrentDateTime() - mStarttime;
  AnsiString windowCaption = TXT_WINDOW_CAPTION " " TXT_OPERATOR_VERSION,
             statusText = "N/A";

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
    case SYSSTATUS::Suspended:
      windowCaption += " - " TXT_OPERATOR_SUSPENDED " " + timeElapsed.FormatString( "nn:ss" ) + " s";
      statusText = TXT_OPERATOR_SUSPENDED;
      break;
    case SYSSTATUS::Running:
      windowCaption += " - " TXT_OPERATOR_RUNNING " " + timeElapsed.FormatString( "nn:ss" ) + " s";
      statusText = TXT_OPERATOR_RUNNING;
      break;
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
      configEnabled = true;
      setConfigEnabled = true;
      runSystemEnabled = true;
      quitEnabled = true;
      break;
    case SYSSTATUS::Suspended:
      runSystemCaption = "Resume";
      configEnabled = true;
      setConfigEnabled = true;
      runSystemEnabled = true;
      quitEnabled = true;
      break;
    case SYSSTATUS::Running:
      runSystemCaption = "Suspend";
      runSystemEnabled = true;
      break;
  }
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
    if( tcpsocket::wait_for_read( mParent.mSockets, socketTimeout, true ) )
      TThread::Synchronize( mParent.ProcessBCIMessages );
}

//--------------------- Handlers for BCI messages ------------------------------

bool
TfMain::HandleSTATUS( istream& is )
{
  STATUS status;
  if( status.ReadBinary( is ) )
  {
    mSysstatus.Status[ Origin( is ) ] = status.GetStatus();
    
    // If we receive a warning message, add a line to the system log and bring it to front.
    if( ( status.GetCode() >= 300 ) && ( status.GetCode() < 400 ) )
    {
      mSyslog.AddSysLogEntry( status.GetStatus(), SYSLOGENTRYMODE_WARNING );
      mSyslog.ShowSysLog();
    }
    // If we receive an error message, add a line to the system log and bring it to front.
    else if( ( status.GetCode() >= 400 ) && ( status.GetCode() < 500 ) )
    {
      mSyslog.AddSysLogEntry( status.GetStatus(), SYSLOGENTRYMODE_ERROR );
      mSyslog.ShowSysLog();
    }

    // If the operator received successful status messages from
    // all core modules, then this is the end of the initialization phase.
    if( ( mSysstatus.SystemState == SYSSTATUS::Initialization ) && ( status.GetCode() < 300 ) )
    {
      mSysstatus.INI[ Origin( is ) ] = true;
      switch( Origin( is ) )
      {
        case EEGSource:
          mSyslog.AddSysLogEntry( "Source confirmed new parameters ..." );
          break;
        case SigProc:
          mSyslog.AddSysLogEntry( "Signal Processing confirmed new parameters ..." );
          break;
        case App:
          mSyslog.AddSysLogEntry( "User Application confirmed new parameters ..." );
          break;
      }
    }
    if( mSysstatus.INI[ EEGSource ] && mSysstatus.INI[ SigProc ] && mSysstatus.INI[ App ]
        && mSysstatus.SystemState == SYSSTATUS::Initialization )
    {
      EnterState( SYSSTATUS::Resting );
    }
  }
  return true;
}

bool
TfMain::HandleSYSCMD( istream& is )
{
  SYSCMD syscmd;
  if( syscmd.ReadBinary( is ) )
  {
    if( syscmd == SYSCMD::Reset )
    {
      Terminate();
    }
    else if( syscmd == SYSCMD::Suspend )
    {
      EnterState( SYSSTATUS::Suspended );
    }
    // If we received 'EndOfParameter', i.e., all parameters arrived,
    // then let's sort the parameters in the parameter list by name.
    else if( syscmd == SYSCMD::EndOfParameter )
    {
      mParameters.Sort();
    }
    // The operator receiving 'EndOfState' marks the end of the publishing phase.
    else if( syscmd == SYSCMD::EndOfState )
    {
      mSysstatus.EOS[ Origin( is ) ] = true;
    }
    // If we received EndOfStates from all the modules, then make the transition
    // to the next system state.
    if( mSysstatus.EOS[ EEGSource ] && mSysstatus.EOS[ SigProc ] && mSysstatus.EOS[ App ]
        && mSysstatus.SystemState == SYSSTATUS::Publishing )
    {
      EnterState( SYSSTATUS::Information );
    }
  }
  return true;
}

bool
TfMain::HandlePARAM( istream& is )
{
  PARAM param;
  if( param.ReadBinary( is ) )
  {
    ++mSysstatus.NumParametersRecv[ Origin( is ) ];
    mParameters.CloneParameter2List( &param );
    // Update the parameter in the configuration window.
    fConfig->RenderParameter( mParameters.GetParamPtr( param.GetName() ) );
  }
  return true;
}

bool
TfMain::HandleSTATE( istream& is )
{
  STATE state;
  if( state.ReadBinary( is ) )
  {
    ++mSysstatus.NumStatesRecv[ Origin( is ) ];
    mStates.AddState2List( &state );
    EnterState( SYSSTATUS::Publishing );
  }
  return true;
}

bool
TfMain::HandleVisSignal( istream& is )
{
  VisSignal visSignal;
  if( visSignal.ReadBinary( is ) )
  {
    ++mSysstatus.NumDataRecv[ Origin( is ) ];
    VISUAL::HandleMessage( visSignal );
  }
  return true;
}

bool
TfMain::HandleVisCfg( istream& is )
{
  VisCfg visCfg;
  if( visCfg.ReadBinary( is ) )
  {
    ++mSysstatus.NumDataRecv[ Origin( is ) ];
    VISUAL::HandleMessage( visCfg );
  }
  return true;
}

bool
TfMain::HandleVisMemo( istream& is )
{
  VisMemo visMemo;
  if( visMemo.ReadBinary( is ) )
  {
    ++mSysstatus.NumDataRecv[ Origin( is ) ];
    VISUAL::HandleMessage( visMemo );
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
  UpdateState( "Running", mSysstatus.SystemState != SYSSTATUS::Running );
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bConfigClick( TObject* )
{
  fConfig->Initialize( &mParameters, &mPreferences );
  fConfig->Show();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bSetConfigClick( TObject* )
{
  if( fConfig->Visible )
    fConfig->Close();

  switch( mSysstatus.SystemState )
  {
    case SYSSTATUS::Information:
    case SYSSTATUS::Initialization:
      EnterState( SYSSTATUS::Initialization );
      break;
    case SYSSTATUS::Resting:
    case SYSSTATUS::Suspended:
      EnterState( SYSSTATUS::Resting );
      break;
  }
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

void __fastcall TfMain::About1Click( TObject* )
{
  fAbout->ShowModal();
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

void __fastcall TfMain::View1Click(TObject *Sender)
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



