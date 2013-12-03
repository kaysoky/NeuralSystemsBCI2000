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

#undef bcierr
#define bcierr (bcierr_(sModuleName))

using namespace std;

static const string sModuleName = FileUtils::ExtractBase( FileUtils::ExecutablePath() );

// ModuleConnection class
ModuleConnection::ModuleConnection( CoreModule& parent )
: MessageChannel( static_cast<iostream&>( *this ) ),
  iostream( &mBuffer ),
  mrParent( parent ),
  mIsLocal( false )
{
  SetOutputLock( &mOutputLock );
}

ModuleConnection::~ModuleConnection()
{
}

bool
ModuleConnection::Open( streamsock& s )
{
  switch( s.connected() )
  {
    case streamsock::locally:
      mIsLocal = true;
      break;
    case streamsock::remote:
      mIsLocal = false;
      break;
    default:
      return false;
  }
  iostream::clear();
  mBuffer.open( s );
  return true;
}

bool
ModuleConnection::ProcessMessages()
{
  if( !CanRead().Wait( 0 ) )
    return false;
  while( mBuffer.is_open() && mBuffer.in_avail() > 0 )
  {
    if( mBuffer.is_open() && istream::peek() == EOF )
      istream::clear();
    if( istream::good() && mBuffer.is_open() )
      MessageChannel::HandleMessage();
  }
  istream::clear();
  return true;
}

bool ModuleConnection::OnProtocolVersion( istream& s )
{ return mrParent.HandleProtocolVersion( s ); }
bool ModuleConnection::OnParam( std::istream& s )
{ return mrParent.HandleParam( s ); }
bool ModuleConnection::OnState( std::istream& s )
{ return mrParent.HandleState( s ); }
bool ModuleConnection::OnVisSignal( std::istream& s )
{ return mrParent.HandleVisSignal( s ); }
bool ModuleConnection::OnVisSignalProperties( std::istream& s )
{ return mrParent.HandleVisSignalProperties( s ); }
bool ModuleConnection::OnStateVector( std::istream& s )
{ return mrParent.HandleStateVector( s ); }
bool ModuleConnection::OnSysCommand( std::istream& s )
{ return mrParent.HandleSysCommand( s ); }
bool ModuleConnection::OnSend( const VisSignalConst& s )
{ return mrParent.OnSendSignal( &s.Signal(), *this ); }
bool ModuleConnection::OnSend( const VisSignal& s )
{ return OnSend( static_cast<const VisSignalConst&>( s ) ); }

// CoreModule class
CoreModule::CoreModule()
: mFiltersInitialized( false ),
  mTerminating( false ),
  mRunning( false ),
  mActiveResting( false ),
  mStartRunPending( false ),
  mStopRunPending( false ),
  mNeedStopRun( false ),
  mReceivingNextModuleInfo( false ),
  mGlobalID( NULL ),
  mOperatorBackLink( false ),
  mAutoConfig( false ),
  mOperator( *this ),
  mPreviousModule( *this ),
  mNextModule( *this )
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
  ShutdownSystem();
  return ( bcierr__.Flushes() == 0 );
}

void
CoreModule::Terminate()
{
  mTerminating = true;
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
CoreModule::Initialize( int& ioArgc, char** ioArgv )
{
  // Make sure there is only one instance of each module running at a time.
  if( !ProcessUtils::AssertSingleInstance( ioArgc, ioArgv, THISMODULE "Module", 5000 ) )
  {
    bcierr << "Another " THISMODULE " Module is currently running.\n"
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
  return ( bcierr__.Flushes() == 0 );
}

// This function contains the main event handling loop.
// It will be entered once when the program starts,
// and only be left when the program exits.
void
CoreModule::MainMessageLoop()
{
  const int bciMessageTimeout = 100; // ms, is max time a GUI event needs to wait
                                     // before it gets processed
  Waitables inputs;
  inputs.Add( mOperator.CanRead() )
        .Add( mPreviousModule.CanRead() );

  while( !mTerminating )
  {
    inputs.Wait( bciMessageTimeout );
    ProcessBCIAndGUIMessages();
    if( !mOperator.IsOpen() )
      Terminate();
  }
}

void
CoreModule::ProcessBCIAndGUIMessages()
{
  bool repeat = true;
  while( repeat )
  {
    mOperator.ProcessMessages();
    mPreviousModule.ProcessMessages();

    repeat = false;
    if( mActiveResting && !mTerminating )
    {
      ProcessFilters();
      mActiveResting &= bcierr__.Empty();
      repeat |= mActiveResting;
    }
    // Allow for the GUI to process messages from its message queue if there are any.
    OnProcessGUIMessages();
    repeat |= OnGUIMessagesPending();
    repeat &= mOperator.IsOpen();
  }
}

void
CoreModule::InitializeOperatorConnection( const string& inOperatorAddress )
{
  // creating connection to the operator
  mOperatorSocket.open( inOperatorAddress.c_str() );
  if( !mOperatorSocket.is_open() )
  { // wait if connection to operator module fails
    const int operatorWaitInterval = 2000; // ms
    ThreadUtils::SleepFor( operatorWaitInterval );
    mOperatorSocket.open( inOperatorAddress.c_str() );
  }
  mOperator.Open( mOperatorSocket );
  if( !mOperator.IsOpen() )
  {
    bcierr << "Could not connect to operator module" << endl;
    return;
  }
  if( mOperatorSocket.wait_for_read( 500 ) )
  { // By immediately sending a neutral message, the operator module indicates that it
    // is able to deal with more recent versions of the protocol.
    mOperator.ProcessMessages();
    mOperator.Send( ProtocolVersion::Current() );
  }
  else
  {
    mOperator.Send( ProtocolVersion::Current().Major() );
  }
  BCIStream::SetOutputChannel( &mOperator );
  GenericVisualization::SetOutputChannel( &mOperator );
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
  mPreviousModule.Open( mPreviousModuleSocket );

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
  mOperator.Send( mParamlist );
  mOperator.Send( SysCommand::EndOfParameter );
  // ... and states.
  mOperator.Send( mStatelist );
  mOperator.Send( SysCommand::EndOfState );

  mOperator.Send( Status( "Waiting for configuration ...", Status::plainMessage ) );
  mOperator.AsyncSend( true ).AsyncReceive( true );
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
    bcierr << "Next module's IP/Port parameters not available";
    return;
  }
  mNextModuleSocket.open( nextModuleAddress.c_str() );
  mNextModule.Open( mNextModuleSocket );
  if( !mNextModule.IsOpen() )
  {
    bcierr << "Could not make a connection to the next module at " << nextModuleAddress;
    return;
  }
  mNextModule.AsyncSend( false );

  mPreviousModule.Close();
  bool waitForConnection = mPreviousModuleSocket.is_open() && !mPreviousModuleSocket.connected();
  if( waitForConnection )
    mPreviousModuleSocket.wait_for_read( cInitialConnectionTimeout, true );
  mPreviousModule.Open( mPreviousModuleSocket );
  if( waitForConnection && !mPreviousModule.IsOpen() )
  {
    bcierr << "Connection to previous module timed out after "
           << float( cInitialConnectionTimeout ) / 1e3 << "s"
           << endl;
    return;
  }
  mPreviousModule.AsyncReceive( true );
}


void
CoreModule::ShutdownSystem()
{
  BCIStream::SetOutputChannel( 0 );
  GenericVisualization::SetOutputChannel( 0 );
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

namespace
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
  };
}

void
CoreModule::AppendFilterDirectory( Param& p ) const
{
  AppendFilter append = { p };
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

  ModuleConnection& conn = IsLastModule() ? mOperator : mNextModule;
  if( conn.IsLocal() && conn.Protocol().Provides( ProtocolVersion::SharedSignalStorage ) )
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
    if( !mOperator.Send( Output ) )
      bcierr << "Could not send output properties to Operator module" << endl;
  }
}

void
CoreModule::InitializeFilters()
{
  mLargeSignals.clear();
  if( bcierr__.Empty() )
  {
    if( mParamlist.Exists( "OperatorBackLink" ) )
      mOperatorBackLink = ::atoi( mParamlist["OperatorBackLink"].Value().c_str() );
    if( !mAutoConfig )
    {
      if( IsLastModule() )
      {
        if( mOperatorBackLink && !mOperator.Send( mOutputSignal.Properties() ) )
          bcierr << "Could not send output properties to Operator module" << endl;
      }
      else
      {
        if( !mNextModule.Send( mOutputSignal.Properties() ) )
          bcierr << "Could not send output properties to " NEXTMODULE " module" << endl;
      }
    }
    EnvironmentBase::EnterInitializationPhase( &mParamlist, &mStatelist, &mStatevector );
    GenericFilter::InitializeFilters();
    EnvironmentBase::EnterNonaccessPhase();
  }
  if( !mPreviousModule.IsOpen() )
    bcierr << PREVMODULE " dropped connection unexpectedly" << endl;
  if( bcierr__.Empty() )
  {
    mOperator.Send( Status( THISMODULE " initialized", Status::firstInitializedMessage + MODTYPE - 1 ) );
    mFiltersInitialized = true;
    mActiveResting = mInputSignal.Properties().IsEmpty();
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
    mOperator.Send( Status( THISMODULE " running", Status::firstRunningMessage + 2 * ( MODTYPE - 1 ) ) );
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
  if( bcierr__.Empty() && !mTerminating )
  {
    BroadcastParameterChanges();
    if( mInputSignal.Properties().IsEmpty() )
      mOperator.Send( SysCommand::Suspend );
    mOperator.Send( Status( THISMODULE " suspended", Status::firstSuspendedMessage + 2 * ( MODTYPE - 1 ) ) );
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
    mOperator.Send( changedParameters );
    if( !mOperator.Send( SysCommand::EndOfParameter ) )
      bcierr << "Could not publish changed parameters" << endl;
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
    mOperator.Send( mStatevector );
    mOperator.Send( mOutputSignal );
  }
  mNextModule.Send( mStatevector );
  if( !IsLastModule() )
    mNextModule.Send( mOutputSignal );
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
    bcierr << "Unexpected Param message" << endl;

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
      bcierr << "Unexpected SysCommand" << endl;
    }
  }
  return is ? true : false;
}

bool
CoreModule::HandleProtocolVersion( istream& is )
{
  if( mOperator.Protocol().Provides( ProtocolVersion::NextModuleInfo ) )
  {
    mReceivingNextModuleInfo = true;
    mNextModule.Protocol().ReadBinary( is );
  }
  else
    mOperator.Protocol().ReadBinary( is );
  return is;
}

bool
CoreModule::OnSendSignal( const GenericSignal* pSignal, const ModuleConnection& inConnection )
{
  if( pSignal->Channels() * pSignal->Elements() * sizeof(float) > 2048 )
  {
    if( ++mLargeSignals[pSignal] == 2 )
    {
      bool share = inConnection.IsLocal();
      share &= inConnection.Protocol().Provides( ProtocolVersion::SharedSignalStorage );
      if( share )
        const_cast<GenericSignal*>( pSignal )->ShareAcrossModules();
    }
  }
  return true;
}
