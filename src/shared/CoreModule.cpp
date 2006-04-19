////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File:    CoreModule.cpp
//
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
//
// Description: A class that represents functionality common to all BCI2000
//          core modules.
////
// $Log$
// Revision 1.2  2006/04/19 16:17:11  mellinger
// Removed Win32 API calls, introduced virtual functions for generic GUI interfacing.
//
// Revision 1.1  2006/03/30 15:42:32  mellinger
// Initial version.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "CoreModule.h"

#include "UGenericFilter.h"
#include "UGenericSignal.h"
#include "UGenericVisualization.h"
#include "USysCommand.h"
#include "UStatus.h"
#include "MessageHandler.h"
#include "TCPStream.h"
#include "UBCIError.h"
#include "MeasurementUnits.h"

#include <iostream>
#include <string>
#include <sstream>

#ifndef _WIN32
# include <sys/sem.h>
#endif

#define BCIERR (__bcierr << THISMODULE ": ")

using namespace std;

CoreModule::CoreModule()
: mpStatevector( NULL ),
  mTerminated( false ),
  mLastRunning( false ),
  mResting( false ),
  mStartRunPending( false ),
  mStopRunPending( false )
{
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
  if( Initialize( inArgc, inArgv ) )
    MainMessageLoop();
  return ( __bcierr.flushes() == 0 );
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
  bool   printHelp = false;
  int i = 1;
  while( i < inArgc )
  {
    string curArg( inArgv[ i ] );
    if( curArg.find( "--" ) == 0 )
    {
      string paramDef = curArg.substr( 2 );
      size_t nameEnd = paramDef.find_first_of( "-=:" );
      string paramName = paramDef.substr( 0, nameEnd ),
             paramValue;
      if( nameEnd != string::npos )
        paramValue = paramDef.substr( nameEnd + 1 );
      PARAM param( paramName.c_str(),
                   "System:Command Line Arguments",
                   "variant",
                   paramValue.c_str()
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

  if( printHelp )
  {
    string executableName = inArgv[ 0 ];
    size_t pos = executableName.find_last_of( "/\\" );
    if( pos != string::npos )
      executableName = executableName.substr( pos + 1 );

    __bciout << "Usage:\n"
             << executableName << " <address>:<port> --<option>-<value>\n"
             << " address:\tip address of operator module (default 127.0.0.1)\n"
             << " port:   \toperator port (default " THISOPPORT ")\n"
             << "Options will appear as parameters in the System section."
             << endl;
    return false;
  }

  if( operatorAddress.empty() )
    operatorAddress = "127.0.0.1";
  if( operatorAddress.find( ":" ) == string::npos )
    operatorAddress += ":" THISOPPORT;

  InitializeOperatorConnection( operatorAddress );
  return ( __bcierr.flushes() == 0 );
}

// This function contains the main event handling loop.
// It will be entered once when the program starts,
// and only be left when the program quits.
void
CoreModule::MainMessageLoop()
{
  const int bciMessageTimeout = 100; // ms -- the maximum amount of time windows
                                     // messages must wait to be processed. Windows
                                     // messages resulting from BCI2000 messages
                                     // (e.g. WM_PAINT messages) will be processed
                                     // without additional delay.
  try
  {
    while( !mTerminated )
    {
      if( !mResting && !GUIMessagesPending() )
        tcpsocket::wait_for_read( mInputSockets, bciMessageTimeout );
      ProcessBCIAndGUIMessages();
      if( !mOperator.is_open() )
        Terminate();
    }
  }
  catch( const char* s )
  {
    BCIERR << s << ", terminating module"
           << endl;
  }
  catch( const exception& e )
  {
    BCIERR << "caught exception "
           << typeid( e ).name() << " (" << e.what() << "),\n"
           << "terminating module"
           << endl;
  }
#ifdef _BORLANDC_
  catch( const Exception& e )
  {
    BCIERR << "caught exception "
           << e.Message.c_str() << ",\n"
           << "terminating module"
           << endl;
  }
#endif
  ShutdownSystem();
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
    // in pairs of STATEVECTOR and VisSignal messages, so we try processing two
    // of them at once.
    const int maxMsgFromPrevModule = 2;
    for( int i = 0; i < maxMsgFromPrevModule
                    && mPreviousModule && mPreviousModule.rdbuf()->in_avail(); ++i )
      MessageHandler::HandleMessage( mPreviousModule );
    // If there are messsages from the operator, all of them must be processed
    // at once because there are message sequences that count as a single
    // message, e.g. a sequence of PARAM messages followed by a SYSCMD::EndOfParameter
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
CoreModule::InitializeOperatorConnection( const string& inOperatorAddress )
{
  // creating connection to the operator
  mOperatorSocket.open( inOperatorAddress.c_str() );
  mOperator.clear();
  mOperator.open( mOperatorSocket );
  if( !mOperator.is_open() )
  {
    BCIERR << "Could not make a connection to the Operator" << endl;
    return;
  }

  mPreviousModuleSocket.open();
  mPreviousModule.clear();
  mPreviousModule.open( mPreviousModuleSocket );

  Environment::EnterConstructionPhase( &mParamlist, &mStatelist, NULL, &mOperator );
  GenericFilter::InstantiateFilters();
  Environment::EnterNonaccessPhase();

#if( MODTYPE == SIGPROC )
  // Add the NumControlSignals parameter.
  mParamlist.Add(
    "Filtering int NumControlSignals= 2"
    " 1 1 128 // the number of transmitted control signals" );
#endif // SIGPROC

  // add parameters for socket connection
  // my receiving socket port number
  mParamlist.Add(
    "System string " THISMODULE "Port= x"
    " 4200 1024 32768 // the " THISMODULE " module's listening port" );
  ostringstream port;
  port << mPreviousModuleSocket.port();
  mParamlist[ THISMODULE "Port" ].Value() = port.str();
  // and IP address
  mParamlist.Add(
    "System string " THISMODULE "IP= x"
    " 127.0.0.1 127.0.0.1 127.0.0.1 // the " THISMODULE " module's listening IP" );
  mParamlist[ THISMODULE "IP" ].Value() = mPreviousModuleSocket.ip();

  // Version control
  mParamlist.Add(
    "System matrix " THISMODULE "Version= { Framework CVS Build } 1 " THISMODULE " % %"
    " % % % // " THISMODULE " version information" );
  mParamlist[ THISMODULE "Version" ].Value( "Framework" ) = THISVERSION;
  mParamlist[ THISMODULE "Version" ].Value( "CVS" ) = "$Revision$ $Date$";
  mParamlist[ THISMODULE "Version" ].Value( "Build" ) = __DATE__ ", " __TIME__;

  // now, publish all parameters
  MessageHandler::PutMessage( mOperator, mParamlist );
  MessageHandler::PutMessage( mOperator, SYSCMD::EndOfParameter );
  // and the states
  MessageHandler::PutMessage( mOperator, mStatelist );
  MessageHandler::PutMessage( mOperator, SYSCMD::EndOfState );

  MessageHandler::PutMessage( mOperator, STATUS( "Waiting for configuration ...", 100 ) );
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
  string ip = mParamlist[ ipParam ].GetValue(),
         port = mParamlist[ portParam ].GetValue();
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
  mpStatevector=NULL;

  GenericFilter::DisposeFilters();
}


void
CoreModule::ResetStatevector()
{
  // State "Running" is the actual memory for the module's running state,
  // so we may not reset it.
  short running = mpStatevector->GetStateValue( "Running" ),
        sourceTime = mpStatevector->GetStateValue( "SourceTime" ),
        stimulusTime = mpStatevector->GetStateValue( "StimulusTime" );
  istringstream iss( mInitialStatevector );
  mpStatevector->ReadBinary( iss );
  mpStatevector->SetStateValue( "Running", running );
  mpStatevector->SetStateValue( "SourceTime", sourceTime );
  mpStatevector->SetStateValue( "StimulusTime", stimulusTime );
}


void
CoreModule::InitializeFilters()
{
  mStartRunPending = true;
  __bcierr.clear();
  GenericFilter::HaltFilters();
  bool errorOccurred = ( __bcierr.flushes() > 0 );
  int numInputChannels = 0,
      numInputElements = 0;
#if( MODTYPE == EEGSRC )
  numInputChannels = 0;
  numInputElements = 0;
#elif( MODTYPE == SIGPROC )
  if( mParamlist.Exists( "TransmitChList" ) )
    numInputChannels = mParamlist[ "TransmitChList" ].GetNumValues();
  if( mParamlist.Exists( "SampleBlockSize" ) )
    numInputElements = ::atoi( mParamlist[ "SampleBlockSize" ].GetValue() );
#elif( MODTYPE == APP )
  if( mParamlist.Exists( "NumControlSignals" ) )
    numInputChannels = ::atoi( mParamlist[ "NumControlSignals" ].GetValue() );
  else
    numInputChannels = 2;
  numInputElements = 1;
#endif // MODTYPE
  SignalProperties inputProperties = SignalProperties( numInputChannels, numInputElements ),
                   outputProperties( 0, 0 );
  Environment::EnterPreflightPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
  GenericFilter::PreflightFilters( inputProperties, outputProperties );
  Environment::EnterNonaccessPhase();
  errorOccurred |= ( __bcierr.flushes() > 0 );
  if( !errorOccurred )
  {
    mOutputSignal = GenericSignal( outputProperties );
    Environment::EnterInitializationPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
    GenericFilter::InitializeFilters();
    Environment::EnterNonaccessPhase();
    errorOccurred |= ( __bcierr.flushes() > 0 );
  }
  if( !mPreviousModule.is_open() )
  {
    BCIERR << PREVMODULE " dropped connection unexpectedly" << endl;
    errorOccurred = true;
  }
  if( !errorOccurred )
    MessageHandler::PutMessage( mOperator, STATUS( THISMODULE " initialized", 199 + MODTYPE ) );
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

  Environment::EnterStartRunPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
  GenericFilter::StartRunFilters();
  Environment::EnterNonaccessPhase();
  if( __bcierr.flushes() == 0 )
  {
    MessageHandler::PutMessage( mOperator, STATUS( THISMODULE " running", 201 + 2 * MODTYPE ) );
    mResting = false;
  }
}


void
CoreModule::StopRunFilters()
{
  mStopRunPending = false;
  mStartRunPending = true;
  Environment::EnterStopRunPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
  GenericFilter::StopRunFilters();
  Environment::EnterNonaccessPhase();
  if( __bcierr.flushes() == 0 )
  {
#if( MODTYPE == EEGSRC ) // The operator wants an extra invitation from the source module.
    MessageHandler::PutMessage( mOperator, SYSCMD::Suspend );
#endif // EEGSRC
    MessageHandler::PutMessage( mOperator, STATUS( THISMODULE " suspended", 202 + 2 * MODTYPE ) );
    mResting = true;
  }
}


void
CoreModule::ProcessFilters( const GenericSignal* input )
{
  Environment::EnterProcessingPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
  GenericFilter::ProcessFilters( input, &mOutputSignal );
  Environment::EnterNonaccessPhase();
  bool errorOccurred = ( __bcierr.flushes() > 0 );
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
  Environment::EnterRestingPhase( &mParamlist, &mStatelist, mpStatevector, &mOperator );
  GenericFilter::RestingFilters();
  Environment::EnterNonaccessPhase();
  bool errorOccurred = ( __bcierr.flushes() > 0 );
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
#if( MODTYPE != EEGSRC ) // For non-source modules, Resting() is called once
                         // after the Running state drops to 0.
  mResting = false;
#endif // EEGSRC
}


bool
CoreModule::HandlePARAM( istream& is )
{
  if( mpStatevector && mpStatevector->GetStateValue( "Running" ) )
    BCIERR << "Unexpected PARAM message" << endl;

  PARAM p;
  if( p.ReadBinary( is ) )
    mParamlist[ p.GetName() ] = p;
  return is;
}


bool
CoreModule::HandleSTATE( istream& is )
{
  STATE s;
  if( s.ReadBinary( is ) )
  {
    if( mpStatevector )
    {
#if( MODTYPE == EEGSRC )
      // Changing a state's value via mpStatevector->PostStateChange()
      // will buffer the change, and postpone it until the next call to
      // mpStatevector->CommitStateChanges(). That call happens
      // after arrival of a STATEVECTOR message to make sure that
      // changes are not overwritten with the content of the previous
      // state vector when it arrives from the application module.
      mpStatevector->PostStateChange( s.GetName(), s.GetValue() );

      // For the "Running" state, the module will undergo a more complex
      // state transition than for other states.
      if( string( "Running" ) == s.GetName() )
      {
        bool running = mpStatevector->GetStateValue( "Running" ),
             nextRunning = s.GetValue();
        if( !running && nextRunning )
        {
          mLastRunning = true;
          StartRunFilters();
          ProcessFilters( NULL );
        }
      }
#else // EEGSRC
      bcierr << "Unexpectedly received a STATE message" << endl;
#endif // EEGSRC
    }
    else
    {
      mStatelist.Delete( s.GetName() );
      mStatelist.Add( s );
    }
  }
  return is;
}


bool
CoreModule::HandleVisSignal( istream& is )
{
  VisSignal s;
  if( s.ReadBinary( is ) && s.GetSourceID() == 0 )
  {
    if( mStartRunPending )
      StartRunFilters();
    const GenericSignal& inputSignal = s;
    ProcessFilters( &inputSignal );
    if( mStopRunPending )
      StopRunFilters();
  }
  return is;
}


bool
CoreModule::HandleSTATEVECTOR( istream& is )
{
  if( mpStatevector->ReadBinary( is ) )
  {
#if( MODTYPE == EEGSRC )
    mpStatevector->CommitStateChanges();
    bool running = mpStatevector->GetStateValue( "Running" );
    if( running && !mLastRunning )
      StartRunFilters();
    else if( !running && mLastRunning )
      StopRunFilters();
    // The EEG source does not receive a signal, so handling must take place
    // on arrival of a STATEVECTOR message.
    // This distinction could be avoided if the state vector was
    // sent _after_ its associated signal; then, all modules might call
    // Process() from HandleSTATEVECTOR().
    if( mLastRunning ) // For the first "Running" block, Process() is called from
                       // HandleSTATE(), and may not be called here.
                       // For the first "Suspended" block, we need to call
                       // Process() one last time.
                       // By evaluating at "mLastRunning" instead of "running" we
                       // obtain this behavior.
      ProcessFilters( NULL );
#else // EEGSRC
    bool running = mpStatevector->GetStateValue( "Running" );
    if( !running && mLastRunning )
      mStopRunPending = true;
#endif // EEGSRC
    mLastRunning = running;
  }
  return is;
}


bool
CoreModule::HandleSYSCMD( istream& is )
{
  SYSCMD s;
  if( s.ReadBinary( is ) )
  {
    if( s == SYSCMD::EndOfState )
    {
      if( mpStatevector != NULL )
        bcierr << "Unexpected SYSCMD::EndOfState message" << endl;

      delete mpStatevector;
      // Initialize the state vector from the state list.
      mpStatevector = new STATEVECTOR( mStatelist );
      ostringstream oss;
      mpStatevector->WriteBinary( oss );
      mInitialStatevector = oss.str();
      mpStatevector->CommitStateChanges();
      InitializeCoreConnections();
      InitializeFilters();
#if( MODTYPE == EEGSRC )
      mResting = ( __bcierr.flushes() == 0 );
#endif // EEGSRC
    }
    else if( s == SYSCMD::EndOfParameter )
    {
      // This happens for subsequent initializations.
      if( mpStatevector != NULL )
      {
        InitializeFilters();
#if( MODTYPE == EEGSRC )
        mResting = ( __bcierr.flushes() == 0 );
#endif // EEGSRC
      }
    }
    else if( s == SYSCMD::Start )
    {
      /* do nothing */
    }
    else if( s == SYSCMD::Reset )
      Terminate();
    else
      BCIERR << "Unexpected SYSCMD" << endl;
  }
  return is;
}

