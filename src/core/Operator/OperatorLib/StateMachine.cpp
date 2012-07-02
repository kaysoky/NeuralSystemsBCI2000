////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that represents the operator module's state machine.
//   The state machine is responsible for
//   - maintaining a global BCI2000 system state (state of operation),
//   - maintaining global parameter and state lists,
//   - handling core module connections,
//   - maintaining visualization object properties,
//   - processing state change requests,
//   - triggering callback events to display text messages, or to visualize data.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "StateMachine.h"

#include "BCI_OperatorLib.h"
#include "BCIError.h"
#include "BCIException.h"
#include "BCIAssert.h"
#include "ProtocolVersion.h"
#include "Status.h"
#include "SysCommand.h"
#include "StateVector.h"
#include "FileUtils.h"
#include "SignalProperties.h"
#include "GenericVisualization.h"
#include "Label.h"
#include "ScriptInterpreter.h"
#include "EnvVariable.h"
#include "BCIDirectory.h"
#include "ParamRef.h"

#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <cstdlib>

using namespace std;

StateMachine::StateMachine()
: mSystemState( Idle ),
  mIntroducedRandomSeed( false ),
  mEventLink( *this )
{
  Reset();
  
  string path;
  EnvVariable::Get( "PATH", path );
  path = FileUtils::InstallationDirectoryS() + FileUtils::PathSeparator + path;
  EnvVariable::Set( "PATH", path );
  EnvVariable::Set( "BCI2000LAUNCHDIR", FileUtils::InstallationDirectoryS() );
  EnvVariable::Set( "BCI2000BINARY", FileUtils::ExecutablePath() );
}

bool
StateMachine::Startup( const char* inArguments )
{
  bool result = ( mSystemState == Idle );
  {
    OSMutex::Lock lock( mDataMutex );
    if( result )
    {
      mIntroducedRandomSeed = false;
      string arguments = inArguments ? inArguments : "";
      if( arguments.empty() )
        arguments = "*";
      istringstream argstream( arguments );
      string ip;
      argstream >> ip;
      string modules;
      getline( argstream, modules, '\0' );
      if( modules.empty() )
        modules = "Source:4000 SignalProcessing:4001 Application:4002";
      istringstream iss( modules );
      bool sourcePort = true;
      while( !iss.eof() )
      {
        string name;
        std::getline( iss >> ws, name, ':' );
        string port;
        iss >> port >> ws;
        string address = ip + ":" + port;
        mConnections.push_back( new CoreConnection( *this, name, address, static_cast<int>( mConnections.size() + 1 ) ) );
        if( sourcePort )
        {
          int iPort;
          istringstream( port ) >> iPort;
          mEventLink.Open( iPort );
          sourcePort = false;
        }
      }
      result &= ( bcierr__.Flushes() == 0 );
      if( !result )
        CloseConnections();
    }
    if( result )
    {
      mpSourceModule = *mConnections.begin();

      const VersionInfo& info = VersionInfo::Current;
      mParameters.Add(
        "System:Configuration matrix OperatorVersion= { Framework Revision Build } 1 Operator % %"
        " % % % // operator module version information" );
      mParameters["OperatorVersion"].Value( "Framework" )
        = info[VersionInfo::VersionID];
      if( info[VersionInfo::Revision].empty() )
      {
        mParameters["OperatorVersion"].Value( "Revision" )
          = info[VersionInfo::SourceDate];
      }
      else
      {
        mParameters["OperatorVersion"].Value( "Revision" )
          = info[VersionInfo::Revision] + ", " +  info[VersionInfo::SourceDate];
      }
      mParameters["OperatorVersion"].Value( "Build" )
        = info[VersionInfo::BuildDate];

      mParameters.Add(
        "System:Additional%20Connections int OperatorBackLink= 1"
        " 1 0 1 // Send final state and signal information to Operator (boolean)" );

      OSThread::Start();
      result &= ( bcierr__.Flushes() == 0 );
    }
    bcierr__.Clear();
  }
  if( result )
    EnterState( WaitingForConnection );
  return result;
}


StateMachine::~StateMachine()
{
  // The StateMachine destructor must be called from the same (main) thread that
  // called its constructor.
  {
    ::Lock<Listeners> lock( mListeners );
    for( Listeners::iterator i = mListeners.begin(); i != mListeners.end(); ++i )
      ( *i )->Abort();
  }
  SharedPointer<OSEvent> pEvent = OSThread::Terminate();
  while( CheckPendingCallback() ) ;
  while( mSystemState != Idle && mSystemState != Fatal )
  {
    const int cReactionTime = 50;
    OSThread::Sleep( cReactionTime );
    while( CheckPendingCallback() ) ;
  }
  pEvent->Wait();
}

// Send a state message containing a certain state value to the EEG source module.
// This function is public because it is called from the CommandInterpreter class.
bool
StateMachine::SetStateValue( const char* inName, State::ValueType inValue )
{
  // We call EnterState() from here to have a consistent behavior if
  // UpdateState() is called for "Running" from a script or a button.
  if( !::stricmp( inName, "Running" ) )
  {
    switch( mSystemState )
    {
      case Resting:
      case Suspended:
        if( inValue )
          EnterState( RunningInitiated );
        break;
      case Running:
        if( !inValue )
          EnterState( SuspendInitiated );
        break;
      default:
        return false;
    }
  }

  OSMutex::Lock lock( mDataMutex );
  if( !mStates.Exists( inName ) )
    return false;
  else
  {
    class State& s = mStates[ inName ];
    s.SetValue( inValue );
    if( !mpSourceModule || !mpSourceModule->PutMessage( s ) )
      return false;
  }
  return true;
}

State::ValueType
StateMachine::GetStateValue( const char* inName ) const
{
  OSMutex::Lock lock( mDataMutex );
  if( !mStates.Exists( inName ) )
    return 0;
  return mStateVector.StateValue( inName );
}

bool
StateMachine::SetEvent( const char* inName, State::ValueType inValue )
{
  if( !mEvents.Exists( inName ) )
    return false;
  if( mSystemState != StateMachine::Running )
    return false;
  ::Lock<EventLink> lock( mEventLink );
  if( !mEventLink.Connected() )
    return false;
  mEventLink << inName << ' ' << inValue << " 0" << endl;
  return true;
}

void
StateMachine::MaintainDebugLog()
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

bool
StateMachine::SetConfig()
{
  bool result = false;
  switch( mSystemState )
  {
    case Information:
    case Initialization:
    case Resting:
    case RestingParamsModified:
    case Suspended:
    case SuspendedParamsModified:
      result = true;
      EnterState( SetConfigIssued );
      break;

    default:
      ;
  }
  return result;
}

bool
StateMachine::StartRun()
{
  bool result = false;
  switch( mSystemState )
  {
    case Resting:
    case Suspended:
      result = SetStateValue( "Running", true );
      break;

    default:
      ;
  }
  return result;
}

bool
StateMachine::StopRun()
{
  bool result = false;
  switch( mSystemState )
  {
    case Running:
      result = SetStateValue( "Running", false );
      break;

    default:
      ;
  }
  return result;
}

// Terminate the state machine thread, which will shut down connections.
bool
StateMachine::Shutdown()
{
  bool result = ( mSystemState != Idle );
  if( result )
  {
    StopRun();
    OSThread::Terminate();
  }
  return result;
}

bool
StateMachine::Reset()
{
  bool result = ( mSystemState == Idle );
  if( result )
  {
    mParameters.Clear();
    mStates.Clear();
    mEvents.Clear();
    mIntroducedRandomSeed = false;
    mPreviousRandomSeed.clear();
    mStateVector = StateVector();
    mControlSignal = GenericSignal();
    mVisualizations.clear();
  }
  return result;
};

void
StateMachine::ParameterChange()
{
  switch( mSystemState )
  {
    case Resting:
      EnterState( RestingParamsModified );
      break;

    case Suspended:
      EnterState( SuspendedParamsModified );
      break;

    default:
      ;
  }
}

StateMachine::ConnectionInfo
StateMachine::Info( size_t i ) const
{
  if( i < mConnections.size() )
    return mConnections[i]->Info();
  return ConnectionInfo();
}

void
StateMachine::BroadcastParameters()
{
  int numParams = mParameters.Size();
  for( ConnectionList::iterator i = mConnections.begin(); i != mConnections.end(); ++i )
    if( ( *i )->PutMessage( mParameters ) )
      OSThread::Sleep( 0 );
}

void
StateMachine::BroadcastEndOfParameter()
{
  for( ConnectionList::iterator i = mConnections.begin(); i != mConnections.end(); ++i )
    ( *i )->PutMessage( SysCommand::EndOfParameter );
}

void
StateMachine::BroadcastStates()
{
  int numStates = mStates.Size();
  for( ConnectionList::iterator i = mConnections.begin(); i != mConnections.end(); ++i )
    if( ( *i )->PutMessage( mStates ) )
      OSThread::Sleep( 0 );
}

void
StateMachine::BroadcastEndOfState()
{
  for( ConnectionList::iterator i = mConnections.begin(); i != mConnections.end(); ++i )
    ( *i )->PutMessage( SysCommand::EndOfState );
}

void
StateMachine::InitializeStateVector()
{
  mParameters.Add(
    "System:State%20Vector"
    " int StateVectorLength= 0 16 % % "
    "// length of the state vector in bytes" );
  mStates.AssignPositions();
  mStateVector = StateVector( mStates );
  ostringstream length;
  length << mStateVector.Length();
  mParameters["StateVectorLength"].Value() = length.str().c_str();
}

void
StateMachine::InitializeModules()
{
  mpSourceModule->PutMessage( SignalProperties( 0, 0 ) );
}

// Here we list allowed state transitions, and associated actions.
#define TRANSITION( a, b )  ( ( int( a ) << 8 ) | int( b ) )
void
StateMachine::EnterState( SysState inState )
{
  if( mSystemState != Fatal || inState == Fatal )
  {
    int transition = TRANSITION( mSystemState, inState ),
        prevState;
    {
      OSMutex::Lock lock( mStateMutex );
      prevState = mSystemState;
      mSystemState = Transition;
      PerformTransition( transition );
      mSystemState = inState;
    }
    ExecuteTransitionCallbacks( transition );
    ExecuteCallback( BCI_OnSystemStateChange );
  }
}

void
StateMachine::PerformTransition( int inTransition )
{
  OSMutex::Lock lock( mDataMutex );
  switch( inTransition )
  {
    case TRANSITION( Idle, WaitingForConnection ):
    case TRANSITION( WaitingForConnection, Publishing ):
    case TRANSITION( Publishing, Publishing ):
      break;

    case TRANSITION( Publishing, Information ):
      Randomize();
      mEventLink.ConfirmConnection();
      break;

    case TRANSITION( Information, SetConfigIssued ):
      MaintainDebugLog();
      InitializeStateVector();
      BroadcastParameters();
      BroadcastEndOfParameter();
      BroadcastStates();
      BroadcastEndOfState();
      InitializeModules();
      break;

    case TRANSITION( Initialization, SetConfigIssued ):
    case TRANSITION( Resting, SetConfigIssued ):
    case TRANSITION( Suspended, SetConfigIssued ):
    case TRANSITION( SuspendedParamsModified, SetConfigIssued ):
    case TRANSITION( RestingParamsModified, SetConfigIssued ):
      MaintainDebugLog();
      BroadcastParameters();
      BroadcastEndOfParameter();
      InitializeModules();
      break;

    case TRANSITION( SetConfigIssued, Resting ):
    case TRANSITION( SetConfigIssued, Initialization ):
      break;

    case TRANSITION( Resting, RunningInitiated ):
    case TRANSITION( Suspended, RunningInitiated ):
      RandomizationWarning();
      MaintainDebugLog();
      break;

    case TRANSITION( RunningInitiated, Running ):
    case TRANSITION( Running, SuspendInitiated ):
    case TRANSITION( SuspendInitiated, SuspendInitiated ):
      break;

    case TRANSITION( SuspendInitiated, Suspended ):
      Randomize();
      BroadcastParameters(); // no EndOfParameter
      break;

    case TRANSITION( Suspended, SuspendedParamsModified ):
    case TRANSITION( Resting, RestingParamsModified ):
      break;

    case TRANSITION( WaitingForConnection, Idle ):
    case TRANSITION( Publishing, Idle ):
    case TRANSITION( Information, Idle ):
    case TRANSITION( Initialization, Idle ):
    case TRANSITION( SetConfigIssued, Idle ):
    case TRANSITION( Resting, Idle ):
    case TRANSITION( RestingParamsModified, Idle ):
    case TRANSITION( RunningInitiated, Idle ):
    case TRANSITION( Running, Idle ):
    case TRANSITION( SuspendInitiated, Idle ):
    case TRANSITION( Suspended, Idle ):
    case TRANSITION( SuspendedParamsModified, Idle ):
      // Send a system command 'Reset' to the EEGsource.
      mpSourceModule->PutMessage( SysCommand::Reset );
      mDebugLog.close();
      CloseConnections();
      break;

    case TRANSITION( Idle, Fatal ):
    case TRANSITION( WaitingForConnection, Fatal ):
    case TRANSITION( Publishing, Fatal ):
    case TRANSITION( Information, Fatal ):
    case TRANSITION( Initialization, Fatal ):
    case TRANSITION( SetConfigIssued, Fatal ):
    case TRANSITION( Resting, Fatal ):
    case TRANSITION( RestingParamsModified, Fatal ):
    case TRANSITION( RunningInitiated, Fatal ):
    case TRANSITION( Running, Fatal ):
    case TRANSITION( SuspendInitiated, Fatal ):
    case TRANSITION( Suspended, Fatal ):
    case TRANSITION( SuspendedParamsModified, Fatal ):
    case TRANSITION( Transition, Fatal ):
    case TRANSITION( Fatal, Fatal ):
      break;

    default:
      bcierr << "Unexpected system state transition: "
             << ( inTransition >> 8 ) << " -> " << ( inTransition & 0xff )
             << endl;
  }
}

void
StateMachine::ExecuteTransitionCallbacks( int inTransition )
{
  switch( inTransition )
  {
    case TRANSITION( Idle, WaitingForConnection ):
      LogMessage( BCI_OnLogMessage, "BCI2000 Started" );
      break;

    case TRANSITION( Publishing, Information ):
      TriggerEvent( BCI_OnConnect );
      break;

    case TRANSITION( Information, SetConfigIssued ):
    case TRANSITION( Initialization, SetConfigIssued ):
    case TRANSITION( Resting, SetConfigIssued ):
    case TRANSITION( Suspended, SetConfigIssued ):
    case TRANSITION( SuspendedParamsModified, SetConfigIssued ):
    case TRANSITION( RestingParamsModified, SetConfigIssued ):
      LogMessage( BCI_OnLogMessage, "Operator set configuration" );
      break;

    case TRANSITION( SetConfigIssued, Resting ):
      TriggerEvent( BCI_OnSetConfig );
      break;

    case TRANSITION( Resting, RunningInitiated ):
      TriggerEvent( BCI_OnStart );
      LogMessage( BCI_OnLogMessage, "Operator started operation" );
      break;

    case TRANSITION( Suspended, RunningInitiated ):
      TriggerEvent( BCI_OnResume );
      LogMessage( BCI_OnLogMessage, "Operator resumed operation" );
      break;

    case TRANSITION( Running, SuspendInitiated ):
      LogMessage( BCI_OnLogMessage, "Operation suspended" );
      break;

    case TRANSITION( SuspendInitiated, Suspended ):
      TriggerEvent( BCI_OnSuspend );
      break;

    case TRANSITION( WaitingForConnection, Idle ):
    case TRANSITION( Publishing, Idle ):
    case TRANSITION( Information, Idle ):
    case TRANSITION( Initialization, Idle ):
    case TRANSITION( SetConfigIssued, Idle ):
    case TRANSITION( Resting, Idle ):
    case TRANSITION( RestingParamsModified, Idle ):
    case TRANSITION( RunningInitiated, Idle ):
    case TRANSITION( Running, Idle ):
    case TRANSITION( SuspendInitiated, Idle ):
    case TRANSITION( Suspended, Idle ):
    case TRANSITION( SuspendedParamsModified, Idle ):
      TriggerEvent( BCI_OnShutdown );
      LogMessage( BCI_OnLogMessage, "Operator shut down connections" );
      break;
  }
  ExecuteCallback( BCI_OnSystemStateChange );
}

int
StateMachine::OnExecute()
{ // The state machine's main message loop.
  while( !OSThread::IsTerminating() )
  {
    if( streamsock::wait_for_read( mSockets, ConnectionTimeout, true ) )
    {
      for( ConnectionList::iterator i = mConnections.begin(); i != mConnections.end(); ++i )
        ( *i )->ProcessBCIMessages();
      if( !OSThread::IsTerminating() )
        ExecuteCallback( BCI_OnCoreInput );
    }
  }
  EnterState( Idle );
  return 0;
}

bool
StateMachine::CheckInitializeVis( const string& inSourceID, const string& inKind )
{
  bool kindDiffers = ( mVisualizations[ inSourceID ].Kind() != inKind );
  if( kindDiffers )
  {
    mVisualizations[ inSourceID ].Kind() = inKind;
    ExecuteCallback( BCI_OnInitializeVis, inSourceID.c_str(), inKind.c_str() );
  }
  return kindDiffers;
}

void
StateMachine::LogMessage( int inCallbackID, const string& inMessage )
{
  string separator;
  switch( inCallbackID )
  {
    case BCI_OnDebugMessage:
    case BCI_OnLogMessage:
      separator = ": ";
      break;
    case BCI_OnWarningMessage:
      separator = ": Warning: ";
      break;
    case BCI_OnErrorMessage:
      separator = ": Error: ";
      break;
    default:
      throw bciexception( "Unknown log message callback ID: " << inCallbackID );
  }
  ExecuteCallback( inCallbackID, inMessage.c_str() );
  time_t t = ::time( NULL );
  mDebugLog << ::ctime( &t ) << separator << inMessage << endl;
  ::Lock<Listeners> lock( mListeners );
  for( Listeners::iterator i = mListeners.begin(); i != mListeners.end(); ++i )
    ( *i )->HandleLogMessage( inCallbackID, inMessage );
}

void
StateMachine::TriggerEvent( int inCallbackID )
{
  const char* pName = ScriptEvents::Name( inCallbackID );
  if( !pName )
    throw bciexception( "Unknown operator event callback ID: " << inCallbackID );

  ExecuteCallback( inCallbackID );
  string script = mScriptEvents.Get( inCallbackID );
  if( !script.empty() )
  {
    LogMessage( BCI_OnLogMessage, "Executing " + string( pName ) + " script ..." );
    ScriptInterpreter( *this ).ExecuteAsynchronously( script );
  }
}

void
StateMachine::Randomize()
{
  // Add a RandomSeed parameter if it's not present already.
  Param p1(
    "System:Randomization"
    " int RandomSeed= 0 0 % % "
    " // seed for the BCI2000 pseudo random number generator" );
  if( mParameters.Exists( "RandomSeed" ) )
    p1.Value() = mParameters["RandomSeed"].Value();
  else
    mIntroducedRandomSeed = true;
  mParameters.Add( p1 );
  Param p2(
    "System:Randomization"
    " int RandomizationWarning= 1 1 0 1 "
    " // warn if subsequent Runs have identical RandomSeed value (boolean)" );
  if( mParameters.Exists( "RandomizationWarning" ) )
    p2.Value() = mParameters["RandomizationWarning"].Value();
  mParameters.Add( p2 );
  
  if( mIntroducedRandomSeed )
  {
    ::srand( static_cast<unsigned int>( ::time( NULL ) ) );
    for( int n = 0; n < ::rand() % 10; ++n )
      ::rand(); // MSVC 2008 initial random number tends to be proportional to seed (!)
    int number = 0;
    while( number == 0 || number == ::atoi( mPreviousRandomSeed.c_str() ) )
      number = ::rand();
    ostringstream oss;
    oss << number;
    mParameters["RandomSeed"].Value() = oss.str();
  }
}

void
StateMachine::RandomizationWarning()
{
  if( !mIntroducedRandomSeed
      && ::atoi( mParameters["RandomizationWarning"].Value().c_str() )
      && mParameters["RandomSeed"].Value().c_str() == mPreviousRandomSeed
      && ::atoi( mPreviousRandomSeed.c_str() ) )
    bciout__ << "In the present configuration, the RandomSeed value does not "
             << "automatically change between runs. "
             << "Any 'random' behavior, such as randomization of the order of trials, "
             << "or the generation of noise signals, will be exactly the same "
             << "on this run as on the previous run."
             << endl;
   
  mPreviousRandomSeed = mParameters["RandomSeed"].Value().c_str();
}


// ------------------------ CoreConnection definitions -------------------------

StateMachine::CoreConnection::CoreConnection( StateMachine& inParent, 
                                              const std::string& inName,
                                              const std::string& inAddress,
                                              int inTag )
: mrParent( inParent ),
  mAddress( inAddress ),
  mTag( inTag ),
  mConnected( false )
{
  for( int i = 0; i < NumConfirmations; ++i )
    mConfirmed[i] = false;

  {
    OSMutex::Lock lock( mInfoMutex );
    mInfo.Name = inName;
  }
  mrParent.mSockets.insert( &mSocket );
  mSocket.set_tcpnodelay( true );
  const int timeout = 1000, // ms
            resolution = 20;
  int timeElapsed = 0;
  while( timeElapsed < timeout && !mSocket.is_open() )
  {
    mSocket.open( mAddress.c_str() );
    if( !mSocket.is_open() )
    {
      timeElapsed += resolution;
      OSThread::Sleep( resolution );
    }
  }
  if( !mSocket.is_open() )
    bcierr__ << "Operator: Could not open socket for listening on "
             << mAddress
             << endl;
}


StateMachine::CoreConnection::~CoreConnection()
{
  mrParent.mSockets.erase( &mSocket );
}


void
StateMachine::CoreConnection::ProcessBCIMessages()
{
  if( mSocket.connected() && !mConnected )
  {
    OSMutex::Lock lock( mrParent.mBCIMessageMutex );
    OnAccept();
  }
  else if( !mSocket.connected() && mConnected )
  {
    OSMutex::Lock lock( mrParent.mBCIMessageMutex );
    OnDisconnect();
  }
  mConnected = mSocket.connected();
  while( mStream && mStream.rdbuf()->in_avail() && !mrParent.IsTerminating() )
  {
    OSMutex::Lock lock( mrParent.mBCIMessageMutex ); // Serialize messages
    HandleMessage( mStream );
    {
      OSMutex::Lock lock( mInfoMutex );
      ++mInfo.MessagesRecv;
    }
    if( !( mStream && mStream.is_open() ) || ( bcierr__.Flushes() > 0 ) )
      mrParent.EnterState( Fatal );
  }
}

//----------------------- Connection bookkeeping -------------------------------

// When a connection is accepted by an open socket, open the associated stream,
// and enter the appropriate information into its ConnectionInfo::Address[] entry.
void
StateMachine::CoreConnection::OnAccept()
{
  mStream.clear();
  mStream.open( mSocket );
  if( mStream.is_open() )
  {
    ostringstream oss;
    oss << mSocket.ip() << ":" << mSocket.port();
    OSMutex::Lock lock( mInfoMutex );
    mInfo.Address = oss.str();
  }
}

// When a connection is closed, close the associated stream, and update
// the information in its ConnectionInfo::Address[] entry.
void
StateMachine::CoreConnection::OnDisconnect()
{
  mStream.close();
  mSocket.open( mAddress.c_str() );
  OSMutex::Lock lock( mInfoMutex );
  mInfo.Address = "";
}


//--------------------- Handlers for BCI messages ------------------------------
bool
StateMachine::CoreConnection::HandleProtocolVersion( istream& is )
{
  ProtocolVersion version;
  if( version.ReadBinary( is ) )
  {
    OSMutex::Lock lock( mInfoMutex );
    mInfo.Version = version;
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleStatus( istream& is )
{
  Status status;
  if( status.ReadBinary( is ) )
  {
    {
      OSMutex::Lock lock( mInfoMutex );
      mInfo.Status = status.Message().c_str();
    }
    switch( status.Content() )
    {
      case Status::debug:
        mrParent.LogMessage( BCI_OnDebugMessage, status.Message() );
        break;
      case Status::warning:
        mrParent.LogMessage( BCI_OnWarningMessage, status.Message() );
        break;
      case Status::error:
        mrParent.LogMessage( BCI_OnErrorMessage, status.Message() );
        break;
      case Status::initialized:
      { // If the operator received successful status messages from
        // all core modules, then this is the end of the initialization phase.
        string message;
        {
          OSMutex::Lock lock( mInfoMutex );
          message = mInfo.Name + " confirmed new parameters ...";
        }
        mrParent.LogMessage( BCI_OnLogMessage, message );
      } break;
      case Status::running:
        Confirm( ConfirmRunning );
        break;
      case Status::suspended:
        Confirm( ConfirmSuspended );
        break;
      default:
        mrParent.LogMessage( BCI_OnLogMessage, status.Message() );
    }

    switch( mrParent.SystemState() )
    {
      case Publishing:
        if( mrParent.Confirmed( ConfirmEndOfStates ) )
        {
          mrParent.ClearConfirmation( ConfirmEndOfStates );
          mrParent.EnterState( StateMachine::Information );
        }
        break;

      case SetConfigIssued:
        switch( status.Content() )
        {
          case Status::error:
            mrParent.ClearConfirmation( ConfirmInitialized );
            mrParent.EnterState( StateMachine::Initialization );
            break;

          case Status::initialized:
            Confirm( ConfirmInitialized );
            if( mrParent.Confirmed( ConfirmInitialized ) )
            {
              mrParent.ClearConfirmation( ConfirmInitialized );
              mrParent.EnterState( StateMachine::Resting );
            }
            break;
        }
        break;

      case SuspendInitiated:
        if( mrParent.Confirmed( ConfirmSuspended ) )
        {
          mrParent.ClearConfirmation( ConfirmSuspended );
          mrParent.EnterState( StateMachine::Suspended );
        }
        break;

      case RunningInitiated:
        if( mrParent.Confirmed( ConfirmRunning ) )
        {
          mrParent.ClearConfirmation( ConfirmRunning );
          mrParent.EnterState( StateMachine::Running );
        }
        break;
    }

    if( status.Content() == Status::error && mrParent.SystemState() != Initialization )
      mrParent.EnterState( Fatal );
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleSysCommand( istream& is )
{
  SysCommand syscmd;
  if( syscmd.ReadBinary( is ) )
  {
    if( syscmd == SysCommand::Reset )
    {
      mrParent.Shutdown();
    }
    else if( syscmd == SysCommand::Suspend )
    {
      mrParent.EnterState( SuspendInitiated );
    }
    else if( syscmd == SysCommand::EndOfParameter )
    {
      /* do nothing */
    }
    // The operator receiving 'EndOfState' marks the end of the publishing phase.
    else if( syscmd == SysCommand::EndOfState )
    {
      Confirm( ConfirmEndOfStates );
      OSMutex::Lock lock( mInfoMutex );
      if( !mInfo.Version.Matches( ProtocolVersion::Current() ) )
      {
        string older = mInfo.Name,
               newer = "Operator";
        if( mInfo.Version.MoreRecentThan( ProtocolVersion::Current() ) )
          swap( older, newer );

        bcierr__ << "Protocol version mismatch between Operator and "
                 << mInfo.Name << " module. \n"
                 << "The " << newer << " module appears to be more recent than the "
                 << older << " module. \n\n"
                 << "Please make sure that all modules share the same"
                 << " BCI2000 major version number. \n"
                 << "The operator module's version number is "
                 << mrParent.mParameters[ "OperatorVersion" ].Value( "Framework" )
                 << ". \n\n"
                 << "BCI2000 will now quit."
                 << endl;
        mrParent.Shutdown();
      }
    }
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleParam( istream& is )
{
  Param param;
  if( param.ReadBinary( is ) )
  {
    ostringstream oss;
    {
      OSMutex::Lock lock( mrParent.mDataMutex );
      mrParent.mParameters.Add( param, mTag );
      mrParent.mParameters[param.Name()].WriteToStream( oss );
      mrParent.ParameterChange();
    }
    mrParent.ExecuteCallback( BCI_OnParameter, oss.str().c_str() );
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleState( istream& is )
{
  class State state;
  if( state.ReadBinary( is ) )
  {
    ostringstream oss;
    {
      OSMutex::Lock lock( mrParent.mDataMutex );
      mrParent.mStates.Delete( state.Name() );
      mrParent.mStates.Add( state );
      state.WriteToStream( oss );
    }
    mrParent.ExecuteCallback( BCI_OnState, oss.str().c_str() );
    mrParent.EnterState( Publishing );
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleStateVector( istream& is )
{
  OSMutex::Lock lock( mrParent.mDataMutex );
  return mrParent.mStateVector.ReadBinary( is );
}

bool
StateMachine::CoreConnection::HandleVisSignal( istream& is )
{
  VisSignal v;
  if( v.ReadBinary( is ) )
  {
    if( v.SourceID().empty() )
    {
      OSMutex::Lock lock( mrParent.mDataMutex );
      mrParent.mControlSignal = v;
    }
    else
    {
      const string kind = "Graph";
      mrParent.CheckInitializeVis( v.SourceID(), kind );
      int channels = v.Signal().Channels(),
          elements = v.Signal().Elements(),
          size = channels * elements;
      bciassert( size >= 0 );
      float* pData = new float[ channels * elements ];
      for( int ch = 0; ch < channels; ++ch )
        for( int el = 0; el < elements; ++el )
          pData[ ch * elements + el ] = static_cast<float>( v.Signal()( ch, el ) );
      mrParent.ExecuteCallback( BCI_OnVisSignal, v.SourceID().c_str(), channels, elements, pData );
      delete[] pData;
    }
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleVisSignalProperties( istream& is )
{
  VisSignalProperties v;
  if( v.ReadBinary( is ) )
  {
    OSMutex::Lock lock( mrParent.mDataMutex );
    if( v.SourceID().empty() )
    {
      mrParent.mControlSignal.SetProperties( v );
    }
    else
    {
      // We treat a VisSignalProperties message as a set of VisCfg
      // messages.
#ifdef TODO
# error Factor this out into a VisSignalProperties::ToVisCfg() method.
#endif // TODO
      const SignalProperties& s = v.SignalProperties();
      const char* sourceID = v.SourceID().c_str();
      VisProperties& p = mrParent.mVisualizations[sourceID];
      if( !s.Name().empty() )
      {
        p.Put( CfgID::WindowTitle, s.Name() );
        mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, sourceID, CfgID::WindowTitle, s.Name().c_str() );
      }

      string channelUnit = s.ChannelUnit().RawToPhysical( s.ChannelUnit().Offset() + 1 );
      p.Put( CfgID::ChannelUnit, channelUnit );
      mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, sourceID, CfgID::ChannelUnit, channelUnit.c_str() );

      int numSamples = static_cast<int>( s.ElementUnit().RawMax() - s.ElementUnit().RawMin() + 1 );
      p.Put( CfgID::NumSamples, numSamples );
      ostringstream oss;
      oss << numSamples;
      mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, sourceID, CfgID::NumSamples, oss.str().c_str() );

      if( numSamples > 0 )
      {
        string symbol;
        double value = s.ElementUnit().Gain() * numSamples;
        int magnitude = static_cast<int>( ::log10( ::fabs( value ) ) );
        if( magnitude > 0 )
          --magnitude;
        else if( magnitude < 0 )
          ++magnitude;
        magnitude /= 3;
        if( magnitude < -3 )
          magnitude = -3;
        if( magnitude > 3 )
          magnitude = 3;
        switch( magnitude )
        {
          case -3:
            symbol = "n";
            value /= 1e-9;
            break;
          case -2:
            symbol = "mu";
            value /= 1e-6;
            break;
          case -1:
            symbol = "m";
            value /= 1e-3;
            break;
          case 0:
            break;
          case 1:
            symbol = "k";
            value /= 1e3;
            break;
          case 2:
            symbol = "M";
            value /= 1e6;
            break;
          case 3:
            symbol = "G";
            value /= 1e9;
            break;
        }
        ostringstream oss;
        oss << setprecision( 10 ) << value / numSamples << symbol << s.ElementUnit().Symbol();
        p.Put( CfgID::SampleUnit, oss.str() );
        mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, sourceID, CfgID::SampleUnit, oss.str().c_str() );
        p.Put( CfgID::SampleOffset, s.ElementUnit().Offset() );
        oss.str( "" );
        oss << s.ElementUnit().Offset();
        mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, sourceID, CfgID::SampleOffset, oss.str().c_str() );
      }

      // Although the SignalProperties class allows for individual units for
      // individual channels, the SignalDisplay class is restricted to a single
      // unit and range.
      string valueUnit = s.ValueUnit().RawToPhysical( s.ValueUnit().Offset() + 1 );
      p.Put( CfgID::ValueUnit, valueUnit );
      mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, sourceID, CfgID::ValueUnit, valueUnit.c_str() );

      double rangeMin = s.ValueUnit().RawMin(),
             rangeMax = s.ValueUnit().RawMax();
      if( rangeMin == rangeMax )
      {
        p.erase( CfgID::MinValue );
        p.erase( CfgID::MaxValue );
      }
      else
      {
        p.Put( CfgID::MinValue, rangeMin );
        oss.str( "" );
        oss << rangeMin;
        mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, sourceID, CfgID::MinValue, oss.str().c_str() );
        p.Put( CfgID::MaxValue, rangeMax );
        oss.str( "" );
        oss << rangeMax;
        mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, sourceID, CfgID::MaxValue, oss.str().c_str() );
      }

      LabelList groupLabels,
                channelLabels;
      int channelGroupSize = 1;
      if( !s.ChannelLabels().IsTrivial() )
      {
        for( int i = 0; i < s.ChannelLabels().Size(); ++i )
        {
          istringstream iss( s.ChannelLabels()[ i ] );
          HierarchicalLabel label;
          iss >> label;
          if( label.size() == 2 )
          {
            if( groupLabels.empty() )
            {
              groupLabels.push_back( Label( 0, label[ 0 ] ) );
            }
            else
            {
              if( label[ 0 ] == groupLabels.begin()->Text() )
                ++channelGroupSize;
              if( label[ 0 ] != groupLabels.rbegin()->Text() )
                groupLabels.push_back( Label( static_cast<int>( groupLabels.size() ), label[ 0 ] ) );
            }
            channelLabels.push_back( Label( static_cast<int>( channelLabels.size() ), label[ 1 ] ) );
          }
          else
          {
            channelLabels.push_back( Label( i, s.ChannelLabels()[ i ] ) );
          }
        }
      }
      p.Put( CfgID::ChannelGroupSize, channelGroupSize );
      oss.str( "" );
      oss << channelGroupSize;
      mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, sourceID, CfgID::ChannelGroupSize, oss.str().c_str() );

      p.Put( CfgID::ChannelLabels, channelLabels );
      oss.str( "" );
      oss << channelLabels;
      mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, sourceID, CfgID::ChannelLabels, oss.str().c_str() );

      p.Put( CfgID::GroupLabels, groupLabels );
      oss.str( "" );
      oss << groupLabels;
      mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, sourceID, CfgID::GroupLabels, oss.str().c_str() );
    }
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleVisMemo( istream& is )
{
  VisMemo v;
  if( v.ReadBinary( is ) )
  {
    const string kind = "Memo";
    mrParent.CheckInitializeVis( v.SourceID(), kind );
    mrParent.ExecuteCallback( BCI_OnVisMemo, v.SourceID().c_str(), v.MemoText().c_str() );
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleVisBitmap( istream& is )
{
  VisBitmap v;
  if( v.ReadBinary( is ) )
  {
    const string kind = "Bitmap";
    mrParent.CheckInitializeVis( v.SourceID(), kind );
    const BitmapImage& b = v.BitmapImage();
    mrParent.ExecuteCallback( BCI_OnVisBitmap, v.SourceID().c_str(), b.Width(), b.Height(), b.RawData() );
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleVisCfg( istream& is )
{
  VisCfg v;
  if( v.ReadBinary( is ) )
  {
    OSMutex::Lock lock( mrParent.mDataMutex );
    mrParent.mVisualizations[v.SourceID()][v.CfgID()] = v.CfgValue();
    mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, v.SourceID().c_str(), v.CfgID(), v.CfgValue().c_str() );
  }
  return true;
}

// ---------- EventLink definitions ------------
void
StateMachine::EventLink::ConfirmConnection()
{
  TerminateWait();
  bool shouldBeConnected = mrParent.Parameters().Exists( "EventLink" );
  if( shouldBeConnected )
    shouldBeConnected &= ( ::atoi( mrParent.Parameters()["EventLink"].Value().c_str() ) != 0 );
  if( shouldBeConnected && !mConnected )
    bcierr << "EventLink: Could not establish connection to Source module";
}

int
StateMachine::EventLink::OnExecute()
{
  const int cReactionTimeMs = 100;
  receiving_udpsocket serverSocket;
  serverSocket.open( "localhost", mPort );
  if( !serverSocket.is_open() )
  {
    bcierr << "EventLink: Could not open UDP port " << mPort << " for listening" << endl;
  }
  else
  {
    while( !serverSocket.wait_for_read( cReactionTimeMs ) && !IsTerminating() )
      ;
    serverSocket.close();
    if( !IsTerminating() )
    {
      ::Lock<EventLink> lock1( *this );
      istringstream iss( mrParent.mpSourceModule->Info().Address );
      string sourceIP;
      std::getline( iss, sourceIP, ':' );
      mSocket.open( sourceIP.c_str(), mPort + 1 );
      this->clear();
      this->open( mSocket );
      ::Lock<StateMachine> lock2( mrParent );
      StateList& events = mrParent.Events();
      for( int i = 0; i < events.Size(); ++i )
        *this << events[i] << endl;
      *this << endl;
      mConnected = true;
    }
  }
  return 0;
}

