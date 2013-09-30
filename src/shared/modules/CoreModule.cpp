////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that represents functionality common to all BCI2000
//          core modules.
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
#include "BCIStream.h"
#include "VersionInfo.h"
#include "FileUtils.h"
#include "ProcessUtils.h"
#include "ExceptionCatcher.h"

#include <string>
#include <sstream>
#include <cstdlib>

#ifndef _WIN32
# include <sys/sem.h>
#endif

#define BCIERR (bcierr__ << sModuleName << ": ")

using namespace std;

static const string sModuleName = FileUtils::ExtractBase( FileUtils::ExecutablePath() );

CoreModule::CoreModule()
: mFiltersInitialized( false ),
  mRunning( false ),
  mActiveResting( false ),
  mStartRunPending( false ),
  mStopRunPending( false ),
  mNeedStopRun( false ),
  mReceivingNextModuleInfo( false ),
  mGlobalID( NULL ),
  mOperatorBackLink( false ),
  mAutoConfig( false )
{
  mOperatorSocket.set_tcpnodelay( true );
  mNextModuleSocket.set_tcpnodelay( true );
  mPreviousModuleSocket.set_tcpnodelay( true );

  SharedPointer<Runnable> p( new MemberCall<void(CoreModule*)>( &CoreModule::InitializeStatevector, this ) );
  MeasurementUnits::AddInitializeCallback( p );
}

CoreModule::~CoreModule()
{
}

void
CoreModule::DoRun( int& ioArgc, char** ioArgv )
{
  if( Initialize( ioArgc, ioArgv ) )
    MainMessageLoop();
}

bool
CoreModule::Run( int& ioArgc, char** ioArgv )
{
  MemberCall< void( CoreModule*, int&, char** ) >
    call( &CoreModule::DoRun, this, ioArgc, ioArgv );
  ExceptionCatcher()
    .SetMessage( "Terminating " + sModuleName + " module" )
    .Run( call );
  TerminateWait();
  ShutdownSystem();
  return ( bcierr__.Flushes() == 0 );
}

void
CoreModule::Terminate()
{
  OSThread::Terminate();
  if( mNeedStopRun )
    StopRunFilters();
  GenericFilter::HaltFilters();
  Environment::OnExit();
}

// Internal functions.
bool
CoreModule::IsLastModule() const
{
#if IS_LAST_MODULE
  return true;
#else
  return false;
#endif
}

bool
CoreModule::IsLocalConnection( const client_tcpsocket& s ) const
{
  return s.ip() == mThisModuleIP;
}

bool
CoreModule::Initialize( int& ioArgc, char** ioArgv )
{
  // Make sure there is only one instance of each module running at a time.
  if( !ProcessUtils::AssertSingleInstance( ioArgc, ioArgv, THISMODULE "Module", 5000 ) )
  {
    BCIERR << "Another " THISMODULE " Module is currently running.\n"
           << "Only one instance of each module may run at a time."
           << endl;
    return false;
  }
  ProcessUtils::GoIdle();

  OnInitialize( ioArgc, ioArgv );

  string operatorAddress;
  bool   executeTests = false,
         printVersion = false,
         printHelp = false;
  int i = 1;
  while( i < ioArgc )
  {
    string curArg( ioArgv[ i ] );
    if( curArg == "--version" || curArg == "-v" )
    {
      printVersion = true;
    }
    else if( curArg == "--help" || curArg == "-?" || curArg == "?" )
    {
      printHelp = true;
    }
    else if( curArg == "--local" )
    {
      if( !mParamlist.Exists( THISMODULE "IP" ) )
        mParamlist.Add( "% string " THISMODULE "IP= 127.0.0.1 // " );
    }
    else if( curArg.find( "--" ) == 0 )
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

  if( printVersion )
  {
    const VersionInfo& info = VersionInfo::Current;
    bciout__ << "BCI2000 " << sModuleName << " \n\n"
             << " Framework: " << info[VersionInfo::VersionID] << " \n";
    if( !info[VersionInfo::Revision].empty() )
      bciout__ << " Revision: " << info[VersionInfo::Revision]
               <<          ", " << info[VersionInfo::SourceDate] << " \n";
    if( !info[VersionInfo::Config].empty() )
      bciout__ << " Config: " << info[VersionInfo::Config] << " \n";
    bciout__ << " Build: " << info[VersionInfo::Build] << endl;
    return true;
  }
  if( printHelp )
  {
    string executableName = FileUtils::ExtractBase( FileUtils::ExecutablePath() );
    bciout__ << "Usage:\n"
             << executableName << " <address>:<port> --<option>=<value>\n"
             << " address:\tIP address of operator module (default 127.0.0.1)\n"
             << " port:   \toperator port (default " THISOPPORT ")\n"
             << "Options will appear as parameters in the System section."
             << "Specifying --version will display version information."
             << endl;
    return false;
  }

  if( mParamlist.Exists( "OperatorIP" ) )
    operatorAddress = mParamlist["OperatorIP"].Value().ToString();

  if( operatorAddress.empty() )
    operatorAddress = "127.0.0.1";
  if( operatorAddress.find( ":" ) == string::npos )
    operatorAddress += ":" THISOPPORT;

  InitializeOperatorConnection( operatorAddress );
  bool success = ( bcierr__.Flushes() == 0 );
  if( success )
    OSThread::Start();
  return success;
}


int
CoreModule::OnExecute()
{
  const int cSocketTimeout = 105;
  while( !IsTerminating() )
  {
    streamsock::set_of_instances inputSockets;
    {
      Lock lock( mOperator );
      if( mOperator.is_open() )
        inputSockets.insert( &mOperatorSocket );
    }
    {
      Lock lock( mPreviousModule );
      if( mPreviousModule.is_open() )
        inputSockets.insert( &mPreviousModuleSocket );
    }
    if( !inputSockets.empty() )
      streamsock::wait_for_read( inputSockets, cSocketTimeout );
    {
      Lock lock( mOperator );
      while( mOperator && mOperator.is_open() && mOperator.rdbuf()->in_avail() )
        mMessageQueue.QueueMessage( mOperator );
    }
    {
      Lock lock( mPreviousModule );
      while( mPreviousModule && mPreviousModule.is_open() && mPreviousModule.rdbuf()->in_avail() )
        mMessageQueue.QueueMessage( mPreviousModule );
    }
    mMessageEvent.Set();
  }
  return 0;
}

// This function contains the main event handling loop.
// It will be entered once when the program starts,
// and only be left when the program exits.
void
CoreModule::MainMessageLoop()
{
  const int bciMessageTimeout = 100; // ms, is max time a GUI event needs to wait
                                     // before it gets processed
  while( !IsTerminating() )
  {
    mMessageEvent.Wait( bciMessageTimeout );
    mMessageEvent.Reset();
    ProcessBCIAndGUIMessages();
    Lock lock( mOperator );
    if( !mOperator.is_open() )
      Terminate();
  }
}

void
CoreModule::ProcessBCIAndGUIMessages()
{
  bool repeat = true;
  do
  {
    while( !mMessageQueue.Empty() )
      MessageHandler::HandleMessage( mMessageQueue );
    repeat = false;
    if( mActiveResting )
      ProcessFilters();
    repeat |= mActiveResting;
    // Allow for the GUI to process messages from its message queue if there are any.
    OnProcessGUIMessages();
    repeat |= OnGUIMessagesPending();
    repeat &= mOperator.is_open();
  } while( repeat );
}


void
CoreModule::InitializeOperatorConnection( const string& inOperatorAddress )
{
  Lock lock( mOperator );
  // creating connection to the operator
  mOperatorSocket.open( inOperatorAddress.c_str() );
  if( !mOperatorSocket.is_open() )
  { // wait if connection to operator module fails
    const int operatorWaitInterval = 2000; // ms
    OSThread::Sleep( operatorWaitInterval );
    mOperatorSocket.open( inOperatorAddress.c_str() );
  }
  mOperator.clear();
  mOperator.open( mOperatorSocket );
  if( !mOperator.is_open() )
  {
    BCIERR << "Could not connect to operator module" << endl;
    return;
  }
  if( mOperatorSocket.wait_for_read( 500 ) )
  { // By immediately sending a neutral message, the operator module indicates that it
    // is able to deal with more recent versions of the protocol.
    MessageHandler::HandleMessage( mOperator );
    MessageHandler::PutMessage( mOperator, ProtocolVersion::Current() );
  }
  else
  {
    MessageHandler::PutMessage( mOperator, ProtocolVersion::Current().Major() );
  }
  BCIStream::SetOperatorStream( &mOperator, &mOperator );
  GenericVisualization::SetOutputStream( &mOperator, &mOperator );
  mParamlist.Add(
    "System:Core%20Connections string OperatorIP= x"
    " 127.0.0.1 % % // the Operator module's IP" );
  mParamlist["OperatorIP"].Value() = mOperatorSocket.ip();

  if( mParamlist.Exists( THISMODULE "IP" ) )
    mPreviousModuleSocket.open( mParamlist[ THISMODULE "IP" ].Value().c_str() );
  else
  {
    streamsock::set_of_addresses addr = streamsock::local_addresses();
    mPreviousModuleSocket.open( addr.rbegin()->c_str() );
  }
  mThisModuleIP = mPreviousModuleSocket.ip();
  mPreviousModule.clear();
  mPreviousModule.open( mPreviousModuleSocket );

  EnvironmentBase::EnterConstructionPhase( &mParamlist, &mStatelist, NULL );
  GenericFilter::InstantiateFilters();
  EnvironmentBase::EnterNonaccessPhase();
  if( bcierr__.Flushes() > 0 )
    return;

  { // add parameters for socket connection
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
  }
  { // Version info
    const VersionInfo& info = VersionInfo::Current;
    mParamlist.Add(
      "System:Configuration matrix " THISMODULE "Version= "
        "{ Framework Revision Build Config } 1 "
        " % % % // " THISMODULE " version information" );
    Param& p = mParamlist[THISMODULE "Version"];
    p.Value( "Framework" ) = info[VersionInfo::VersionID];
    p.Value( "Revision" ) = info[VersionInfo::Revision] + ", " +  info[VersionInfo::SourceDate];
    p.Value( "Build" ) = info[VersionInfo::Build];
    p.Value( "Config" ) = info[VersionInfo::Config];
  }

  { // Filter chain documentation
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
  }
  { // Filter directory documentation
    mParamlist.Add( "System:Configuration matrix Filters= 0 1 % % % // Filter Directory" );
    AppendFilterDirectory( mParamlist["Filters"] );
  }
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
  string nextModuleAddress, address = "NextModuleAddress";
  if( mNextModuleInfo.Exists( address ) )
    nextModuleAddress = mNextModuleInfo[address].Value();
  else
  {
    string ip = NEXTMODULE "IP";
    if( mParamlist.Exists( ip ) )
    {
      ip = mParamlist[ip].Value();
      string port = NEXTMODULE "Port";
      if( mParamlist.Exists( port ) )
      {
        port = mParamlist[port].Value();
        nextModuleAddress = ip + ":" + port;
      }
    }
  }
  if( nextModuleAddress.empty() )
  {
    BCIERR << "Next module's IP/Port parameters not available";
    return;
  }
  {
    Lock lock( mNextModule );
    mNextModuleSocket.open( nextModuleAddress.c_str() );
    mNextModule.open( mNextModuleSocket );
    if( !mNextModule.is_open() )
    {
      BCIERR << "Could not make a connection to the next module at " << nextModuleAddress;
      return;
    }
  }
  OSThread::Sleep( 0 ); // This prevents mysterious closing of the mNextModuleSocket.

  {
    Lock lock( mPreviousModule );
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
}


void
CoreModule::ShutdownSystem()
{
  BCIError::SetOperatorStream( NULL, NULL );
  GenericVisualization::SetOutputStream( NULL, NULL );
  mOperatorSocket.close();
  mPreviousModuleSocket.close();
  mNextModuleSocket.close();
  GenericFilter::DisposeFilters();
}

void
CoreModule::InitializeStatevector()
{
  int sampleBlockSize = ::atoi( mParamlist["SampleBlockSize"].Value().c_str() );
  if( sampleBlockSize > 0 && sampleBlockSize + 1 != mStatevector.Samples() )
  {
    // The state vector holds an additional sample which is used to initialize
    // the subsequent state vector at the beginning of a new block.
    mStatevector = StateVector( mStatelist, sampleBlockSize + 1 );
    mInitialStatevector = mStatevector;
    mStatevector.CommitStateChanges();
  }
}

void
CoreModule::ResetStatevector()
{
  State::ValueType sourceTime = mStatevector.StateValue( "SourceTime" );
  mStatevector = mInitialStatevector;
  mStatevector.SetStateValue( "SourceTime", sourceTime );
  mStatevector.CommitStateChanges();
}

void
CoreModule::AppendFilterDirectory( Param& p ) const
{
  struct AppendFilter
  {
    bool operator()( const Directory::Node* pNode )
    {
      if( pNode->Children().empty() )
      {
        int idx = param.NumRows();
        param.SetNumRows( idx + 1 );
        param.Value( idx, 0 ) = pNode->Path();
      }
      return true;
    }
    Param& param;
  } append = { p };
  Directory::Traverse( GenericFilter::Directory(), append );
}

void
CoreModule::InitializeFilterChain( const SignalProperties& Input )
{
  bcierr__.Clear();
  GenericFilter::HaltFilters();
  InitializeInputSignal( Input );

  // Does the Operator module accept the AutoConfig protocol?
  if( mParamlist.Exists( "AutoConfig" ) )
    mAutoConfig = ::atoi( mParamlist["AutoConfig"].Value().c_str() );

  AutoConfigFilters();
  if( bcierr__.Empty() )
    InitializeFilters();
}

void
CoreModule::InitializeInputSignal( const SignalProperties& Input )
{
#ifdef TODO
# error The inputPropertiesFixed variable may be removed once property messages contain an UpdateRate field.
#endif // TODO
  SignalProperties inputFixed( Input );
  if( !Input.IsEmpty() )
  {
    MeasurementUnits::Initialize( mParamlist );
    inputFixed.SetUpdateRate( 1.0 / MeasurementUnits::SampleBlockDuration() );
  }
  mInputSignal = GenericSignal( inputFixed );
  mActiveResting = Input.IsEmpty();
}

void
CoreModule::AutoConfigFilters()
{
  for( int i = 0; i < mParamlist.Size(); ++i )
    mParamlist[i].Unchanged();

  ParamList restoreParams;
  if( !mAutoConfig )
  {
    restoreParams = mParamlist;
    if( restoreParams.Exists( "SubjectRun" ) )
      restoreParams.Delete( "SubjectRun" ); // special case for backward compatibility reasons
  }

  EnvironmentBase::EnterPreflightPhase( &mParamlist, &mStatelist, &mStatevector );
  SignalProperties Output( 0, 0 );
  GenericFilter::PreflightFilters( mInputSignal.Properties(), Output );
  EnvironmentBase::EnterNonaccessPhase();
  mOutputSignal = GenericSignal( Output );

  client_tcpsocket& outputSocket = IsLastModule() ? mOperatorSocket : mNextModuleSocket;
  ProtocolVersion& outputProtocol = IsLastModule() ? mOperatorProtocol : mNextModuleProtocol;
  if( IsLocalConnection( outputSocket ) && outputProtocol.Provides( ProtocolVersion::SharedSignalStorage ) )
    mOutputSignal.ShareAcrossModules();

  for( int i = 0; i < restoreParams.Size(); ++i )
  {
    const Param& r = restoreParams[i];
    Param& p = mParamlist[r.Name()];
    if( p.Changed() )
    {
      p = r;
      p.Unchanged();
    }
  }
  if( mParamlist.Exists( "DebugLevel" ) && ::atoi( mParamlist["DebugLevel"].Value().c_str() ) )
  {
    for( int i = 0; i < mParamlist.Size(); ++i )
      if( mParamlist[i].Changed() )
        bciout << "AutoConfig: " << mParamlist[i];
  }
  if( mAutoConfig && bcierr__.Empty() )
  {
    BroadcastParameterChanges();
    Lock lock( mOperator );
    if( !MessageHandler::PutMessage( mOperator, Output ) )
      BCIERR << "Could not send output properties to Operator module" << endl;
  }
}

void
CoreModule::InitializeFilters()
{
  if( bcierr__.Empty() )
  {
    if( mParamlist.Exists( "OperatorBackLink" ) )
      mOperatorBackLink = ::atoi( mParamlist["OperatorBackLink"].Value().c_str() );
    if( !mAutoConfig )
    {
      if( IsLastModule() )
      {
        Lock lock( mOperator );
        if( mOperatorBackLink && !MessageHandler::PutMessage( mOperator, mOutputSignal.Properties() ) )
          BCIERR << "Could not send output properties to Operator module" << endl;
      }
      else
      {
        Lock lock( mNextModule );
        if( !MessageHandler::PutMessage( mNextModule, mOutputSignal.Properties() ) )
          BCIERR << "Could not send output properties to " NEXTMODULE " module" << endl;
      }
    }
    EnvironmentBase::EnterInitializationPhase( &mParamlist, &mStatelist, &mStatevector );
    GenericFilter::InitializeFilters();
    EnvironmentBase::EnterNonaccessPhase();
  }
  {
    Lock lock( mPreviousModule );
    if( !mPreviousModule.is_open() )
      BCIERR << PREVMODULE " dropped connection unexpectedly" << endl;
  }
  if( bcierr__.Empty() )
  {
    Lock lock( mOperator );
    MessageHandler::PutMessage( mOperator, Status( THISMODULE " initialized", Status::firstInitializedMessage + MODTYPE - 1 ) );
    mFiltersInitialized = true;
  }
}


void
CoreModule::StartRunFilters()
{
  mStartRunPending = false;
  mActiveResting = false;
  EnvironmentBase::EnterStartRunPhase( &mParamlist, &mStatelist, &mStatevector );
  GenericFilter::StartRunFilters();
  EnvironmentBase::EnterNonaccessPhase();
  mNeedStopRun = true;
  if( bcierr__.Empty() )
  {
    Lock lock( mOperator );
    MessageHandler::PutMessage( mOperator, Status( THISMODULE " running", Status::firstRunningMessage + 2 * ( MODTYPE - 1 ) ) );
  }
}


void
CoreModule::StopRunFilters()
{
  mStopRunPending = false;
  EnvironmentBase::EnterStopRunPhase( &mParamlist, &mStatelist, &mStatevector );
  GenericFilter::StopRunFilters();
  EnvironmentBase::EnterNonaccessPhase();
  mNeedStopRun = false;
  ResetStatevector();
  if( bcierr__.Empty() && !OSThread::IsTerminating() )
  {
    BroadcastParameterChanges();

    Lock lock( mOperator );
    if( mInputSignal.Properties().IsEmpty() )
      MessageHandler::PutMessage( mOperator, SysCommand::Suspend );
    MessageHandler::PutMessage( mOperator, Status( THISMODULE " suspended", Status::firstSuspendedMessage + 2 * ( MODTYPE - 1 ) ) );
  }
  EnvironmentBase::EnterRestingPhase( &mParamlist, &mStatelist, &mStatevector );
  GenericFilter::RestingFilters();
  EnvironmentBase::EnterNonaccessPhase();
  mActiveResting = mInputSignal.Properties().IsEmpty();
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
    Lock lock( mOperator );
    bool success = MessageHandler::PutMessage( mOperator, changedParameters )
                && MessageHandler::PutMessage( mOperator, SysCommand::EndOfParameter );
    if( !success )
      BCIERR << "Could not publish changed parameters" << endl;
  }
}

void
CoreModule::ProcessFilters()
{
  bool wasRunning = mRunning;
  StateUpdate();
  if( mStartRunPending )
    StartRunFilters();
  EnvironmentBase::EnterProcessingPhase( &mParamlist, &mStatelist, &mStatevector );
  GenericFilter::ProcessFilters( mInputSignal, mOutputSignal, !( mRunning || wasRunning ) );
  EnvironmentBase::EnterNonaccessPhase();
  if( bcierr__.Empty() && ( mRunning || wasRunning ) )
    SendOutput();
  if( bcierr__.Empty() && mStopRunPending )
    StopRunFilters();
  if( !bcierr__.Empty() )
    Terminate();
}

void
CoreModule::SendOutput()
{
  if( IsLastModule() && mOperatorBackLink )
  {
    Lock lock( mOperator );
    MessageHandler::PutMessage( mOperator, mStatevector );
    MessageHandler::PutMessage( mOperator, mOutputSignal );
  }
  {
    Lock lock( mNextModule );
    MessageHandler::PutMessage( mNextModule, mStatevector );
    if( !IsLastModule() )
      MessageHandler::PutMessage( mNextModule, mOutputSignal );
  }
}

void
CoreModule::StateUpdate()
{
  mStatevector.CommitStateChanges();
  bool running = mStatevector.StateValue( "Running" );
  mStopRunPending |= mRunning && !running;
  mStartRunPending |= running && !mRunning;
  mRunning = running;
}

bool
CoreModule::HandleParam( istream& is )
{
  if( mRunning )
    BCIERR << "Unexpected Param message" << endl;

  ParamList& list = mReceivingNextModuleInfo ? mNextModuleInfo : mParamlist;
  Param p;
  if( p.ReadBinary( is ) )
    list[p.Name()] = p;
  return is ? true : false;
}


bool
CoreModule::HandleState( istream& is )
{
  State s;
  if( s.ReadBinary( is ) )
  {
    if( mStatevector.Length() > 0 )
    {
      // Changing a state's value via mStatevector.PostStateChange()
      // will buffer the change, and postpone it until the next call to
      // mStatevector.CommitStateChanges(). That call happens
      // after arrival of a StateVector message to make sure that
      // changes are not overwritten with the content of the previous
      // state vector when it arrives from the application module.
      mStatevector.PostStateChange( s.Name(), s.Value() );
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
      bcierr << "Unexpected VisSignal message";
    else
      ProcessFilters();
  }
  return is ? true : false;
}


bool
CoreModule::HandleVisSignalProperties( istream& is )
{
  VisSignalProperties s;
  if( s.ReadBinary( is ) && s.SourceID().empty() )
  {
    if( !mFiltersInitialized )
      InitializeFilterChain( s.SignalProperties() );
  }
  return is ? true : false;
}


bool
CoreModule::HandleStateVector( istream& is )
{
  mStatevector.ReadBinary( is );
  if( mInputSignal.Properties().IsEmpty() )
    ProcessFilters();
  return is ? true : false;
}

bool
CoreModule::HandleSysCommand( istream& is )
{
  SysCommand s;
  if( s.ReadBinary( is ) )
  {
    if( s == SysCommand::EndOfState )
    {
      InitializeCoreConnections();
    }
    else if( s == SysCommand::EndOfParameter )
    {
      mReceivingNextModuleInfo = false;
      mFiltersInitialized = false;
    }
    else if( s == SysCommand::Start )
    {
      /* do nothing */
    }
    else if( s == SysCommand::Reset )
    {
      Terminate();
    }
    else
    {
      BCIERR << "Unexpected SysCommand" << endl;
    }
  }
  return is ? true : false;
}

bool
CoreModule::HandleProtocolVersion( istream& is )
{
  if( mOperatorProtocol.Provides( ProtocolVersion::NextModuleInfo ) )
  {
    mReceivingNextModuleInfo = true;
    mNextModuleProtocol.ReadBinary( is );
  }
  else
    mOperatorProtocol.ReadBinary( is );
  return is;
}

