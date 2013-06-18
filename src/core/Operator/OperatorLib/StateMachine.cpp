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
#include "BCIStream.h"
#include "BCIException.h"
#include "BCIAssert.h"
#include "ProtocolVersion.h"
#include "Status.h"
#include "SysCommand.h"
#include "StateVector.h"
#include "FileUtils.h"
#include "RunManager.h"
#include "SignalProperties.h"
#include "GenericVisualization.h"
#include "Label.h"
#include "ScriptInterpreter.h"
#include "EnvVariable.h"
#include "ParamRef.h"
#include "WildcardMatch.h"

#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <cstdlib>

using namespace std;

StateMachine::StateMachine()
: mSystemState( Idle ),
  mEventLink( *this )
{
  Init();

  string path;
  EnvVariable::Get( "PATH", path );
  path = FileUtils::InstallationDirectoryS() + FileUtils::PathSeparator + path;
  EnvVariable::Set( "PATH", path );
  EnvVariable::Set( "BCI2000LAUNCHDIR", FileUtils::InstallationDirectoryS() );
  EnvVariable::Set( "BCI2000BINARY", FileUtils::ExecutablePath() );
}

void
StateMachine::Init()
{
  CloseConnections();
  WatchDataLock lock( this );
  mParameters.Clear();
  mStates.Clear();
  mEvents.Clear();
  mIntroducedRandomSeed = false;
  mPreviousRandomSeed.clear();
  mStateVector = StateVector();
  mControlSignal = GenericSignal();
  mVisualizations.clear();
}

bool
StateMachine::Startup( const char* inArguments )
{
  if( SystemState() != Idle )
    return false;

  DataLock lock( this );
  mSystemState = Transition;

  const VersionInfo& info = VersionInfo::Current;
  mParameters.Add(
    "System:Configuration matrix OperatorVersion= { Framework Revision Build Config } 1 Operator % %"
    " % % % // operator module version information" );
  Param& p = mParameters["OperatorVersion"];
  p.Value( "Framework" ) = info[VersionInfo::VersionID];
  p.Value( "Revision" ) = info[VersionInfo::Revision] + ", " +  info[VersionInfo::SourceDate];
  p.Value( "Build" ) = info[VersionInfo::Build];
  p.Value( "Config" ) = info[VersionInfo::Config];

  mParameters.Add(
    "System:Protocol int OperatorBackLink= 1"
    " 1 0 1 // Send final state and signal information to Operator (boolean)" );

  mParameters.Add(
    "System:Protocol int AutoConfig= 1"
    " 1 0 1 // Use AutoConfig protocol extension (boolean)" );

  mIntroducedRandomSeed = false;
  mLocalAddress.clear();
  mpSourceModule = 0;

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
  while( !iss.eof() )
  {
    string name;
    std::getline( iss >> ws, name, ':' );
    string port;
    iss >> port >> ws;
    string address = ip + ":" + port;
    CoreConnection* p = new CoreConnection( *this, name, address, static_cast<int>( mConnections.size() + 1 ) );
    mConnections.push_back( p );
    mSockets.insert( p->Socket() );
    if( mLocalAddress.empty() )
    {
      mpSourceModule = p;
      mLocalAddress = "localhost:" + port;
      mEventLink.Open( ::atoi( port.c_str() ) );
    }
  }
  mSystemState = Idle;
  if( bcierr__.Empty() && IsConsistentState( WaitingForConnection ) )
    EnterState( WaitingForConnection );
  bool success = bcierr__.Empty() && SystemState() == WaitingForConnection;
  if( success )
    OSThread::Start();
  else
    CloseConnections();
  bcierr__.Clear();
  return success;
}

void
StateMachine::CloseConnections()
{
  mLocalAddress = "";
  mpSourceModule = NULL;
  for( size_t i = 0; i < mConnections.size(); ++i )
  {
    mSockets.erase( mConnections[i]->Socket() );
    delete mConnections[i];
  }
  mConnections.clear();
  mEventLink.Close();
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

// Send a state message containing a certain state value to the SignalSource module.
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
      case Resting | ParamsModified:
      case Suspended | ParamsModified:
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

  WatchDataLock lock( this );
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
  DataLock lock( this );
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
    string filePath = RunManager::CurrentSession( mParameters ) + ".dbg",
           dir = FileUtils::ExtractDirectory( filePath );
    FileUtils::MakeDirectory( dir, true );
    mDebugLog.close();
    mDebugLog.clear();
    mDebugLog.open( filePath.c_str(), ios_base::out | ios_base::app );
    if( !mDebugLog.is_open() )
      bcierr << "Cannot write debug log: " << filePath;
  }
  else
    mDebugLog.close();
}

void
StateMachine::DebugWarning()
{
  const string suffix = "Version",
               row = "Build",
               pattern = "*\\<debug\\>*",
               sep = ", ";
  string debugList;
  for( int i = 0; i < mParameters.Size(); ++i )
  {
    const Param& p = mParameters[i];
    ptrdiff_t pos = p.Name().length() - suffix.length();
    if( pos > 0 && p.Name().substr( pos ) == suffix )
    {
      string module = p.Name().substr( 0, pos );
      if( p.RowLabels().Exists( row ) && bci::WildcardMatch( pattern, p.Value( row, 0 ), false ) )
        debugList += sep + module;
    }
  }
  if( !debugList.empty() )
    bciwarn__ << "The following modules were built in debug mode: "
              << debugList.substr( sep.length() ) << ".\n"
              << "Debug mode is useful for development, but affects perfomance"
              << " -- do not use debug builds for experiments!\n";
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
    case Suspended:
    case Resting | ParamsModified:
    case Suspended | ParamsModified:
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
    case Resting | ParamsModified:
    case Suspended | ParamsModified:
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
    Init();
  return result;
};

void
StateMachine::ParameterChange()
{
  switch( mSystemState )
  {
    case Resting:
    case Suspended:
      mSystemState |= ParamsModified;
      break;
  }
}

StateMachine::ConnectionInfo
StateMachine::Info( size_t i ) const
{
  if( i < mConnections.size() )
  {
    const CoreConnection* p = mConnections[i];
    return p->Info()();
  }
  return ConnectionInfo();
}

void
StateMachine::BroadcastParameters()
{
  for( size_t i = 0; i < mConnections.size(); ++i )
    if( mConnections[i]->PutMessage( mParameters ) )
      ThreadUtils::Yield();
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
      ThreadUtils::Yield();
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
  mAutoParameters.Clear();
  mpSourceModule->PutMessage( SignalProperties( 0, 0 ) );
}

// Here we list allowed state transitions, and associated actions.
#define TRANSITION( a, b )  ( ( int( a ) << 8 ) | int( b ) )
void
StateMachine::EnterState( SysState inState )
{
  int curState = mSystemState & ~StateFlags;
  if( curState != Fatal || inState == Fatal )
  {
    int transition = TRANSITION( curState, inState );
    {
      OSMutex::Lock lock( mStateMutex );
      mSystemState = Transition;
      PerformTransition( transition );
      switch( inState )
      {
        case Initialization:
        case SetConfigIssued:
        case RunningInitiated:
          SetConnectionState( inState );
          break;
      }
      mSystemState = inState;
    }
    ExecuteTransitionCallbacks( transition );
  }
}

void
StateMachine::PerformTransition( int inTransition )
{
  DataLock lock( this );
  switch( inTransition )
  {
    case TRANSITION( Idle, Idle ):
    case TRANSITION( Idle, WaitingForConnection ):
    case TRANSITION( WaitingForConnection, Publishing ):
    case TRANSITION( Publishing, Publishing ):
      break;

    case TRANSITION( Publishing, Information ):
      DebugWarning();
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

    case TRANSITION( WaitingForConnection, Idle ):
    case TRANSITION( Publishing, Idle ):
    case TRANSITION( Information, Idle ):
    case TRANSITION( Initialization, Idle ):
    case TRANSITION( SetConfigIssued, Idle ):
    case TRANSITION( Resting, Idle ):
    case TRANSITION( RunningInitiated, Idle ):
    case TRANSITION( Running, Idle ):
    case TRANSITION( SuspendInitiated, Idle ):
    case TRANSITION( Suspended, Idle ):
      // Send a system command 'Reset' to the SignalSource module.
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
    case TRANSITION( RunningInitiated, Fatal ):
    case TRANSITION( Running, Fatal ):
    case TRANSITION( SuspendInitiated, Fatal ):
    case TRANSITION( Suspended, Fatal ):
    case TRANSITION( Transition, Fatal ):
    case TRANSITION( Fatal, Fatal ):
      break;

    default:
      bcidebug(
        "Unexpected system state transition: " 
         << ( inTransition >> 8 ) << " -> " << ( inTransition & 0xff )
      );
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
    case TRANSITION( RunningInitiated, Idle ):
    case TRANSITION( Running, Idle ):
    case TRANSITION( SuspendInitiated, Idle ):
    case TRANSITION( Suspended, Idle ):
      TriggerEvent( BCI_OnShutdown );
      LogMessage( BCI_OnLogMessage, "Operator shut down connections" );
      break;
  }
  ExecuteCallback( BCI_OnSystemStateChange );
  CheckWatches();
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

string
StateMachine::SuggestUDPAddress( const string& inAddressHint ) const
{
  sending_udpsocket socket( LocalAddress().c_str() );
  string result,
         local = socket.address(),
         local2,
         hint = inAddressHint.empty() ? local : inAddressHint;
  socket.open( socket.ip(), socket.port() + 1 );
  local2 = socket.address();  

  string ip = "";
  uint16_t port = 0;
  istringstream iss( hint );
  if( getline( iss, ip, ':' ) >> port )
  {
    int maxPort = port + 250;
    for( ; port < maxPort && result.empty(); ++port )
    {
      socket.open( ip.c_str(), port );
      if( socket.is_open() )
      {
        string a = socket.address();
        bool ok = a != local
                && a != local2
                && Watches().SelectByAddress( a ).Empty();
        if( ok )
          result = a;
      }
    }
  }
  return result;
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
      throw std_logic_error( "Unknown log message callback ID: " << inCallbackID );
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
    throw std_logic_error( "Unknown operator event callback ID: " << inCallbackID );

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

void
StateMachine::Handle( const CoreConnection&, const ProtocolVersion& )
{
}

void
StateMachine::Handle( const CoreConnection& c, const Status& s )
{
  int kind = 0;
  switch( s.Content() )
  {
    case Status::debug:
      kind = BCI_OnDebugMessage;
      break;
    case Status::warning:
      kind = BCI_OnWarningMessage;
      break;
    case Status::error:
      kind = BCI_OnErrorMessage;
      break;
    default:
      kind = BCI_OnLogMessage;
  }
  string name = c.Info()().Name,
         msg = s.Message();
  if( kind == BCI_OnLogMessage )
  {
    if( !::stricmp( name.c_str(), msg.substr( 0, name.length() ).c_str() ) )
      msg = msg.substr( name.length() );
    if( !msg.empty() )
    {
      if( !::isspace( *msg.begin() ) )
        msg = " " + msg;
      string::iterator i = msg.begin();
      while( ::isspace( *i ) && i != msg.end() )
        ++i;
      if( i != msg.end() )
        *i = ::toupper( *i );
      msg = ":" + msg;
    }
    msg = name + msg;
  }
  else if( mConnections.size() > 1 )
  {
    msg = "[" + name + "] " + msg;
  }
  LogMessage( kind, msg );

  if( s.Content() == Status::error )
  {
    switch( SystemState() )
    {
      case Initialization:
        break;
      case SetConfigIssued:
        EnterState( Initialization );
        break;
      default:
        EnterState( Fatal );
    }
  }
}

void
StateMachine::Handle( const CoreConnection& c, const SysCommand& s )
{
  if( s == SysCommand::Reset )
  {
    Shutdown();
  }
  else if( s == SysCommand::Suspend )
  {
    if( SystemState() == Running )
      EnterState( SuspendInitiated );
  }
  else if( s == SysCommand::EndOfParameter )
  {
    /* do nothing */
  }
  else if( s == SysCommand::EndOfState )
  {
    ConnectionInfo info = c.Info()();
    if( !info.Version.Matches( ProtocolVersion::Current() ) )
    {
      string older = info.Name,
             newer = "Operator";
      if( info.Version.MoreRecentThan( ProtocolVersion::Current() ) )
        swap( older, newer );

      bcierr__ << "Protocol version mismatch between Operator and "
               << info.Name << " module. \n"
               << "The " << newer << " module appears to be more recent than the "
               << older << " module. \n\n"
               << "Please make sure that all modules share the same"
               << " BCI2000 major version number. \n"
               << "The operator module's version number is "
               << mParameters[ "OperatorVersion" ].Value( "Framework" )
               << ". \n\n"
               << "BCI2000 will now quit."
               << endl;
      Shutdown();
    }
  }
}

void
StateMachine::Handle( const CoreConnection& inConnection, const Param& inParam )
{
  switch( SystemState() & ~StateFlags )
  {
    case SetConfigIssued:
      mAutoParameters[inParam.Name()] = inParam;
      break;
    
    default:
    { ostringstream oss;
      {
        DataLock lock( this );
        mParameters.Add( inParam, static_cast<int>( inConnection.Tag() ) );
        mParameters[inParam.Name()].WriteToStream( oss );
        ParameterChange();
      }
      ExecuteCallback( BCI_OnParameter, oss.str().c_str() );
    } break;
  }
}

void
StateMachine::Handle( const CoreConnection&, const State& inState )
{
  ostringstream oss;
  {
    DataLock lock( this );
    mStates.Delete( inState.Name() );
    mStates.Add( inState );
    inState.WriteToStream( oss );
  }
  ExecuteCallback( BCI_OnState, oss.str().c_str() );
}

bool
StateMachine::HandleStateVector( const CoreConnection&, istream& is )
{
  WatchDataLock lock( this );
  return mStateVector.ReadBinary( is );
}

void
StateMachine::Handle( const CoreConnection&, const VisSignal& inSignal )
{
  if( inSignal.SourceID().empty() )
  {
    WatchDataLock lock( this );
    mControlSignal = inSignal;
  }
  else
  {
    const string kind = "Graph";
    CheckInitializeVis( inSignal.SourceID(), kind );
    int channels = inSignal.Signal().Channels(),
        elements = inSignal.Signal().Elements(),
        size = channels * elements;
    bciassert( size >= 0 );
    float* pData = new float[ channels * elements ];
    for( int ch = 0; ch < channels; ++ch )
      for( int el = 0; el < elements; ++el )
        pData[ ch * elements + el ] = static_cast<float>( inSignal.Signal()( ch, el ) );
    ExecuteCallback( BCI_OnVisSignal, inSignal.SourceID().c_str(), channels, elements, pData );
    delete[] pData;
  }
}

void
StateMachine::Handle( const CoreConnection& c, const VisSignalProperties& v )
{
  DataLock lock( this );
  if( v.SourceID().empty() )
  {
    if( ( SystemState() & ~StateFlags ) == SetConfigIssued )
    {
      size_t idx = c.Tag();
      for( size_t i = idx; i < mConnections.size(); ++i )
        if( mConnections[i]->PutMessage( mAutoParameters ) )
          ThreadUtils::Yield();
      if( idx < mConnections.size() )
        mConnections[idx]->PutMessage( v );
      else
        mControlSignal.SetProperties( v );
    }
  }
  else
  {
    vector<VisCfg> visCfg = v.ToVisCfg();
    for( size_t i = 0; i < visCfg.size(); ++i )
      Handle( c, visCfg[i] );
  }
}

void
StateMachine::Handle( const CoreConnection&, const VisMemo& v )
{
  const string kind = "Memo";
  CheckInitializeVis( v.SourceID(), kind );
  ExecuteCallback( BCI_OnVisMemo, v.SourceID().c_str(), v.MemoText().c_str() );
}

void
StateMachine::Handle( const CoreConnection&, const VisBitmap& v )
{
  const string kind = "Bitmap";
  CheckInitializeVis( v.SourceID(), kind );
  const BitmapImage& b = v.BitmapImage();
  ExecuteCallback( BCI_OnVisBitmap, v.SourceID().c_str(), b.Width(), b.Height(), b.RawData() );
}

void
StateMachine::Handle( const CoreConnection&, const VisCfg& v )
{
  DataLock lock( this );
  mVisualizations[v.SourceID()][v.CfgID()] = v.CfgValue();
  ExecuteCallback( BCI_OnVisPropertyMessage, v.SourceID().c_str(), v.CfgID(), v.CfgValue().c_str() );
}

void
StateMachine::Handle( const CoreConnection& inConnection, SysState inState )
{
  int systemState = SystemState() & ~StateFlags;
  if( systemState == Transition || systemState == inState )
    return;

  switch( inState )
  {
    case Publishing:
      EnterState( Publishing );
      break;
    default:
      if( IsConsistentState( inState ) )
        EnterState( inState );
  }
}

bool
StateMachine::IsConsistentState( SysState s ) const
{
  for( size_t i = 0; i < mConnections.size(); ++i )
    if( mConnections[i]->State() != s )
      return false;
  return true;
}

void
StateMachine::SetConnectionState( SysState s )
{
  for( size_t i = 0; i < mConnections.size(); ++i )
    mConnections[i]->EnterState( s );
}

// ------------------------ CoreConnection definitions -------------------------

StateMachine::CoreConnection::CoreConnection( StateMachine& inParent,
                                              const std::string& inName,
                                              const std::string& inAddress,
                                              ptrdiff_t inTag )
: mrParent( inParent ),
  mAddress( inAddress ),
  mTag( inTag ),
  mState_( Idle )
{
  Info()().Name = inName;
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
  if( mSocket.is_open() )
    EnterState( WaitingForConnection );
  else
    bcierr__ << "Operator: Could not open socket for listening on " << mAddress;
}

void
StateMachine::CoreConnection::ProcessBCIMessages()
{
  if( mSocket.connected() && State() == WaitingForConnection )
    OnAccept();
  else if( !mSocket.connected() && State() != WaitingForConnection )
    OnDisconnect();
  while( mStream && mStream.rdbuf()->in_avail() && !mrParent.IsTerminating() )
  {
    OSMutex::Lock lock( mrParent.mBCIMessageMutex ); // Serialize messages
    HandleMessage( mStream );
    ++Info()().MessagesRecv;
    if( !( mStream && mStream.is_open() ) )
      EnterState( Fatal );
  }
}

void
StateMachine::CoreConnection::EnterState( SysState inState )
{
  mState_ = inState;
  mrParent.Handle( *this, inState );
}

//----------------------- Connection bookkeeping -------------------------------

// When a connection is accepted by an open socket, open the associated stream,
// and enter the appropriate information into its ConnectionInfo::Address[] entry.
void
StateMachine::CoreConnection::OnAccept()
{
  {
    OSMutex::Lock lock( mrParent.mBCIMessageMutex );
    mStream.clear();
    mStream.open( mSocket );
    if( mStream.is_open() )
    {
      ostringstream oss;
      oss << mSocket.ip() << ":" << mSocket.port();
      Info()().Address = oss.str();
      EnterState( Publishing );
    }
  }
}

// When a connection is closed, close the associated stream, and update
// the information in its ConnectionInfo::Address[] entry.
void
StateMachine::CoreConnection::OnDisconnect()
{ 
  {
    OSMutex::Lock lock( mrParent.mBCIMessageMutex );
    mStream.close();
    Info()().Address = "";
  }
  EnterState( Idle );
}

//--------------------- Handlers for BCI messages ------------------------------
bool
StateMachine::CoreConnection::HandleProtocolVersion( istream& is )
{
  ProtocolVersion version;
  if( version.ReadBinary( is ) )
    Info()().Version = version;
  return true;
}

bool
StateMachine::CoreConnection::HandleStatus( istream& is )
{
  Status status;
  if( status.ReadBinary( is ) )
  {
    Info()().Status = status.Message().c_str();
    mrParent.Handle( *this, status );
    switch( status.Content() )
    {
      case Status::initialized:
        EnterState( Resting );
        break;
      case Status::running:
        EnterState( Running );
        break;
      case Status::suspended:
        EnterState( Suspended );
        break;
    }
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleSysCommand( istream& is )
{
  SysCommand syscmd;
  if( syscmd.ReadBinary( is ) )
  { // Receiving 'EndOfState' marks the end of the publishing phase.
    if( syscmd == SysCommand::EndOfState )
      EnterState( Information );
    mrParent.Handle( *this, syscmd );
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleParam( istream& is )
{
  Param param;
  if( param.ReadBinary( is ) )
    mrParent.Handle( *this, param );
  return true;
}

bool
StateMachine::CoreConnection::HandleState( istream& is )
{
  class State state;
  if( state.ReadBinary( is ) )
    mrParent.Handle( *this, state );
  return true;
}

bool
StateMachine::CoreConnection::HandleStateVector( istream& is )
{
  return mrParent.HandleStateVector( *this, is );
}

bool
StateMachine::CoreConnection::HandleVisSignal( istream& is )
{
  VisSignal v;
  if( v.ReadBinary( is ) )
    mrParent.Handle( *this, v );
  return true;
}

bool
StateMachine::CoreConnection::HandleVisSignalProperties( istream& is )
{
  VisSignalProperties v;
  if( v.ReadBinary( is ) )
    mrParent.Handle( *this, v );
  return true;
}

bool
StateMachine::CoreConnection::HandleVisMemo( istream& is )
{
  VisMemo v;
  if( v.ReadBinary( is ) )
    mrParent.Handle( *this, v );
  return true;
}

bool
StateMachine::CoreConnection::HandleVisBitmap( istream& is )
{
  VisBitmap v;
  if( v.ReadBinary( is ) )
    mrParent.Handle( *this, v );
  return true;
}

bool
StateMachine::CoreConnection::HandleVisCfg( istream& is )
{
  VisCfg v;
  if( v.ReadBinary( is ) )
    mrParent.Handle( *this, v );
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
  int timeout = 2000;
  while( !serverSocket.is_open() && timeout > 0 )
  {
    serverSocket.open( "localhost", mPort );
    if( !serverSocket.is_open() )
    {
      ThreadUtils::SleepFor( cReactionTimeMs );
      timeout -= cReactionTimeMs;
    }
  }
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
      const CoreConnection* p = *mrParent.mConnections.begin();
      istringstream iss( p->Info()().Address );
      string sourceIP;
      std::getline( iss, sourceIP, ':' );
      ::Lock<EventLink> lock1( *this );
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

