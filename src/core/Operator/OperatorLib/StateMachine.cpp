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
#include "PCHIncludes.h"
#pragma hdrstop

#include "StateMachine.h"

#include "BCI_OperatorLib.h"
#include "BCIError.h"
#include "ProtocolVersion.h"
#include "Status.h"
#include "SysCommand.h"
#include "StateVector.h"
#include "Version.h"
#include "BCIDirectory.h"
#include "SignalProperties.h"
#include "GenericVisualization.h"
#include "Label.h"

#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>

using namespace std;

StateMachine::StateMachine()
: CallbackBase( &mDataMutex ),
  OSThread(),
  mSystemState( Idle ),
  mpDataLock( NULL )
{
}


bool
StateMachine::Startup( const char* inModuleList )
{
  OSMutex::Lock lock( mDataMutex );

  const char* moduleList = inModuleList;
  if( moduleList == NULL )
    moduleList = "Core Module:4000";
  istringstream iss( moduleList );
  while( !iss.eof() )
  {
    string name;
    getline( iss >> ws, name, ':' );
    int port;
    iss >> port;
    mConnections.push_back( new CoreConnection( *this, name, port, mConnections.size() + 1 ) );
  }
  mpSourceModule = *mConnections.begin();

  iss.clear();
  iss.str( BCI2000_VERSION );
  iss >> mVersionInfo;
  mParameters.Add(
    "System:Configuration matrix OperatorVersion= { Framework Revision Build } 1 Operator % %"
    " % % % // operator module version information" );
  mParameters[ "OperatorVersion" ].Value( "Framework" )
    = mVersionInfo[ VersionInfo::VersionID ];
  if( mVersionInfo[ VersionInfo::Revision ].empty() )
  {
    mParameters[ "OperatorVersion" ].Value( "Revision" )
      = mVersionInfo[ VersionInfo::SourceDate ];
  }
  else
  {
    mParameters[ "OperatorVersion" ].Value( "Revision" )
      = mVersionInfo[ VersionInfo::Revision ] + ", " +  mVersionInfo[ VersionInfo::SourceDate ];
  }
  mParameters[ "OperatorVersion" ].Value( "Build" )
    = mVersionInfo[ VersionInfo::BuildDate ];

  OSThread::Start();
  bool result = ( bcierr__.Flushes() == 0 );
  if( result )
    ExecuteCallback( BCI_OnLogMessage, "BCI2000 Started" );
  else
    EnterState( StateMachine::Fatal );
  return result;
}


// Initiate program termination, and perform de-initialization.
bool
StateMachine::Shutdown()
{
  OSMutex::Lock lock( mDataMutex );
  if( !OSThread::IsTerminating() )
  {
    // Send a system command 'Reset' to the EEGsource.
    mpSourceModule->PutMessage( SysCommand::Reset );
    OSThread::Terminate();
    mDebugLog.close();
    ExecuteCallback( BCI_OnShutdown );
  }
  return true;
}


StateMachine::~StateMachine()
{
  Shutdown();
  while( !OSThread::IsTerminated() )
    OSThread::Sleep( ConnectionTimeout / 2 );
  DeleteConnections();
}

// Send a state message containing a certain state value to the EEG source module.
// This function is public because it is called from the SCRIPT class.
bool
StateMachine::SetStateValue( const char* inName, long inValue )
{
  OSMutex::Lock lock( mDataMutex );
  // We call EnterState() from here to have a consistent behavior if
  // UpdateState() is called for "Running" from a script or a button.
  if( string( "Running" ) == inName )
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

  if( !mStates.Exists( inName ) )
    return false;
  else
  {
    class State& s = mStates[ inName ];
    s.SetValue( inValue );
    if( !mpSourceModule->PutMessage( s ) )
      return false;
  }
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
  OSMutex::Lock lock( mDataMutex );
  bool result = false;
  switch( SystemState() )
  {
    case Information:
    case Initialization:
    case Resting:
    case RestingParamsModified:
    case Suspended:
    case SuspendedParamsModified:
      EnterState( Initialization );
      result = true;
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
  switch( SystemState() )
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
  switch( SystemState() )
  {
    case Running:
      result = SetStateValue( "Running", false );
      break;

    default:
      ;
  }
  return result;
}

void
StateMachine::ParameterChange()
{
  switch( SystemState() )
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
    {
      OSThread::Sleep( 0 );
      OSMutex::Lock lock( ( *i )->mInfoMutex );
      ( *i )->mInfo.ParametersSent += numParams;
      ( *i )->mInfo.MessagesSent += numParams - 1;
    }
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
    {
      OSThread::Sleep( 0 );
      OSMutex::Lock lock( ( *i )->mInfoMutex );
      ( *i )->mInfo.MessagesSent += numStates;
      ( *i )->mInfo.StatesSent += numStates;
    }
}

void
StateMachine::BroadcastEndOfState()
{
  for( ConnectionList::iterator i = mConnections.begin(); i != mConnections.end(); ++i )
    ( *i )->PutMessage( SysCommand::EndOfState );
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
  int transition = TRANSITION( mSystemState, inState ),
      prevState;
  {
    OSMutex::Lock lock( mStateMutex );
    prevState = mSystemState;
    mSystemState = inState;
  }
  switch( transition )
  {
    case TRANSITION( Idle, Publishing ):
      break;

    case TRANSITION( Publishing, Publishing ):
      break;

    case TRANSITION( Publishing, Information ):
      // Add the state vector's length to the system parameters.
      {
        OSMutex::Lock lock( mDataMutex );
        mParameters.Add(
          "System:State%20Vector"
          " int StateVectorLength= 0 16 1 30 "
          "// length of the state vector in bytes" );
        mStates.AssignPositions();
        ostringstream length;
        length << StateVector( mStates ).Length();
        mParameters[ "StateVectorLength" ].Value() = length.str().c_str();
      }
      ExecuteCallback( BCI_OnConnect );
      break;

    case TRANSITION( Information, Information ):
      break;

    case TRANSITION( Information, Initialization ):
      {
        OSMutex::Lock lock( mDataMutex );
        MaintainDebugLog();
        BroadcastParameters();
        BroadcastEndOfParameter();
        BroadcastStates();
        BroadcastEndOfState();
        InitializeModules();
      }
      ExecuteCallback( BCI_OnLogMessage, "Operator set configuration" );
      break;

    case TRANSITION( Initialization, Resting ):
      ExecuteCallback( BCI_OnSetConfig );
      break;

    case TRANSITION( Initialization, Initialization ):
    case TRANSITION( Resting, Initialization ):
    case TRANSITION( Suspended, Initialization ):
    case TRANSITION( SuspendedParamsModified, Initialization ):
    case TRANSITION( RestingParamsModified, Initialization ):
      {
        OSMutex::Lock lock( mDataMutex );
        MaintainDebugLog();
        BroadcastParameters();
        BroadcastEndOfParameter();
        InitializeModules();
      }
      ExecuteCallback( BCI_OnLogMessage, "Operator set configuration" );
      break;

    case TRANSITION( Resting, RunningInitiated ):
      {
        OSMutex::Lock lock( mDataMutex );
        MaintainDebugLog();
      }
      ExecuteCallback( BCI_OnStart );
      ExecuteCallback( BCI_OnLogMessage, "Operator started operation" );
      break;

    case TRANSITION( Suspended, RunningInitiated ):
      {
        OSMutex::Lock lock( mDataMutex );
        MaintainDebugLog();
      }
      ExecuteCallback( BCI_OnResume );
      ExecuteCallback( BCI_OnLogMessage, "Operator resumed operation" );
      break;

    case TRANSITION( RunningInitiated, Running ):
      break;

    case TRANSITION( Running, SuspendInitiated ):
      ExecuteCallback( BCI_OnLogMessage, "Operation suspended" );
      break;

    case TRANSITION( SuspendInitiated, SuspendInitiated ):
      break;

    case TRANSITION( SuspendInitiated, Suspended ):
      {
        OSMutex::Lock lock( mDataMutex );
        BroadcastParameters(); // no EndOfParameter
      }
      ExecuteCallback( BCI_OnSuspend );
      break;

    case TRANSITION( Suspended, SuspendedParamsModified ):
    case TRANSITION( Resting, RestingParamsModified ):
      break;

    case TRANSITION( Idle, Fatal ):
    case TRANSITION( Publishing, Fatal ):
    case TRANSITION( Information, Fatal ):
    case TRANSITION( Initialization, Fatal ):
    case TRANSITION( Resting, Fatal ):
    case TRANSITION( RestingParamsModified, Fatal ):
    case TRANSITION( RunningInitiated, Fatal ):
    case TRANSITION( Running, Fatal ):
    case TRANSITION( SuspendInitiated, Fatal ):
    case TRANSITION( Suspended, Fatal ):
    case TRANSITION( SuspendedParamsModified, Fatal ):
    case TRANSITION( Fatal, Fatal ):
      break;

    default:
      bcierr << "Unexpected system state transition: "
             << prevState << " -> " << inState
             << endl;
  }
  ExecuteCallback( BCI_OnSystemStateChange );
}

int
StateMachine::Execute()
{ // The state machine's main message loop.
  while( !OSThread::IsTerminating() )
    if( streamsock::wait_for_read( mSockets, ConnectionTimeout, true ) )
    {
      for( ConnectionList::iterator i = mConnections.begin(); i != mConnections.end(); ++i )
        ( *i )->ProcessBCIMessages();
      if( !OSThread::IsTerminating() )
        ExecuteCallback( BCI_OnCoreInput );
    }
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

// ------------------------ CoreConnection definitions -------------------------

StateMachine::CoreConnection::CoreConnection( StateMachine& inParent, const std::string& inName, short inPort, int inTag )
: mrParent( inParent ),
  mPort( inPort ),
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
  mSocket.open( "*", mPort );
  if( !mSocket.is_open() )
    bcierr__ << "Operator: Could not open socket for listening on port "
             << mPort
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
    OSMutex::Lock lock( mrParent.mDataMutex );
    OnAccept();
  }
  else if( !mSocket.connected() && mConnected )
  {
    OSMutex::Lock lock( mrParent.mDataMutex );
    OnDisconnect();
  }
  mConnected = mSocket.connected();
  while( mStream && mStream.rdbuf()->in_avail() && !mrParent.IsTerminating() )
  {
    OSMutex::Lock lock( mrParent.mDataMutex );
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
  mSocket.open( "*", mPort );
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
    time_t t = ::time( NULL );
    switch( status.Content() )
    {
      case Status::debug:
      {
        mrParent.ExecuteCallback( BCI_OnDebugMessage, status.Message().c_str() );
        mrParent.mDebugLog << ::ctime( &t ) << ": "
                           << status.Message()
                           << endl;
        break;
      }
      case Status::warning:
        mrParent.ExecuteCallback( BCI_OnWarningMessage, status.Message().c_str() );
        mrParent.mDebugLog << ::ctime( &t ) 
                           << ": Warning: "
                           << status.Message()
                           << endl;
        break;
      case Status::error:
        mrParent.ExecuteCallback( BCI_OnErrorMessage, status.Message().c_str() );
        mrParent.mDebugLog << ::ctime( &t )
                           << ": Error: "
                           << status.Message()
                           << endl;
        break;
      case Status::initialized:
      { // If the operator received successful status messages from
        // all core modules, then this is the end of the initialization phase.
        Confirm( ConfirmInitialized );
        string message;
        {
          OSMutex::Lock lock( mInfoMutex );
          message = mInfo.Name + " confirmed new parameters ...";
        }
        mrParent.ExecuteCallback( BCI_OnLogMessage, message.c_str() );
        mrParent.mDebugLog << ::ctime( &t )
                           << message
                           << endl;
      } break;
      case Status::running:
        Confirm( ConfirmRunning );
        break;
      case Status::suspended:
        Confirm( ConfirmSuspended );
        break;
      default:
        mrParent.ExecuteCallback( BCI_OnLogMessage, status.Message().c_str() );
        mrParent.mDebugLog << ::ctime( &t )
                           << status.Message()
                           << endl;
    }

    if( mrParent.Confirmed( ConfirmInitialized ) && mrParent.SystemState() == Initialization )
    {
      mrParent.ClearConfirmation( ConfirmInitialized );
      mrParent.EnterState( StateMachine::Resting );
    }

    if( mrParent.Confirmed( ConfirmSuspended ) && mrParent.SystemState() == SuspendInitiated )
    {
      mrParent.ClearConfirmation( ConfirmSuspended );
      mrParent.EnterState( StateMachine::Suspended );
    }

    if( mrParent.Confirmed( ConfirmRunning ) && mrParent.SystemState() == RunningInitiated )
    {
      mrParent.ClearConfirmation( ConfirmRunning );
      mrParent.EnterState( StateMachine::Running );
    }
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
    // If we received EndOfStates from all the modules, then make the transition
    // to the next system state.
    if( mrParent.Confirmed( ConfirmEndOfStates ) && mrParent.SystemState() == Publishing )
    {
      mrParent.EnterState( StateMachine::Information );
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
    {
      OSMutex::Lock lock( mInfoMutex );
      ++mInfo.ParametersRecv;
    }
    mrParent.mParameters.Add( param, mTag );
    ostringstream oss;
    mrParent.mParameters[param.Name()].WriteToStream( oss );
    mrParent.ParameterChange();
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
    {
      OSMutex::Lock lock( mInfoMutex );
      ++mInfo.StatesRecv;
    }
    mrParent.mStates.Delete( state.Name() );
    mrParent.mStates.Add( state );
    ostringstream oss;
    state.WriteToStream( oss );
    mrParent.ExecuteCallback( BCI_OnState, oss.str().c_str() );
    mrParent.EnterState( Publishing );
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleVisSignal( istream& is )
{
  VisSignal v;
  if( v.ReadBinary( is ) )
  {
    {
      OSMutex::Lock lock( mInfoMutex );
      ++mInfo.DataRecv;
    }
    const string kind = "Graph";
    mrParent.CheckInitializeVis( v.SourceID(), kind );
    int channels = v.Signal().Channels(),
        elements = v.Signal().Elements();
    float* pData = new float[ channels * elements ];
    for( int ch = 0; ch < channels; ++ch )
      for( int el = 0; el < elements; ++el )
        pData[ ch * elements + el ] = static_cast<float>( v.Signal()( ch, el ) );
    mrParent.ExecuteCallback( BCI_OnVisSignal, v.SourceID().c_str(), channels, elements, pData );
    delete[] pData;
  }
  return true;
}

bool
StateMachine::CoreConnection::HandleVisSignalProperties( istream& is )
{
  VisSignalProperties v;
  if( v.ReadBinary( is ) )
  {
    {
      OSMutex::Lock lock( mInfoMutex );
      ++mInfo.DataRecv;
    }
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
              groupLabels.push_back( Label( groupLabels.size(), label[ 0 ] ) );
          }
          channelLabels.push_back( Label( channelLabels.size(), label[ 1 ] ) );
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
  return true;
}

bool
StateMachine::CoreConnection::HandleVisMemo( istream& is )
{
  VisMemo v;
  if( v.ReadBinary( is ) )
  {
    {
      OSMutex::Lock lock( mInfoMutex );
      ++mInfo.DataRecv;
    }
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
    {
      OSMutex::Lock lock( mInfoMutex );
      ++mInfo.DataRecv;
    }
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
    {
      OSMutex::Lock lock( mInfoMutex );
      ++mInfo.DataRecv;
    }
    mrParent.mVisualizations[v.SourceID()][v.CfgID()] = v.CfgValue();
    mrParent.ExecuteCallback( BCI_OnVisPropertyMessage, v.SourceID().c_str(), v.CfgID(), v.CfgValue().c_str() );
  }
  return true;
}



