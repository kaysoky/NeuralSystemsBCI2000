/******************************************************************************
 * Program:   Core Modules                                                    *
 * Module:    UCoreMain.cpp                                                   *
 * Comment:   The core module framework code for BCI2000                      *
 * Version:   0.30                                                            *
 * Author:    Gerwin Schalk, juergen.mellinger@uni-tuebingen.de               *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.07 - 03/06/2000 - Created Application from generic EEGsource module     *
 * V0.08 - 03/09/2000 - Created Application again from generic EEGsource mod. *
 * V0.09 - 03/16/2000 - Complete communication in between all core modules    *
 * V0.10 - 03/20/2000 - Updated to C++Builder 5.0                             *
 * V0.12 - 03/30/2000 - stable and documented version                         *
 * V0.13 - 04/19/2000 - test version                                          *
 * V0.14 - 05/23/2000 - couple little changes to make it consistent with V0.14*
 * V0.15 - 06/15/2000 - couple little changes to make it consistent with V0.15*
 * V0.16 - 08/07/2000 - couple little changes to make it consistent with V0.16*
 * V0.17 - 08/15/2000 - couple little changes to make it consistent with V0.17*
 * V0.18 - 09/21/2000 - improved socket handling                              *
 * V0.19 - 04/11/2001 - more stable version                                   *
 * V0.19b- 04/11/2001 - CORECOMM (blocking sockets) for core communication    *
 * V0.20 - 04/13/2001 - using coremessages to receive from SignalProcessing   *
 *                      and to send out info to the EEGSource                 *
 * V0.21 - 05/15/2001 - updated user task and added some time stamping        *
 * V0.22 - 06/07/2001 - upgraded to shared libraries V0.17                    *
 * V0.23 - 08/16/2001 - accumulated bug fixes of V0.22x                       *
 * V0.24 - 08/31/2001 - also uses Synchronize() for Task->Initialize          *
 * V0.25 - 10/22/2002 - changed TTask interface to GenericFilter interface, jm*
 * V0.26 - 05/27/2004 - replaced CORECOMM and COREMESSAGE by TCPStream and    *
 *                        MessageHandler classes, jm                          *
 * V0.30 - 05/27/2004 - Removed multi-threading,                              *
 *                      unified code for all modules except operator, jm      *
 *                    - Made sure that only a single instance of each module  *
 *                      type will run at a time, jm                           *
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "UCoreMain.h"

#include "..\shared\defines.h"
#include "UGenericFilter.h"
#include "UGenericSignal.h"
#include "UGenericVisualization.h"
#include "USysCommand.h"
#include "UStatus.h"
#include "MessageHandler.h"
#include "TCPStream.h"
#include "UBCIError.h"
#include "MeasurementUnits.h"

#include <string>
#include <sstream>
#include <assert>

#pragma package(smart_init)
#pragma resource "*.dfm"

#define BCIERR (__bcierr << THISMODULE ": ")

using namespace std;

TfMain *fMain;

__fastcall
TfMain::TfMain( TComponent* Owner )
: TForm( Owner ),
  mpStatevector( NULL ),
  mTerminated( false ),
  mMessageHandler( *this ),
  mLastRunning( false ),
  mResting( false ),
  mMutex( NULL )
{
  // Make sure there is only one instance of each module running at a time.
  // We achieve this by creating a mutex -- if it exists, there is
  // another instance running, and we move that instance to the front and exit.
  const char appTitle[] = THISMODULE " Module";
  mMutex = ::CreateMutex( NULL, TRUE, appTitle );
  if( ::GetLastError() == ERROR_ALREADY_EXISTS )
  {
    Application->Title = "";
    HWND runningApp = ::FindWindow( "TApplication", appTitle );
    if( runningApp != NULL )
      ::SetForegroundWindow( runningApp );
    Application->Terminate();
    return;
  }

  mInputSockets.insert( &mOperatorSocket );
  mInputSockets.insert( &mPreviousModuleSocket );

  Caption = "BCI2000/" THISMODULE " V" THISVERSION;
  Application->Title = appTitle;
  Color = THISCOLOR;
  eOperatorPort->Text = THISOPPORT;

  AnsiString  connectto;

  for( int i = 0; i <= ::ParamCount(); ++i )
  {
    if( ::ParamStr( i ) == "AUTOSTART" )
    {
      if( i + 1 <= ::ParamCount() )
        connectto = ::ParamStr( i + 1 );
      else
        connectto = "127.0.0.1";
      Application->ShowMainForm = false;
      Startup( connectto );
      if( __bcierr.flushes() > 0 )
        Application->Terminate();
    }
  }
}

__fastcall
TfMain::~TfMain( void )
{
  ShutdownSystem();
  delete mpStatevector;
  Terminate();
  if( mMutex != NULL )
  {
    ::ReleaseMutex( mMutex );
    ::CloseHandle( mMutex );
  }
}

bool
TfMain::HandleSTATEVECTOR( istream& is )
{
  if( mpStatevector->ReadBinary( is ) )
  {
#if( MODTYPE == EEGSRC )
    mpStatevector->CommitStateChanges();
#endif // EEGSRC
    bool running = mpStatevector->GetStateValue( "Running" );
    if( running && !mLastRunning )
      StartRunFilters();
    else if( !running && mLastRunning )
      StopRunFilters();
#if( MODTYPE == EEGSRC )
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
#endif // EEGSRC
    mLastRunning = running;
  }
  return is;
}

bool
TfMain::HandleVisSignal( istream& is )
{
  VisSignal s;
  if( s.ReadBinary( is ) && s.GetSourceID() == 0 )
  {
    const GenericSignal& inputSignal = s;
    ProcessFilters( &inputSignal );
  }
  return is;
}

bool
TfMain::HandlePARAM( istream& is )
{
  if( mpStatevector && mpStatevector->GetStateValue( "Running" ) )
    BCIERR << "Unexpected PARAM message" << endl;

  PARAM p;
  if( p.ReadBinary( is ) )
    mParamlist[ p.GetName() ] = p;
  return is;
}

bool
TfMain::HandleSTATE( istream& is )
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
      mStatelist.AddState2List( &s );
  }
  return is;
}

bool
TfMain::HandleSYSCMD( istream& is )
{
  SYSCMD s;
  if( s.ReadBinary( is ) )
  {
    if( s == SYSCMD::EndOfState )
    {
      if( mpStatevector != NULL )
        bcierr << "Unexpected SYSCMD::EndOfState message" << endl;

      delete mpStatevector;
      // This is the first initialization.
      mpStatevector = new STATEVECTOR( &mStatelist );
      ostringstream oss;
      mpStatevector->WriteBinary( oss );
      mInitialStatevector = oss.str();
      mpStatevector->CommitStateChanges();
      InitializeConnections();
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

void
TfMain::HandleResting()
{
  RestingFilters();
#if( MODTYPE != EEGSRC ) // For non-source modules, Resting() is called once
                         // after the Running state drops to 0.
  mResting = false;
#endif // EEGSRC
}

void
TfMain::ResetStatevector()
{
  // State "Running" is the actual memory for the module's running state,
  // so we may not reset it.
  short running = mpStatevector->GetStateValue( "Running" ),
        sourceTime = mpStatevector->GetStateValue( "SourceTime" ),
        stimulusTime = mpStatevector->GetStateValue( "StimulusTime" );
  mpStatevector->ReadBinary( istringstream( mInitialStatevector ) );
  mpStatevector->SetStateValue( "Running", running );
  mpStatevector->SetStateValue( "SourceTime", sourceTime );
  mpStatevector->SetStateValue( "StimulusTime", stimulusTime );
}

void
TfMain::InitializeFilters()
{
  __bcierr.clear();
  GenericFilter::HaltFilters();
  bool errorOccurred = ( __bcierr.flushes() > 0 );
  float samplingRate,
        sampleBlockSize;
  PARAM* param = mParamlist.GetParamPtr( "SamplingRate" );
  samplingRate = param ? ::atoi( param->GetValue() ) : 1.0;
  param = mParamlist.GetParamPtr( "SampleBlockSize" );
  sampleBlockSize = param ? ::atoi( param->GetValue() ) : 1.0;
  MeasurementUnits::InitializeTimeUnit( samplingRate / sampleBlockSize );

  int numInputChannels = 0,
      numInputElements = 0;
#if( MODTYPE == EEGSRC )
  numInputChannels = 0;
  numInputElements = 0;
#elif( MODTYPE == SIGPROC )
  param = mParamlist.GetParamPtr( "TransmitChList" );
  numInputChannels = param ? param->GetNumValues() : 0;
  numInputElements = sampleBlockSize;
#elif( MODTYPE == APP )
  param = mParamlist.GetParamPtr( "NumControlSignals" );
  numInputChannels = param ? ::atoi( param->GetValue() ) : 2;
  numInputElements = 1;
#endif
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
TfMain::StartRunFilters()
{
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
TfMain::StopRunFilters()
{
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
TfMain::ProcessFilters( const GenericSignal* input )
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
TfMain::RestingFilters()
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

// **************************************************************************
// Function:   ShutdownConnections
// Purpose:    shuts down all connections and resets the display on the screen
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
TfMain::ShutdownConnections()
{
  eReceivingPort->Text = "N/A";
  eReceivingIP->Text = "N/A";
  rReceivingConnected->Checked = false;

  eSendingPort->Text = "N/A";
  eSendingIP->Text = "N/A";
  rSendingConnected->Checked = false;

  mOperatorSocket.close();
  mPreviousModuleSocket.close();
  mNextModuleSocket.close();
}

// **************************************************************************
// Function:   ShutdownSystem
// Purpose:    disables the timer and terminates the receiving thread
//             (which, in turn, shuts down all connections)
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
TfMain::ShutdownSystem()
{
  ShutdownConnections();

  delete mpStatevector;
  mpStatevector=NULL;

  GenericFilter::DisposeFilters();
}

// **************************************************************************
// Function:   Startup
// Purpose:    The listening socket
//             is opened and the connection to the operator is established
//             we then publish all our parameters and all our states
// Parameters: N/A
// Returns:    0 ... on error (e.g., Operator could not be found)
//             1 ... on success
// **************************************************************************
void
TfMain::Startup( AnsiString inTarget )
{
  // clear all parameters and states first
  mParamlist.ClearParamList();
  mStatelist.ClearStateList();

  // creating connection to the operator
  mOperatorSocket.open( ( inTarget + ":" + eOperatorPort->Text ).c_str() );
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
  eReceivingPort->Text = AnsiString( mPreviousModuleSocket.port() );
  eReceivingIP->Text = mPreviousModuleSocket.ip().c_str();

  Environment::EnterConstructionPhase( &mParamlist, &mStatelist, NULL, &mOperator );
  GenericFilter::InstantiateFilters();
  Environment::EnterNonaccessPhase();

#if( MODTYPE == SIGPROC )
  // Add the NumControlSignals parameter.
  mParamlist.AddParameter2List(
    "Filtering int NumControlSignals= 2"
    " 1 1 128 // the number of transmitted control signals" );
#endif // SIGPROC

  // add parameters for socket connection
  // my receiving socket port number
  mParamlist.AddParameter2List(
    "System string " THISMODULE "Port= x"
    " 4200 1024 32768 // the " THISMODULE " module's listening port" );
  mParamlist[ THISMODULE "Port" ].SetValue( AnsiString( mPreviousModuleSocket.port() ).c_str() );
  // and IP address
  mParamlist.AddParameter2List(
    "System string " THISMODULE "IP= x"
    " 127.0.0.1 127.0.0.1 127.0.0.1 // the " THISMODULE " module's listening IP" );
  mParamlist[ THISMODULE "IP" ].SetValue( mPreviousModuleSocket.ip() );

  // now, publish all parameters
  MessageHandler::PutMessage( mOperator, mParamlist );
  MessageHandler::PutMessage( mOperator, SYSCMD::EndOfParameter );
  // and the states
  MessageHandler::PutMessage( mOperator, mStatelist );
  MessageHandler::PutMessage( mOperator, SYSCMD::EndOfState );

  MessageHandler::PutMessage( mOperator, STATUS( "Waiting for configuration ...", 100 ) );
  Application->OnIdle = ApplicationIdleHandler;
}

// **************************************************************************
// Function:   InitializeConnections
// Purpose:    based upon the information in the list of parameters,
//             initialize the client socket connection to signal processing
// Parameters: N/A
// Returns:    ERR_NOERR - in case the socket connection could be established
//             ERR_NOSOCKCONN  - if socket connection could not be opened
//             ERR_NOSOCKPARAM - if there was no parameters describing the destination IP+port
// **************************************************************************
void
TfMain::InitializeConnections()
{
  PARAM* ipParam = mParamlist.GetParamPtr( NEXTMODULE "IP" ),
       * portParam = mParamlist.GetParamPtr( NEXTMODULE "Port" );

  if( !ipParam || !portParam )
  {
    BCIERR << NEXTMODULE "IP/Port parameters not available"
           << endl;
    return;
  }
  string nextModuleAddress = string( ipParam->GetValue() ) + ":" + portParam->GetValue();
  mNextModuleSocket.open( nextModuleAddress.c_str() );
  mNextModule.open( mNextModuleSocket );
  if( !mNextModule.is_open() )
  {
    BCIERR << "Could not make a connection to the " NEXTMODULE " module"
           << endl;
    return;
  }
  rSendingConnected->Checked = true;
  eSendingIP->Text = ipParam->GetValue();
  eSendingPort->Text = portParam->GetValue();

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

void __fastcall
TfMain::bConnectClick( TObject* )
{
  Startup( eOperatorIP->Text );
  if( __bcierr.flushes() == 0 )
  {
    bConnect->Enabled = false;
    bDisconnect->Enabled = true;
  }
}

void __fastcall
TfMain::bDisconnectClick( TObject* )
{
  ShutdownSystem();
  bConnect->Enabled = true;
  bDisconnect->Enabled = false;
}

void
TfMain::ProcessBCIAndWindowsMessages()
{
  while( mPreviousModule && mPreviousModule.rdbuf()->in_avail()
        || mOperator && mOperator.rdbuf()->in_avail()
        || mResting
        || ::GetQueueStatus( QS_ALLEVENTS ) )
  {
    // If there is a message from the previous module, it has highest priority.
    // For the SignalProcessing and the Application modules, these messages occur
    // in pairs of STATEVECTOR and VisSignal messages, so we try processing two
    // of them at once.
    const maxMsgFromPrevModule = 2;
    for( int i = 0; i < maxMsgFromPrevModule
                    && mPreviousModule && mPreviousModule.rdbuf()->in_avail(); ++i )
      mMessageHandler.HandleMessage( mPreviousModule );
    // If there are messsages from the operator, all of them must be processed
    // at once because there are message sequences that count as a single
    // message, e.g. a sequence of PARAM messages followed by a SYSCMD::EndOfParameter
    // message.
    const maxMsgFromOperator = 1000;
    for( int i = 0; i < maxMsgFromOperator && mOperator && mOperator.rdbuf()->in_avail(); ++i )
      mMessageHandler.HandleMessage( mOperator );
    // The mResting flag is treated as a pending message from the module to itself.
    // For non-source modules, it is cleared from the HandleResting() function
    // much as pending messages are cleared from the stream by the HandleMessage()
    // function.
    if( mResting )
      HandleResting();
    mResting &= mOperator.is_open();
    // Last of all, process messages from the Windows message queue if there are any.
    Application->ProcessMessages();
    ::Sleep( 0 );
  }
}

// This function contains the main event handling loop.
// It will be entered once when the program starts,
// and only be left when the program quits.
void __fastcall
TfMain::ApplicationIdleHandler( TObject*, bool& )
{
  Application->OnIdle = NULL;

  const bciMessageTimeout = 100; // ms -- the maximum amount of time windows
                                 // messages must wait to be processed. Windows
                                 // messages resulting from BCI2000 messages
                                 // (e.g. WM_PAINT messages) will be processed
                                 // without additional delay.
  try
  {
    while( !mTerminated )
    {
      if( !mResting && !::GetQueueStatus( QS_ALLEVENTS ) )
        tcpsocket::wait_for_read( mInputSockets, bciMessageTimeout );
      ProcessBCIAndWindowsMessages();
      rReceivingConnected->Checked = mPreviousModule.is_open();
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
  Application->Terminate();
}





