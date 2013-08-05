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
//   - triggering event callbacks to display text messages, or to visualize data.
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
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "MessageHandler.h"
#include "CallbackBase.h"
#include "SockStream.h"
#include "ParamList.h"
#include "StateList.h"
#include "StateVector.h"
#include "GenericSignal.h"
#include "VisTable.h"
#include "VersionInfo.h"
#include "ProtocolVersion.h"
#include "ScriptEvents.h"
#include "Watches.h"
#include "OSThread.h"
#include "OSMutex.h"
#include "Lockable.h"

#include <set>
#include <fstream>

class CommandInterpreter;

class StateMachine : public CallbackBase, private OSThread
{
 public:
  enum
  {
    ConnectionTimeout = 500, // ms
  };

  // Methods
 public:
  StateMachine();
  virtual ~StateMachine();

 private:
  StateMachine( const StateMachine& );
  StateMachine& operator=( const StateMachine& );

 public:
  // State manipulation.
  //  As an argument to the Startup() function, provide a list of module names
  //  and ports as in "SignalSource:4000 SignalProcessing:4001 Application:4002".
  //  In case of an empty argument, a single list entry "Core Module:4000" is
  //  assumed.
  bool Startup( const char* = NULL );
  bool SetConfig();
  bool StartRun();
  bool StopRun();
  bool Shutdown();
  bool Reset();

  // Properties

  //  System state.
  enum SysState
  {
    Idle = 0,
    WaitingForConnection,
    Publishing,
    Information,
    Initialization,
    SetConfigIssued,
    Resting,
    RunningInitiated,
    Running,
    SuspendInitiated,
    Suspended,
    Fatal,
    Transition,

    NumStates,

    ParamsModified = 1 << 8,
    StateFlags = ParamsModified,
  };

  int SystemState() const
    { OSMutex::Lock lock( mStateMutex ); return mSystemState; }

  int NumConnections() const
    { return static_cast<int>( mConnections.size() ); }

  //  Locking.
  void Lock() const
    { mDataMutex.Acquire(); }
  void Unlock() const
    { mDataMutex.Release(); }
  typedef ::Lock_<StateMachine> DataLock;
  struct WatchDataLock : DataLock
  {
    WatchDataLock( StateMachine* s ) : DataLock( s ) {}
    WatchDataLock( StateMachine& s ) : DataLock( s ) {}
    ~WatchDataLock() { ( *this )().Watches().Check(); }
  };
  void CheckWatches() { WatchDataLock( this ); }

  //  Parameter list.
  ParamList& Parameters()
    { return mParameters; }
  const ParamList& Parameters() const
    { return mParameters; }
  void ParameterChange();

  // State list.
  StateList& States()
    { return mStates; }
  const StateList& States() const
    { return mStates; }
  bool SetStateValue( const char* name, State::ValueType value );
  State::ValueType GetStateValue( const char* name ) const;

  // Event list.
  StateList& Events()
    { return mEvents; }
  const StateList& Events() const
    { return mEvents; }
  bool SetEvent( const char* name, State::ValueType value );

  // Control signal.
  const GenericSignal& ControlSignal() const
    { return mControlSignal; }

  // Table of visualization properties.
  VisTable& Visualizations()
    { return mVisualizations; }
  const VisTable& Visualizations() const
    { return mVisualizations; }

  // Scripting events.
  ScriptEvents& EventScripts()
    { return mScriptEvents; }
  const ScriptEvents& EventScripts() const
    { return mScriptEvents; }

  // Watches.
  Watch::List& Watches()
    { return mWatches; }
  const Watch::List& Watches() const
    { return mWatches; }

  // Address of local connection.
  const std::string& LocalAddress() const
    { return mLocalAddress; }
  std::string SuggestUDPAddress( const std::string& ) const;

  // Issue a log message.
  void LogMessage( int messageCallbackID, const std::string& );

  // Interface to CommandInterpreter class.
 public:
  void AddListener( CommandInterpreter& listener )
    { ::Lock lock( mListeners ); mListeners.insert( &listener ); }
  void RemoveListener( CommandInterpreter& listener )
    { ::Lock lock( mListeners ); mListeners.erase( &listener ); }

 private:
  virtual int OnExecute();

  void Init();

  void EnterState( enum SysState );
  void PerformTransition( int transition );
  void ExecuteTransitionCallbacks( int transition );

  void BroadcastParameters();
  void BroadcastEndOfParameter();
  void BroadcastParameterChanges();
  void BroadcastStates();
  void BroadcastEndOfState();
  void InitializeStateVector();
  void InitializeModules();
  void MaintainDebugLog();
  void DebugWarning();
  void Randomize();
  void RandomizationWarning();

  void TriggerEvent( int eventCallbackID );
  
 private:
  int mSystemState;
  // A mutex protecting access to the SystemState property:
  OSMutex           mStateMutex;
  // A mutex to be acquired while manipulating StateMachine data members.
  OSMutex           mDataMutex;
  #define mDataMutex (void) // use Lock()/Unlock() rather than accessing this member directly
  // A mutex to serialize incoming BCI messages:
  OSMutex           mBCIMessageMutex;

  bool              mIntroducedRandomSeed;
  std::string       mPreviousRandomSeed;
  ParamList         mParameters,
                    mAutoParameters;
  StateList         mStates,
                    mEvents;
  StateVector       mStateVector;
  GenericSignal     mControlSignal;
  VisTable          mVisualizations;
  std::string       mLocalAddress;

  std::ofstream     mDebugLog;

  struct Listeners : std::set<CommandInterpreter*>, Lockable {} mListeners;
  ScriptEvents      mScriptEvents;
  Watch::List       mWatches;

 public:
   struct ConnectionInfo : Lockable
  {
    ConnectionInfo()
    : Version( ProtocolVersion::None() ),
      Name( "" ),
      Address( "" ),
      Status( "no status available" ),
      MessagesSent( 0 ),
      MessagesRecv( 0 )
    {}

    ProtocolVersion Version;
    std::string     Name,
                    Address,
                    Status;
    long MessagesSent,
         MessagesRecv;
  };

 private:
  class CoreConnection : public MessageHandler
  {
    typedef StateMachine::SysState SysState;

   public:
    CoreConnection( StateMachine&, const std::string& name, const std::string& address, ptrdiff_t tag );
    ~CoreConnection() {}

    ptrdiff_t Tag() const
      { return mTag; }
    SysState State() const
      { return mState_; }
    ::Lock_<const ConnectionInfo> Info() const
      { return mInfo_; }
    streamsock* Socket()
      { return &mSocket; }

    void ProcessBCIMessages();
    void EnterState( SysState );
    template<typename T> bool PutMessage( const T& t )
      { return OnPutMessage( MessageHandler::PutMessage<T>( mStream, t ).flush() ); }

   private:
    virtual bool HandleProtocolVersion( std::istream& );
    virtual bool HandleStatus( std::istream& );
    virtual bool HandleSysCommand( std::istream& );
    virtual bool HandleParam( std::istream& );
    virtual bool HandleState( std::istream& );
    virtual bool HandleStateVector( std::istream& );
    virtual bool HandleVisSignal( std::istream& );
    virtual bool HandleVisSignalProperties( std::istream& );
    virtual bool HandleVisMemo( std::istream& );
    virtual bool HandleVisBitmap( std::istream& );
    virtual bool HandleVisCfg( std::istream& );

    void OnAccept();
    void OnDisconnect();
    bool OnPutMessage( bool inSuccess )
      { if( inSuccess ) ++Info()().MessagesSent; return inSuccess; }
    ::Lock_<ConnectionInfo> Info()
      { return mInfo_; }
 
    StateMachine&    mrParent;
    std::string      mAddress;
    ptrdiff_t        mTag;
    server_tcpsocket mSocket;
    sockstream       mStream;
    // Members that should not be accessed directly.
    SysState         mState_;
    ConnectionInfo   mInfo_;
  };
  typedef std::vector<CoreConnection*> ConnectionList;

 public:
  ConnectionInfo Info( size_t i ) const;

 public:
  bool HandleStateVector( const CoreConnection&, std::istream& );
  void Handle( const CoreConnection&, const ProtocolVersion& );
  void Handle( const CoreConnection&, const Status& );
  void Handle( const CoreConnection&, const SysCommand& );
  void Handle( const CoreConnection&, const Param& );
  void Handle( const CoreConnection&, const State& );
  void Handle( const CoreConnection&, const VisSignal& );
  void Handle( const CoreConnection&, const VisSignalProperties& );
  void Handle( const CoreConnection&, const VisMemo& );
  void Handle( const CoreConnection&, const VisBitmap& );
  void Handle( const CoreConnection&, const VisCfg& );
  void Handle( const CoreConnection&, SysState );

 private:
  bool IsConsistentState( SysState ) const;
  void SetConnectionState( SysState );
  void CloseConnections();

  streamsock::set_of_instances mSockets;
  ConnectionList mConnections;
  CoreConnection* mpSourceModule;

 private:
  bool CheckInitializeVis( const std::string& sourceID, const std::string& kind );

  class EventLink;
  friend class EventLink;
  class EventLink : public sockstream, public Lockable, private OSThread
  {
   public:
    EventLink( StateMachine& s ) : mrParent( s ), mConnected( false ), mPort( 0 ) {}
    ~EventLink() { OSThread::TerminateWait(); }
    void Open( int port ) { mPort = port; OSThread::Start(); }
    void Close() { OSThread::TerminateWait(); mSocket.close(); mConnected = false; }
    void ConfirmConnection();
    bool Connected() const { return mConnected; }
   private:
    int OnExecute();
   private:
    StateMachine& mrParent;
    int mPort;
    sending_udpsocket mSocket;
    bool mConnected;
  } mEventLink;
};

#endif // STATE_MACHINE_H

