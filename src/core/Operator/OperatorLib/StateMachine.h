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

#include "CallbackBase.h"
#include "SockStream.h"
#include "Param.h"
#include "ParamList.h"
#include "State.h"
#include "StateList.h"
#include "StateVector.h"
#include "GenericSignal.h"
#include "VisTable.h"
#include "VersionInfo.h"
#include "ProtocolVersion.h"
#include "ScriptEvents.h"
#include "MessageHandler.h"
#include "OSThread.h"
#include "OSMutex.h"

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
    RestingParamsModified,
    RunningInitiated,
    Running,
    SuspendInitiated,
    Suspended,
    SuspendedParamsModified,
    Fatal,
    Transition,

    NumStates
  };

  enum SysState SystemState() const
    { OSMutex::Lock lock( mStateMutex ); return mSystemState; }

  int NumConnections() const
    { return static_cast<int>( mConnections.size() ); }

  //  Locking.
  void Lock()
    { mDataMutex.Acquire(); }
  void Unlock()
    { mDataMutex.Release(); }

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

  // Issue a log message.
  void LogMessage( int messageCallbackID, const std::string& );

  // Interface to CommandInterpreter class.
 public:
  void AddListener( CommandInterpreter& listener )
    { ::Lock<Listeners> lock( mListeners ); mListeners.insert( &listener ); }
  void RemoveListener( CommandInterpreter& listener )
    { ::Lock<Listeners> lock( mListeners ); mListeners.erase( &listener ); }

 private:
  virtual int OnExecute();

  void Terminate();

  void EnterState( enum SysState );
  void PerformTransition( int transition );
  void ExecuteTransitionCallbacks( int transition );

  void BroadcastParameters();
  void BroadcastEndOfParameter();
  void BroadcastStates();
  void BroadcastEndOfState();
  void InitializeStateVector();
  void InitializeModules();
  void MaintainDebugLog();
  void Randomize();
  void RandomizationWarning();

  void TriggerEvent( int eventCallbackID );

 private:
  enum SysState mSystemState;
  // A mutex protecting access to the SystemState property:
  OSMutex           mStateMutex;
  // A mutex to be acquired while manipulating StateMachine data members,
  // and released during callbacks:
  OSMutex           mDataMutex;
  // A mutex to serialize incoming BCI messages:
  OSMutex           mBCIMessageMutex;

  bool              mIntroducedRandomSeed;
  std::string       mPreviousRandomSeed;
  ParamList         mParameters;
  StateList         mStates,
                    mEvents;
  StateVector       mStateVector;
  GenericSignal     mControlSignal;
  VisTable          mVisualizations;
  ScriptEvents      mScriptEvents;

  std::ofstream     mDebugLog;

  struct Listeners : std::set<CommandInterpreter*>, Lockable
  {
  } mListeners;

 public:
  struct ConnectionInfo
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
  class CoreConnection;
  friend class CoreConnection;
  class CoreConnection : public MessageHandler
  {
    friend class StateMachine;

   public:
    CoreConnection( StateMachine&, const std::string& inName, const std::string& inAddress, int inTag );
    ~CoreConnection();

    enum Confirmation
    {
      ConfirmInitialized,
      ConfirmRunning,
      ConfirmSuspended,
      ConfirmEndOfStates,

      NumConfirmations
    };

   private:
    void Confirm( Confirmation c )
      { mConfirmed[c] = true; }
   public:
    void ClearConfirmation( Confirmation c )
      { mConfirmed[c] = false; }
    bool Confirmed( Confirmation c ) const
      { return mConfirmed[c]; }
   private:
    bool mConfirmed[NumConfirmations];

   public:
    void ProcessBCIMessages();

    template<typename T> bool PutMessage( const T& t )
      {
        bool result = MessageHandler::PutMessage<T>( mStream, t ).flush();
        if( result )
        {
          OSMutex::Lock lock( mInfoMutex );
          ++mInfo.MessagesSent;
        }
        return result;
      }

    const ConnectionInfo Info() const
      { OSMutex::Lock lock( mInfoMutex ); return mInfo; }

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

    StateMachine&    mrParent;
    std::string      mAddress;
    int              mTag;
    server_tcpsocket mSocket;
    sockstream       mStream;
    bool             mConnected;
    bool             mConfirmation[NumConfirmations];
    ConnectionInfo   mInfo;
    // A mutex to protect info data.
    OSMutex          mInfoMutex;
  };
  typedef std::vector<CoreConnection*> ConnectionList;


 public:
  ConnectionInfo Info( size_t i ) const;

 private:
  streamsock::set_of_instances mSockets;
  CoreConnection* mpSourceModule;

  void ClearConfirmation( CoreConnection::Confirmation c )
  {
    for( ConnectionList::iterator i = mConnections.begin(); i != mConnections.end(); ++i )
      ( *i )->ClearConfirmation( c );
  }

  bool Confirmed( CoreConnection::Confirmation c ) const
  {
    bool result = true;
    for( ConnectionList::const_iterator i = mConnections.begin(); i != mConnections.end(); ++i )
      result &= ( *i )->Confirmed( c );
    return result;
  }

  void CloseConnections()
  {
    mpSourceModule = NULL;
    for( ConnectionList::iterator i = mConnections.begin(); i != mConnections.end(); ++i )
      delete *i;
    mConnections.clear();
    mEventLink.Close();
  }

  ConnectionList mConnections;

 private:
  bool CheckInitializeVis( const std::string& sourceID, const std::string& kind );

  class EventLink;
  friend class EventLink;
  class EventLink : public sockstream, public Lockable, private OSThread
  {
   public:
    EventLink( StateMachine& s ) : mrParent( s ), mConnected( false ), mPort( 0 ) {}
    void Open( int port ) { mPort = port; OSThread::Start(); }
    void Close() { OSThread::TerminateWait(); mSocket.close(); }
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

