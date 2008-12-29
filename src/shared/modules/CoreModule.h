////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that represents functionality common to all BCI2000
//          core modules.
//
//          For a core module that does not use any GUI elements,
//          CoreModule::Run() takes care of all framework functionality, such
//          that a module's minimal main() function reads:
//
//          int main( int argc, char** argv )
//          {
//            bool success = CoreModule().Run( argc, argv );
//            return ( success ? 0 : -1 );
//          }
//
//          For core modules that use GUI elements, the GUI's message loop must
//          be replaced by the message loop implemented within the CoreModule
//          class. To process GUI messages, override CoreModule::ProcessGUIMessages()
//          from a derived class:
//
//          class CoreModuleGUI : public CoreModule
//          {
//            public:
//              CoreModuleGUI() {}
//              virtual void ProcessGUIMessages()
//              {
//                // Example applying to Borland VCL
//                Application->ProcessMessages();
//                ::Sleep( 0 );
//              }
//              virtual bool GUIMessagesPending()
//              {
//                return ::GetQueueStatus( QS_ALLINPUT );
//              }
//           };
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef CORE_MODULE_H
#define CORE_MODULE_H

#include "Environment.h"
#include "Param.h"
#include "State.h"
#include "GenericSignal.h"
#include "SockStream.h"
#include "MessageHandler.h"

#define SIGSRC  1
#define SIGSRC_NAME  "SignalSource"
#define SIGPROC 2
#define SIGPROC_NAME "SignalProcessing"
#define APP     3
#define APP_NAME     "Application"

#define THISVERSION "0.40"

#if( MODTYPE == SIGSRC )
# define THISMODULE SIGSRC_NAME
# define THISOPPORT "4000"
# define PREVMODULE APP_NAME
# define NEXTMODULE SIGPROC_NAME
#elif( MODTYPE == SIGPROC )
# define PREVMODULE SIGSRC_NAME
# define THISOPPORT "4001"
# define THISMODULE SIGPROC_NAME
# define NEXTMODULE APP_NAME
#elif( MODTYPE == APP )
# define PREVMODULE SIGPROC_NAME
# define THISOPPORT "4002"
# define THISMODULE APP_NAME
# define NEXTMODULE SIGSRC_NAME
#else
# error Unknown MODTYPE value
#endif


class CoreModule : private MessageHandler
{
  static const int cInitialConnectionTimeout = 20000; // ms

 public:
  CoreModule();
  virtual ~CoreModule();
  bool Run( int argc, char** argv );

 protected:
  // Override to integrate with a GUI library.
  virtual void ProcessGUIMessages() {}
  virtual bool GUIMessagesPending() { return false; }
  // Calling Terminate() will end message processing.
  void Terminate() { mTerminated = true; }

 private:
  // No copying or assignment.
  CoreModule( const CoreModule& );
  CoreModule& operator=( const CoreModule& );

  bool Initialize( int argc, char** argv );
  void MainMessageLoop();
  void ProcessBCIAndGUIMessages();
  void ProcessBCIEvents();

  void InitializeOperatorConnection( const std::string& operatorAddress );
  void InitializeCoreConnections();
  void ShutdownSystem();
  void ResetStatevector();

  void InitializeFilters( const class SignalProperties& );
  void StartRunFilters();
  void StopRunFilters();
  void ProcessFilters( const class GenericSignal& );
  void RestingFilters();

  void HandleResting();

  // BCI message handling functions.
  bool HandleParam( std::istream& );
  bool HandleState( std::istream& );
  bool HandleVisSignal( std::istream& );
  bool HandleVisSignalProperties( std::istream& );
  bool HandleStateVector( std::istream& );
  bool HandleSysCommand( std::istream& );

  bool             mTerminated;
  ParamList        mParamlist;
  StateList        mStatelist;
  StateVector*     mpStatevector;
  std::string      mInitialStatevector;
  GenericSignal    mOutputSignal;
  client_tcpsocket mOperatorSocket,
                   mNextModuleSocket;
  streamsock::set_of_instances mInputSockets;
  server_tcpsocket mPreviousModuleSocket;
  sockstream       mOperator,
                   mNextModule,
                   mPreviousModule;
  bool             mFiltersInitialized,
                   mLastRunning,
                   mResting,
                   mStartRunPending,
                   mStopRunPending;
  void*            mMutex;
  int              mSampleBlockSize;
};

#endif // CORE_MODULE_H

