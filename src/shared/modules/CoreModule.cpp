////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that represents functionality common to all BCI2000
//          core modules.
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

#include "CoreModule.h"

#include "GenericFilter.h"
#include "GenericSignal.h"
#include "GenericVisualization.h"
#include "ProtocolVersion.h"
#include "SysCommand.h"
#include "Status.h"
#include "MessageHandler.h"
#include "MeasurementUnits.h"
#include "SockStream.h"
#include "BCIError.h"
#include "VersionInfo.h"
#include "Version.h"
#include "ExceptionCatcher.h"

#include <string>
#include <sstream>

#ifdef __BORLANDC__
# include <vcl.h>
#endif
#ifndef _WIN32
# include <sys/sem.h>
#endif

#define BCIERR (bcierr__ << THISMODULE ": ")

using namespace std;

static GenericSignal sNullSignal( 0, 0 );

CoreModule::CoreModule()
: mReceivingThread( *this ),
  mTerminated( false ),
  mpStatevector( NULL ),
  mFiltersInitialized( false ),
  mLastRunning( false ),
  mResting( false ),
  mStartRunPending( false ),
  mStopRunPending( false ),
  mFirstStatevectorPending( false ),
  mMutex( NULL ),
  mSampleBlockSize( 0 )
#if _WIN32
, mFPMask( cDisabledFPExceptions )
#endif // _WIN32
{
  mOperatorSocket.set_tcpnodelay( true );
  mNextModuleSocket.set_tcpnodelay( true );
  mPreviousModuleSocket.set_tcpnodelay( true );
}

CoreModule::~CoreModule()
{
  delete mpStatevector;
  if( mMutex != NULL )
  {
#ifdef _WIN32
    ::ReleaseMutex( mMutex );
    ::CloseHandle( mMutex );
#elif 0
    ::semctl( reinterpret_cast<int>( mMutex ), 0, IPC_RMID, 0 );
#endif // _WIN32
  }
}

void
CoreModule::DoRun( int inArgc, char** inArgv )
{
  if( Initialize( inArgc, inArgv ) )
    MainMessageLoop();
}

bool
CoreModule::Run( int inArgc, char** inArgv )
{
  MemberCall< void( CoreModule*, int, char** ) >
    call( &CoreModule::DoRun, this, inArgc, inArgv );
  ExceptionCatcher()
    .SetMessage( "terminating " THISMODULE " module" )
    .Run( call );
  ShutdownSystem();
  return ( bcierr__.Flushes() == 0 );
}

// Internal functions.
bool
CoreModule::Initialize( int inArgc, char** inArgv )
{
  OnInitialize( inArgc, inArgv );
  // Make sure there is only one instance of each module running at a time.
  // We create a mutex from the module name to encode that we are running.
#ifdef _WIN32
  mMutex = ::CreateMutex( NULL, true, THISMODULE " Module" );
  if( ::GetLastError() == ERROR_ALREADY_EXISTS )
    return false;
#elif 0
  key_t key = 'BCI0' | MODTYPE;
  int semaphore = ::semget( key, 1, IPC_CREAT | IPC_EXCL | 0666 );
  if( semaphore == -1 )
  {
    mMutex = NULL;
    return false;
  }
  mMutex = reinterpret_cast<void*>( semaphore );
#endif // _WIN32

  string operatorAddress;
  bool   printVersion = false,
         printHelp = false;
  int i = 1;
  while( i < inArgc )
  {
    string curArg( inArgv[ i ] );
    if( curArg == "--version" || curArg == "-v" )
    {
      printVersion = true;
    }
    if( curArg.find( "--" ) == 0 )
    {
      string paramDef = curArg.substr( 2 );
      size_t nameEnd = paramDef.find_first_of( "-=:" );
      string paramName = paramDef.substr( 0, nameEnd ),
             paramValue;
      if( nameEnd != string::npos )
        paramValue = paramDef.substr( nameEnd + 1 );
      Param param( paramName,
                   "System:Command Line Arguments",
                   "variant",
                   paramValue,
                   paramValue,
                   paramValue,
                   paramValue
                 );
      mParamlist.Add( param );
    }
    else if( curArg == "AUTOSTART" )
    { /* do nothing */
    }
    else if( operatorAddress.empty() )
    {
      operatorAddress = curArg;
    }
    else
    {
      printHelp = true;
    }
    ++i;
  }

  bciout__.SetFlushHandler( BCIError::PlainMessage );
  if( printVersion )
  {
    VersionInfo versionInfo;
    istringstream iss( BCI2000_VERSION );
    iss >> versionInfo;
    bciout__ << "BCI2000 " THISMODULE " \n\n"
             << " Framework: " << versionInfo[ VersionInfo::VersionID ] << " \n";
    if( !versionInfo[ VersionInfo::Revision ].empty() )
      bciout__ << " Revision: " << versionInfo[ VersionInfo::Revision ]
               <<          ", " << versionInfo[ VersionInfo::SourceDate ] << " \n";
    bciout__ << " Build: " << versionInfo[ VersionInfo::BuildDate ]
             << endl;
    return true;
  }
  if( printHelp )
  {
    string executableName = inArgv[ 0 ];
    size_t pos = executableName.find_last_of( "/\\" );
    if( pos != string::npos )
      executableName = executableName.substr( pos + 1 );

    bciout__ << "Usage:\n"
             << executableName << " <address>:<port> --<option>-<value>\n"
             << " address:\tip address of operator module (default 127.0.0.1)\n"
             << " port:   \toperator port (default " THISOPPORT ")\n"
             << "Options will appear as parameters in the System section."
             << "Specifying --version will display version information."
             << endl;
    return false;
  }

  if( operatorAddress.empty() )
    operatorAddress = "127.0.0.1";
  if( operatorAddress.find( ":" ) == string::npos )
    operatorAddress += ":" THISOPPORT;

  InitializeOperatorConnection( operatorAddress );
  bool success = ( bcierr__.Flushes() == 0 );
  if( success )
    mReceivingThread.Start();
  return success;
}

int
CoreModule::ReceivingThread::Execute()
{
  const int cSocketTimeout = 105;
  while( !mrParent.mTerminated )
  {
    mrParent.mConnectionLock.Acquire();
    streamsock::set_of_instances inputSockets;
    if( mrParent.mOperator.is_open() )
      inputSockets.insert( &mrParent.mOperatorSocket );
    if( mrParent.mPreviousModule.is_open() )
      inputSockets.insert( &mrParent.mPreviousModuleSocket );
    mrParent.mConnectionLock.Release();
    if( !inputSockets.empty() )
      streamsock::wait_for_read( inputSockets, cSocketTimeout );

    mrParent.mConnectionLock.Acquire();
    while( mrParent.mOperator && mrParent.mOperator.rdbuf()->in_avail() )
      mrParent.mMessageQueue.QueueMessage( mrParent.mOperator );
    while( mrParent.mPreviousModule && mrParent.mPreviousModule.rdbuf()->in_avail() )
      mrParent.mMessageQueue.QueueMessage( mrParent.mPreviousModule );
    mrParent.mConnectionLock.Release();

    mrParent.mMessageEvent.Set();
  }
  return 0;
}

// This function contains the main event handling loop.
// It will be entered once when the program starts,
// and only be left when the program quits.
void
CoreModule::MainMessageLoop()
{
  const int bciMessageTimeout = 100; // ms, is max time a GUI event needs to wait
                                     // before it gets processed
  while( !mTerminated )
  {
    mMessageEvent.Wait( bciMessageTimeout );
    mMessageEvent.Reset();
    ProcessBCIAndGUIMessages();
    mConnectionLock.Acquire();
    bool operatorOpen = mOperator.is_open();
    mConnectionLock.Release();
    if( !operatorOpen )
      Terminate();
  }
}


void
CoreModule::ProcessBCIAndGUIMessages()
{
  while( !mMessageQueue.Empty()
        || mResting
        || OnGUIMessagesPending() )
  {
    while( !mMessageQueue.Empty() )
      MessageHandler::HandleMessage( mMessageQueue );
    // The mResting flag is treated as a pending message from the module to itself.
    // For non-source modules, it is cleared from the HandleResting() function
    // much as pending messages are cleared from the stream by the HandleMessage()
    // function.
    if( mResting )
      HandleResting();
    mConnectionLock.Acquire();
    mResting &= mOperator.is_open();
    mConnectionLock.Release();
    // Last of all, allow for the GUI to process messages from its message queue if there are any.
    OnProcessGUIMessages();
  }
}


void
CoreModule::InitializeOperatorConnection( const string& inOperatorAddress )
{
  OSMutex::Lock lock( mConnectionLock );
  // creating connection to the operator
  mOperatorSocket.open( inOperatorAddress.c_str() );
  if( !mOperatorSocket.is_open() )
  { // wait if connection to operator module fails
    const int operatorWaitInterval = 2000; // ms
    mOperatorSocket.wait_for_write( operatorWaitInterval );
    mOperatorSocket.open( inOperatorAddress.c_str() );
  }
  mOperator.clear();
  mOperator.open( mOperatorSocket );
  if( !mOperator.is_open() )
  {
    BCIERR << "Could not connect to operator module" << endl;
    return;
  }
  BCIError::SetOperatorStream( &mOperator, &mConnectionLock );
  GenericVisualization::SetOutputStream( &mOperator, &mConnectionLock );

  if( mParamlist.Exists( THISMODULE "IP" ) )
    mPreviousModuleSocket.open( mParamlist[ THISMODULE "IP" ].Value().c_str() );
  else
  {
    streamsock::set_of_addresses addr = streamsock::local_addresses();
    mPreviousModuleSocket.open( addr.rbegin()->c_str() );
  }
  mPreviousModule.clear();
  mPreviousModule.open( mPreviousModuleSocket );

  EnvironmentBase::EnterConstructionPhase( &mParamlist, &mStatelist, NULL );
  GenericFilter::InstantiateFilters();
  EnvironmentBase::EnterNonaccessPhase();
  if( bcierr__.Flushes() > 0 )
    return;

  // add parameters for socket connection
  // my receiving socket port number
  mParamlist.Add(
    "System:Core%20Connections string " THISMODULE "Port= x"
    " 4200 1024 65535 // the " THISMODULE " module's listening port" );
  ostringstream port;
  port << mPreviousModuleSocket.port();
  mParamlist[ THISMODULE "Port" ].Value() = port.str();
  // and IP address
  mParamlist.Add(
    "System:Core%20Connections string " THISMODULE "IP= x"
    " 127.0.0.1 % % // the " THISMODULE " module's listening IP" );
  mParamlist[ THISMODULE "IP" ].Value() = mPreviousModuleSocket.ip();

  // Version control
  VersionInfo versionInfo;
  istringstream iss( BCI2000_VERSION );
  iss >> versionInfo;
  mParamlist.Add(
    "System:Configuration matrix " THISMODULE "Version= "
      "{ Framework Revision Build } 1 "
      " % % % // " THISMODULE " version information" );
  mParamlist[ THISMODULE "Version" ].Value( "Framework" )
    = versionInfo[ VersionInfo::VersionID ];
  if( versionInfo[ VersionInfo::Revision ].empty() )
    mParamlist[ THISMODULE "Version" ].Value( "Revision" )
      = versionInfo[ VersionInfo::SourceDate ];
  else
    mParamlist[ THISMODULE "Version" ].Value( "Revision" )
      = versionInfo[ VersionInfo::Revision ] + ", " +  versionInfo[ VersionInfo::SourceDate ];
  mParamlist[ THISMODULE "Version" ].Value( "Build" )
    = versionInfo[ VersionInfo::BuildDate ];
  // Filter chain documentation
  mParamlist.Add(
    "System:Configuration matrix " THISMODULE "FilterChain= "
      "0 { Filter%20Name Position%20String } "
      " % % % // " THISMODULE " filter chain" );
  Param& p = mParamlist[ THISMODULE "FilterChain" ];
  const GenericFilter::ChainInfo& chain = GenericFilter::GetChainInfo();
  p.SetNumRows( chain.size() );
  for( size_t row = 0; row < chain.size(); ++row )
  {
    p.Value( row, "Filter Name" ) = chain[ row ].name;
    p.Value( row, "Position String" ) = chain[ row ].position;
  }
  // First, send a protocol version message
  MessageHandler::PutMessage( mOperator, ProtocolVersion::Current() );
  // Now, publish parameters ...
  mParamlist.Sort();
  MessageHandler::PutMessage( mOperator, mParamlist );
  MessageHandler::PutMessage( mOperator, SysCommand::EndOfParameter );
  // ... and states.
  MessageHandler::PutMessage( mOperator, mStatelist );
  MessageHandler::PutMessage( mOperator, SysCommand::EndOfState );

  MessageHandler::PutMessage( mOperator, Status( "Waiting for configuration ...", Status::plainMessage ) );
}


void
CoreModule::InitializeCoreConnections()
{
  OSMutex::Lock lock( mConnectionLock );

  const char* ipParam = NEXTMODULE "IP",
            * portParam = NEXTMODULE "Port";

  if( !mParamlist.Exists( ipParam ) || !mParamlist.Exists( portParam ) )
  {
    BCIERR << NEXTMODULE "IP/Port parameters not available"
           << endl;
    return;
  }
  string ip = mParamlist[ ipParam ].Value(),
         port = mParamlist[ portParam ].Value();
  mNextModuleSocket.open( ( ip + ":" + port ).c_str() );
  mNextModule.open( mNextModuleSocket );
  if( !mNextModule.is_open() )
  {
    BCIERR << "Could not make a connection to the " NEXTMODULE " module"
           << endl;
    return;
  }
  OSThread::Sleep( 0 ); // This prevents mysterious closing of the mNextModuleSocket.

  mPreviousModule.close();
  mPreviousModule.clear();
  bool waitForConnection = mPreviousModuleSocket.is_open() && !mPreviousModuleSocket.connected();
  if( waitForConnection )
    mPreviousModuleSocket.wait_for_read( cInitialConnectionTimeout, true );
  mPreviousModule.open( mPreviousModuleSocket );
  if( waitForConnection && !mPreviousModule.is_open() )
  {
    BCIERR << "Connection to previous module timed out after "
           << float( cInitialConnectionTimeout ) / 1e3 << "s"
           << endl;
    return;
  }
}


void
CoreModule::ShutdownSystem()
{
  BCIError::SetOperatorStream( NULL, NULL );
  GenericVisualization::SetOutputStream( NULL, NULL );

  mOperatorSocket.close();
  mPreviousModuleSocket.close();
  mNextModuleSocket.close();

  delete mpStatevector;
  mpStatevector = NULL;

  GenericFilter::DisposeFilters();
}


void
CoreModule::ResetStatevector()
{
  State::ValueType sourceTime = mpStatevector->StateValue( "SourceTime" ),
                   stimulusTime = mpStatevector->StateValue( "StimulusTime" );
  for( int i = 0; i < mpStatevector->Samples(); ++i )
  {
    istringstream iss( mInitialStatevector );
    ( *mpStatevector )( i ).ReadBinary( iss );
  }
  mpStatevector->SetStateValue( "SourceTime", sourceTime );
  mpStatevector->SetStateValue( "StimulusTime", stimulusTime );
}


void
CoreModule::InitializeFilters( const SignalProperties& inputProperties )
{
  mStartRunPending = true;
  bcierr__.Clear();
  GenericFilter::HaltFilters();
  bool errorOccurred = ( bcierr__.Flushes() > 0 );
  EnvironmentBase::EnterPreflightPhase( &mParamlist, &mStatelist, mpStatevector );
#ifdef TODO
# error The inputPropertiesFixed variable may be removed once property messages contain an UpdateRate field.
#endif // TODO
  SignalProperties inputPropertiesFixed( inputProperties );
  // MeasurementUnits gets initialized in Environment::EnterPreflightPhase
  inputPropertiesFixed.SetUpdateRate( 1.0 / MeasurementUnits::SampleBlockDuration() );
  mInputSignal = GenericSignal( inputPropertiesFixed );
  SignalProperties outputProperties( 0, 0 );
  GenericFilter::PreflightFilters( inputPropertiesFixed, outputProperties );
  EnvironmentBase::EnterNonaccessPhase();
  errorOccurred |= ( bcierr__.Flushes() > 0 );
  OSMutex::Lock lock( mConnectionLock );
  if( !errorOccurred )
  {
#if( MODTYPE != APP )
    if( !MessageHandler::PutMessage( mNextModule, outputProperties ) )
      BCIERR << "Could not send output properties to " NEXTMODULE " module" << endl;
#endif // APP
    mOutputSignal = GenericSignal( outputProperties );
    EnvironmentBase::EnterInitializationPhase( &mParamlist, &mStatelist, mpStatevector );
    GenericFilter::InitializeFilters();
    EnvironmentBase::EnterNonaccessPhase();
    errorOccurred |= ( bcierr__.Flushes() > 0 );
  }
  if( !mPreviousModule.is_open() )
  {
    BCIERR << PREVMODULE " dropped connection unexpectedly" << endl;
    errorOccurred = true;
  }
  if( !errorOccurred )
  {
    MessageHandler::PutMessage( mOperator, Status( THISMODULE " initialized", Status::firstInitializedMessage + MODTYPE - 1 ) );
    mFiltersInitialized = true;
  }
#if( MODTYPE == SIGSRC )
  mResting = !errorOccurred;
#endif // SIGSRC
}


void
CoreModule::StartRunFilters()
{
  mStartRunPending = false;
  mFirstStatevectorPending = true;
  // The first state vector written to disk is not the one
  // received in response to the first EEG data block. Without resetting it
  // to its initial value,
  // there would be state information from the end of the (possibly
  // interrupted) previous run written with the first EEG data block.
  ResetStatevector();
  mpStatevector->SetStateValue( "Running", 1 );

  EnvironmentBase::EnterStartRunPhase( &mParamlist, &mStatelist, mpStatevector );
  GenericFilter::StartRunFilters();
  EnvironmentBase::EnterNonaccessPhase();
  if( bcierr__.Flushes() == 0 )
  {
    OSMutex::Lock lock( mConnectionLock );
    MessageHandler::PutMessage( mOperator, Status( THISMODULE " running", Status::firstRunningMessage + 2 * ( MODTYPE - 1 ) ) );
    mResting = false;
  }
}


void
CoreModule::StopRunFilters()
{
  mStopRunPending = false;
  mStartRunPending = true;
  EnvironmentBase::EnterStopRunPhase( &mParamlist, &mStatelist, mpStatevector );
  GenericFilter::StopRunFilters();
  EnvironmentBase::EnterNonaccessPhase();
  if( bcierr__.Flushes() == 0 )
  {
    BroadcastParameterChanges();

    OSMutex::Lock lock( mConnectionLock );
#if( MODTYPE == SIGSRC ) // The operator wants an extra invitation from the source module.
    MessageHandler::PutMessage( mOperator, SysCommand::Suspend );
#endif // SIGSRC
    MessageHandler::PutMessage( mOperator, Status( THISMODULE " suspended", Status::firstSuspendedMessage + 2 * ( MODTYPE - 1 ) ) );
    mResting = true;
  }
}

void
CoreModule::BroadcastParameterChanges()
{
  ParamList changedParameters;
  for( int i = 0; i < mParamlist.Size(); ++i )
    if( mParamlist[ i ].Changed() )
      changedParameters.Add( mParamlist[ i ] );

  if( !changedParameters.Empty() )
  {
    OSMutex::Lock lock( mConnectionLock );
    bool success = MessageHandler::PutMessage( mOperator, changedParameters )
                && MessageHandler::PutMessage( mOperator, SysCommand::EndOfParameter );
    if( !success )
      BCIERR << "Could not publish changed parameters" << endl;
  }
}

void
CoreModule::ProcessFilters( const GenericSignal& input )
{
  EnvironmentBase::EnterProcessingPhase( &mParamlist, &mStatelist, mpStatevector );
  GenericFilter::ProcessFilters( input, mOutputSignal );
  EnvironmentBase::EnterNonaccessPhase();
  bool errorOccurred = ( bcierr__.Flushes() > 0 );
  if( errorOccurred )
  {
    Terminate();
    return;
  }
  OSMutex::Lock lock( mConnectionLock );
  MessageHandler::PutMessage( mNextModule, *mpStatevector );
#if( MODTYPE != APP )
  MessageHandler::PutMessage( mNextModule, mOutputSignal );
#endif // APP
}


void
CoreModule::RestingFilters()
{
  EnvironmentBase::EnterRestingPhase( &mParamlist, &mStatelist, mpStatevector );
  GenericFilter::RestingFilters();
  EnvironmentBase::EnterNonaccessPhase();
  bool errorOccurred = ( bcierr__.Flushes() > 0 );
  OSMutex::Lock lock( mConnectionLock );
  if( errorOccurred || !mOperator.is_open() )
  {
    mResting = false;
    Terminate();
  }
}


void
CoreModule::HandleResting()
{
  RestingFilters();
#if( MODTYPE != SIGSRC ) // For non-source modules, Resting() is called once
                         // after the Running state drops to 0.
  mResting = false;
#endif // SIGSRC
}


bool
CoreModule::HandleParam( istream& is )
{
  if( mpStatevector && mpStatevector->StateValue( "Running" ) )
    BCIERR << "Unexpected Param message" << endl;

  Param p;
  if( p.ReadBinary( is ) )
    mParamlist[ p.Name() ] = p;
  return is ? true : false;
}


bool
CoreModule::HandleState( istream& is )
{
  State s;
  if( s.ReadBinary( is ) )
  {
    if( mpStatevector )
    {
#if( MODTYPE == SIGSRC )
      // Changing a state's value via mpStatevector->PostStateChange()
      // will buffer the change, and postpone it until the next call to
      // mpStatevector->CommitStateChanges(). That call happens
      // after arrival of a StateVector message to make sure that
      // changes are not overwritten with the content of the previous
      // state vector when it arrives from the application module.
       mpStatevector->PostStateChange( s.Name(), s.Value() );

      // For the "Running" state, the module will undergo a more complex
      // state transition than for other states.
      if( string( "Running" ) == s.Name() )
      {
        bool running = mpStatevector->StateValue( "Running" ),
             nextRunning = s.Value();
        if( !running && nextRunning )
        {
          mLastRunning = true;
          StartRunFilters();
          ProcessFilters( sNullSignal );
        }
      }
#else // SIGSRC
      bcierr << "Unexpectedly received a State message" << endl;
#endif // SIGSRC
    }
    else
    {
#ifdef TODO
# error Remove saving of State::Kind once the protocol has been modified
#endif // TODO
      int kind = State::StateKind;
      if( mStatelist.Exists( s.Name() ) )
        kind = mStatelist[s.Name()].Kind();
      mStatelist.Delete( s.Name() );
      mStatelist.Add( s );
      mStatelist[s.Name()].SetKind( kind );
    }
  }
  return is ? true : false;
}


bool
CoreModule::HandleVisSignal( istream& is )
{
  VisSignal s;
  if( s.ReadBinary( is ) && s.SourceID() == "" )
  {
    mInputSignal.AssignValues( s );

    if( !mFiltersInitialized )
      bcierr << "Unexpected VisSignal message" << endl;
    else
    {
      if( mStartRunPending )
        StartRunFilters();
      ProcessFilters( mInputSignal );
      if( mStopRunPending )
        StopRunFilters();
    }
  }
  return is ? true : false;
}


bool
CoreModule::HandleVisSignalProperties( istream& is )
{
  VisSignalProperties s;
  if( s.ReadBinary( is ) && s.SourceID() == "" )
  {
    if( !mFiltersInitialized )
      InitializeFilters( s.SignalProperties() );
  }
  return is ? true : false;
}


bool
CoreModule::HandleStateVector( istream& is )
{
#if( MODTYPE == SIGSRC )
  bool success = false;
  if( mFirstStatevectorPending )
  { // To avoid overwriting state values with their initial values,
    // we ignore the first statevector that comes in from the application
    // module.
    success = StateVector( mStatelist ).ReadBinary( is );
    mFirstStatevectorPending = false;
  }
  else
  {
    success = mpStatevector->ReadBinary( is );
  }
  if( success )
  {
    mpStatevector->CommitStateChanges();
    // The source module does not receive a signal, so handling must take place
    // on arrival of a StateVector message.
    if( mLastRunning ) // For the first "Running" block, Process() is called from
                       // HandleState(), and may not be called here.
                       // For the first "Suspended" block, we need to call
                       // Process() one last time.
                       // By evaluating at "mLastRunning" instead of "running" we
                       // obtain this behavior.
    {
      ProcessFilters( sNullSignal );
    }
    // One of the filters might have set Running to 0 during ProcessFilters().
    bool running = mpStatevector->StateValue( "Running" );
    if( !running && mLastRunning )
      StopRunFilters();
    mLastRunning = running;
  }
#else // SIGSRC
  if( mpStatevector->ReadBinary( is ) )
  {
    bool running = mpStatevector->StateValue( "Running" );
    if( !running && mLastRunning )
      mStopRunPending = true;
    mLastRunning = running;
  }
#endif // SIGSRC
  return is ? true : false;
}


bool
CoreModule::HandleSysCommand( istream& is )
{
  SysCommand s;
  if( s.ReadBinary( is ) )
  {
    int sampleBlockSize = 1;
    if( mParamlist.Exists( "SampleBlockSize" ) )
      sampleBlockSize = static_cast<int>( PhysicalUnit()
                                         .SetOffset( 0.0 ).SetGain( 1.0 ).SetSymbol( "" )
                                         .PhysicalToRaw( mParamlist[ "SampleBlockSize" ].Value() )
                                        );
    if( sampleBlockSize < 1 )
      sampleBlockSize = 1;
    mSampleBlockSize = sampleBlockSize;

    if( s == SysCommand::EndOfState )
    {
      if( mpStatevector != NULL )
        bcierr << "Unexpected SysCommand::EndOfState message" << endl;

      delete mpStatevector;
      // Initialize the state vector from the state list.
      mStatelist.AssignPositions();
      // The state vector holds an additional sample which is used to initialize
      // the subsequent state vector.
      mpStatevector = new StateVector( mStatelist, mSampleBlockSize + 1 );
      ostringstream oss;
      ( *mpStatevector )( 0 ).WriteBinary( oss );
      mInitialStatevector = oss.str();
      mpStatevector->CommitStateChanges();
      InitializeCoreConnections();
      mFiltersInitialized = false;
    }
    else if( s == SysCommand::EndOfParameter )
    {
      // This happens for subsequent initializations.
      if( mpStatevector != NULL )
      {
        delete mpStatevector;
      // The state vector holds an additional sample which is used to initialize
      // the subsequent state vector.
        mpStatevector = new StateVector( mStatelist, mSampleBlockSize + 1 );
        ResetStatevector();
        mpStatevector->SetStateValue( "Running", 0 );
        mFiltersInitialized = false;
      }
    }
    else if( s == SysCommand::Start )
    {
      /* do nothing */
    }
    else if( s == SysCommand::Reset )
      Terminate();
    else
      BCIERR << "Unexpected SysCommand" << endl;
  }
  return is ? true : false;
}


