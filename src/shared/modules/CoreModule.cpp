////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that represents functionality common to all BCI2000
//          core modules.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
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
#include "BCIEvent.h"
#include "PrecisionTime.h"
#include "VersionInfo.h"
#include "Version.h"

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

CoreModule::CoreModule()
: mpStatevector( NULL ),
  mTerminated( false ),
  mLastRunning( false ),
  mResting( false ),
  mFiltersInitialized( false ),
  mStartRunPending( false ),
  mStopRunPending( false ),
  mBlockDuration( 0 ),
  mSampleBlockSize( 0 )
{
  mOperatorSocket.set_tcpnodelay( true );
  mNextModuleSocket.set_tcpnodelay( true );
  mPreviousModuleSocket.set_tcpnodelay( true );
  BCIEvent::SetEventQueue( mBCIEvents );
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


bool
CoreModule::Run( int inArgc, char** inArgv )
{
  try
  {
    if( Initialize( inArgc, inArgv ) )
      MainMessageLoop();
  }
  catch( const char* s )
  {
    bcierr << s << ", terminating " THISMODULE " module"
           << endl;
  }
  catch( const exception& e )
  {
    bcierr << "unhandled exception "
           << typeid( e ).name() << " (" << e.what() << "),\n"
           << "terminating " THISMODULE " module"
           << endl;
  }
#ifdef __BORLANDC__
  catch( const Exception& e )
  {
    bcierr << "unhandled exception "
           << e.Message.c_str() << ",\n"
           << "terminating " THISMODULE " module"
           << endl;
  }
#endif // __BORLANDC__
  ShutdownSystem();
  return ( bcierr__.Flushes() == 0 );
}

// Internal functions.
bool
CoreModule::Initialize( int inArgc, char** inArgv )
{
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

  mInputSockets.insert( &mOperatorSocket );
  mInputSockets.insert( &mPreviousModuleSocket );

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
  return ( bcierr__.Flushes() == 0 );
}

// This function contains the main event handling loop.
// It will be entered once when the program starts,
// and only be left when the program quits.
void
CoreModule::MainMessageLoop()
{
  const int bciMessageTimeout = 100; // ms -- the maximum amount of time GUI
                                     // messages must wait to be processed. GUI
                                     // messages resulting from BCI2000 messages
                                     // (e.g. WM_PAINT messages) will be processed
                                     // without additional delay.
  while( !mTerminated )
  {
    if( !mResting && !GUIMessagesPending() )
      streamsock::wait_for_read( mInputSockets, bciMessageTimeout );
    ProcessBCIAndGUIMessages();
    if( !mOperator.is_open() )
      Terminate();
  }
}


void
CoreModule::ProcessBCIAndGUIMessages()
{
  while( mPreviousModule && mPreviousModule.rdbuf()->in_avail()
        || mOperator && mOperator.rdbuf()->in_avail()
        || mResting
        || GUIMessagesPending() )
  {
    // If there is a message from the previous module, it has highest priority.
    // For the SignalProcessing and the Application modules, these messages occur
    // in pairs of StateVector and VisSignal messages, so we try processing two
    // of them at once.
    const int maxMsgFromPrevModule = 2;
    for( int i = 0; i < maxMsgFromPrevModule
                    && mPreviousModule && mPreviousModule.rdbuf()->in_avail(); ++i )
      MessageHandler::HandleMessage( mPreviousModule );
    // If there are messsages from the operator, all of them must be processed
    // at once because there are message sequences that count as a single
    // message, e.g. a sequence of Param messages followed by a SYSCMD::EndOfParameter
    // message.
    const int maxMsgFromOperator = 1000;
    for( int i = 0; i < maxMsgFromOperator && mOperator && mOperator.rdbuf()->in_avail(); ++i )
      MessageHandler::HandleMessage( mOperator );
    // The mResting flag is treated as a pending message from the module to itself.
    // For non-source modules, it is cleared from the HandleResting() function
    // much as pending messages are cleared from the stream by the HandleMessage()
    // function.
    if( mResting )
      HandleResting();
    mResting &= mOperator.is_open();
    // Last of all, allow for the GUI to process messages from its message queue if there are any.
    ProcessGUIMessages();
  }
}

void
CoreModule::ProcessBCIEvents()
{
  if( mpStatevector != NULL )
  {
    // When translating event time stamps into sample positions, we assume a block's
    // source time stamp to match the next block's first sample.
    PrecisionTime sourceTime = mpStatevector->StateValue( "SourceTime" );

    while( !mBCIEvents.IsEmpty()
           && PrecisionTime::SignedDiff( mBCIEvents.FrontTimeStamp(), sourceTime ) <= 0 )
    {
      int offset = ( ( mBlockDuration - ( sourceTime - mBCIEvents.FrontTimeStamp() + 1 ) )
                   * mSampleBlockSize ) / mBlockDuration;
      istringstream iss( mBCIEvents.FrontDescriptor() );
      string name;
      iss >> name;
      State::ValueType value;
      iss >> value;
      int duration;
      if( !( iss >> duration ) )
        duration = -1;

      bcidbg( 10 ) << "Setting State \"" << name
                   << "\" to " << value
                   << " at offset " << offset
                   << " with duration " << duration
                   << " from event:\n" << mBCIEvents.FrontDescriptor()
                   << endl;

      offset = max( offset, 0 );
      if( duration < 0 )
      { // No duration given -- set the state at the current and following positions.
        mpStatevector->SetStateValue( name, offset, value );
      }
      else if( duration == 0 )
      { // Set the state at a single position only.
        // For zero duration events, avoid overwriting a previous event by
        // moving the current one if possible, and reposting if not.
        while( offset <= mSampleBlockSize && mpStatevector->StateValue( name, offset ) != 0 )
          ++offset;
        if( offset == mSampleBlockSize )
        { // Re-post the event to be processed in the next block
          mBCIEvents.PushBack( mBCIEvents.FrontDescriptor(), mBCIEvents.FrontTimeStamp() );
        }
        else
        {
          mpStatevector->SetStateValue( name, offset, value );
          mpStatevector->SetStateValue( name, offset + 1, 0 );
        }
      }
      else
      {
        bcierr__ << "Event durations > 0 are currently unsupported "
                 << "(" << iss.str() << ")"
                 << endl;
      }
      mBCIEvents.PopFront();
    }
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

  if( mParamlist.Exists( THISMODULE "IP" ) )
    mPreviousModuleSocket.open( mParamlist[ THISMODULE "IP" ].Value().c_str() );
  else
  {
    streamsock::set_of_addresses addr = streamsock::local_addresses();
    mPreviousModuleSocket.open( addr.rbegin()->c_str() );
  }
  mPreviousModule.clear();
  mPreviousModule.open( mPreviousModuleSocket );

  EnvironmentBase::EnterConstructionPhase( &mParamlist, &mStatelist, NULL, &mOperator );
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

  mPreviousModule.close();
  mPreviousModule.clear();
  if( mPreviousModuleSocket.is_open() && !mPreviousModuleSocket.connected() )
    mPreviousModuleSocket.wait_for_read( cInitialConnectionTimeout, true );
  mPreviousModule.open( mPreviousModuleSocket );
  if( !mPreviousModule.is_open() )
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
    ( *mpStatevector )( i ).ReadBinary( istringstream( mInitialStatevector ) );
  mpStatevector->SetStateValue( "SourceTime", sourceTime );
  mpStatevector->SetStateValue( "StimulusTime", stimulusTime );
}


void
CoreModule::InitializeFilters( const SignalProperties& inputProperties )
{
  mStartRunPending = true;
  bcierr__.clear();
  GenericFilter::HaltFilters();
  bool errorOccurred = ( bcierr__.Flushes() > 0 );
  SignalProperties outputProperties( 0, 0 );
  EnvironmentBase::EnterPreflightPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
  GenericFilter::PreflightFilters( inputProperties, outputProperties );
  EnvironmentBase::EnterNonaccessPhase();
  errorOccurred |= ( bcierr__.Flushes() > 0 );
  if( !errorOccurred )
  {
    mBlockDuration = 1.0 / MeasurementUnits::ReadAsTime( "1ms" );
#if( MODTYPE != APP )
    MessageHandler::PutMessage( mNextModule, outputProperties );
#endif // APP
    mOutputSignal = GenericSignal( outputProperties );
    EnvironmentBase::EnterInitializationPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
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
  // The first state vector written to disk is not the one
  // received in response to the first EEG data block. Without resetting it
  // to its initial value,
  // there would be state information from the end of the (possibly
  // interrupted) previous run written with the first EEG data block.
  ResetStatevector();
  mpStatevector->SetStateValue( "Running", 1 );

  EnvironmentBase::EnterStartRunPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
  GenericFilter::StartRunFilters();
  EnvironmentBase::EnterNonaccessPhase();
  if( bcierr__.Flushes() == 0 )
  {
    MessageHandler::PutMessage( mOperator, Status( THISMODULE " running", Status::firstRunningMessage + 2 * ( MODTYPE - 1 ) ) );
    mResting = false;
  }
}


void
CoreModule::StopRunFilters()
{
  mStopRunPending = false;
  mStartRunPending = true;
  EnvironmentBase::EnterStopRunPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
  GenericFilter::StopRunFilters();
  EnvironmentBase::EnterNonaccessPhase();
  if( bcierr__.Flushes() == 0 )
  {
#if( MODTYPE == SIGSRC ) // The operator wants an extra invitation from the source module.
    MessageHandler::PutMessage( mOperator, SysCommand::Suspend );
#endif // SIGSRC
    MessageHandler::PutMessage( mOperator, Status( THISMODULE " suspended", Status::firstSuspendedMessage + 2 * ( MODTYPE - 1 ) ) );
    mResting = true;
  }
}


void
CoreModule::ProcessFilters( const GenericSignal& input )
{
  EnvironmentBase::EnterProcessingPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
  GenericFilter::ProcessFilters( input, mOutputSignal );
  EnvironmentBase::EnterNonaccessPhase();
  bool errorOccurred = ( bcierr__.Flushes() > 0 );
  if( errorOccurred )
  {
    Terminate();
    return;
  }
  MessageHandler::PutMessage( mNextModule, *mpStatevector );
#if( MODTYPE != APP )
  MessageHandler::PutMessage( mNextModule, mOutputSignal );
#endif // APP
}


void
CoreModule::RestingFilters()
{
  EnvironmentBase::EnterRestingPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
  GenericFilter::RestingFilters();
  EnvironmentBase::EnterNonaccessPhase();
  bool errorOccurred = ( bcierr__.Flushes() > 0 );
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
  return is;
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
          static GenericSignal nullSignal( 0, 0 );
          ProcessFilters( nullSignal );
        }
      }
#else // SIGSRC
      bcierr << "Unexpectedly received a State message" << endl;
#endif // SIGSRC
    }
    else
    {
      mStatelist.Delete( s.Name() );
      mStatelist.Add( s );
    }
  }
  return is;
}


bool
CoreModule::HandleVisSignal( istream& is )
{
  VisSignal s;
  if( s.ReadBinary( is ) && s.SourceID() == 0 )
  {
    const GenericSignal& inputSignal = s;
    if( !mFiltersInitialized )
      bcierr << "Unexpected VisSignal message" << endl;
    else
    {
      if( mStartRunPending )
        StartRunFilters();
      ProcessFilters( inputSignal );
      if( mStopRunPending )
        StopRunFilters();
    }
  }
  return is;
}


bool
CoreModule::HandleVisSignalProperties( istream& is )
{
  VisSignalProperties s;
  if( s.ReadBinary( is ) && s.SourceID() == 0 )
    if( !mFiltersInitialized )
      InitializeFilters( s );
  return is;
}


bool
CoreModule::HandleStateVector( istream& is )
{
  if( mpStatevector->ReadBinary( is ) )
  {
#if( MODTYPE == SIGSRC )
    mpStatevector->CommitStateChanges();
    bool running = mpStatevector->StateValue( "Running" );
    if( running && !mLastRunning )
      StartRunFilters();
    // The source module does not receive a signal, so handling must take place
    // on arrival of a StateVector message.
    if( mLastRunning ) // For the first "Running" block, Process() is called from
                       // HandleState(), and may not be called here.
                       // For the first "Suspended" block, we need to call
                       // Process() one last time.
                       // By evaluating at "mLastRunning" instead of "running" we
                       // obtain this behavior.
    {
      ProcessBCIEvents();
      static GenericSignal nullSignal( 0, 0 );
      ProcessFilters( nullSignal );
    }
    if( !running && mLastRunning )
      StopRunFilters();
#else // SIGSRC
    bool running = mpStatevector->StateValue( "Running" );
    if( !running && mLastRunning )
      mStopRunPending = true;
#endif // SIGSRC
    mLastRunning = running;
  }
  return is;
}


bool
CoreModule::HandleSysCommand( istream& is )
{
  SysCommand s;
  if( s.ReadBinary( is ) )
  {
    int sampleBlockSize = 1;
    if( mParamlist.Exists( "SampleBlockSize" ) )
      sampleBlockSize = ::atoi( mParamlist[ "SampleBlockSize" ].Value().c_str() );
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
  return is;
}


