////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File:    CoreModule.h
//
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
//
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
// $Log$
// Revision 1.2  2006/04/19 16:17:11  mellinger
// Removed Win32 API calls, introduced virtual functions for generic GUI interfacing.
//
// Revision 1.1  2006/03/30 15:42:32  mellinger
// Initial version.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef CoreModuleH
#define CoreModuleH

#include "UEnvironment.h"
#include "UParameter.h"
#include "UState.h"
#include "UGenericSignal.h"
#include "TCPStream.h"
#include "MessageHandler.h"

#define EEGSRC  1
#define EEGSRC_NAME  "EEGSource"
#define SIGPROC 2
#define SIGPROC_NAME "SignalProcessing"
#define APP     3
#define APP_NAME     "Application"

#define THISVERSION "0.40"

#if( MODTYPE == EEGSRC )
# define THISMODULE EEGSRC_NAME
# define THISOPPORT "4000"
# define PREVMODULE APP_NAME
# define NEXTMODULE SIGPROC_NAME
#elif( MODTYPE == SIGPROC )
# define PREVMODULE EEGSRC_NAME
# define THISOPPORT "4001"
# define THISMODULE SIGPROC_NAME
# define NEXTMODULE APP_NAME
#elif( MODTYPE == APP )
# define PREVMODULE SIGPROC_NAME
# define THISOPPORT "4002"
# define THISMODULE APP_NAME
# define NEXTMODULE EEGSRC_NAME
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

  void InitializeOperatorConnection( const std::string& operatorAddress );
  void InitializeCoreConnections();
  void ShutdownSystem();
  void ResetStatevector();

  void InitializeFilters();
  void StartRunFilters();
  void StopRunFilters();
  void ProcessFilters( const class GenericSignal* );
  void RestingFilters();

  void HandleResting();

  // BCI message handling functions.
  bool HandlePARAM(       std::istream& );
  bool HandleSTATE(       std::istream& );
  bool HandleVisSignal(   std::istream& );
  bool HandleSTATEVECTOR( std::istream& );
  bool HandleSYSCMD(      std::istream& );

  bool             mTerminated;
  PARAMLIST        mParamlist;
  STATELIST        mStatelist;
  STATEVECTOR*     mpStatevector;
  std::string      mInitialStatevector;
  GenericSignal    mOutputSignal;
  client_tcpsocket mOperatorSocket,
                   mNextModuleSocket;
  tcpsocket::set_of_instances mInputSockets;
  server_tcpsocket mPreviousModuleSocket;
  tcpstream        mOperator,
                   mNextModule,
                   mPreviousModule;
  bool             mLastRunning,
                   mResting,
                   mStartRunPending,
                   mStopRunPending;
  void*            mMutex;
};

#endif // CoreModuleH
