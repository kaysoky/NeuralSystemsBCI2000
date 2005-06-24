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
#ifndef UCoreMainH
#define UCoreMainH

#include "../shared/UEnvironment.h"
#include "../shared/UParameter.h"
#include "../shared/UState.h"
#include "../shared/UGenericSignal.h"
#include "../shared/TCPStream.h"
#include "../shared/MessageHandler.h"
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>

#define ERR_NOERR       0
#define ERR_NOSOCKCONN  1
#define ERR_NOSOCKPARAM 2

#define EEGSRC  1
#define EEGSRC_NAME  "EEGSource"
#define SIGPROC 2
#define SIGPROC_NAME "SignalProcessing"
#define APP     3
#define APP_NAME     "Application"

#define THISVERSION "0.30"

#if( MODTYPE == EEGSRC )
# define THISMODULE EEGSRC_NAME
# define THISCOLOR clRed
# define THISOPPORT "4000"
# define PREVMODULE APP_NAME
# define NEXTMODULE SIGPROC_NAME
#elif( MODTYPE == SIGPROC )
# define PREVMODULE EEGSRC_NAME
# define THISCOLOR clWhite
# define THISOPPORT "4001"
# define THISMODULE SIGPROC_NAME
# define NEXTMODULE APP_NAME
#elif( MODTYPE == APP )
# define PREVMODULE SIGPROC_NAME
# define THISCOLOR clBlue
# define THISOPPORT "4002"
# define THISMODULE APP_NAME
# define NEXTMODULE EEGSRC_NAME
#else
# error Unknown MODTYPE value
#endif


class TfMain : public TForm
{
  static const cInitialConnectionTimeout = 20000; // ms

__published: // IDE-managed Components
        TEdit *eOperatorIP;
        TLabel *Label1;
        TLabel *Label2;
        TEdit *eOperatorPort;
        TButton *bConnect;
        TButton *bDisconnect;
        TEdit *eReceivingPort;
        TEdit *eReceivingIP;
        TLabel *Label3;
        TLabel *Label4;
        TLabel *Label5;
        TEdit *eSendingIP;
        TLabel *Label6;
        TEdit *eSendingPort;
        TCheckBox *rReceivingConnected;
        TCheckBox *rSendingConnected;
        void __fastcall bConnectClick(TObject *Sender);
        void __fastcall bDisconnectClick(TObject *Sender);
public:      // User declarations
              __fastcall TfMain(TComponent* Owner);
              __fastcall ~TfMain( void );
private:     // User declarations
        void __fastcall  ApplicationIdleHandler( TObject*, bool& );

        void             InitializeFilters();
        void             StartRunFilters();
        void             StopRunFilters();
        void             ProcessFilters( const class GenericSignal* );
        void             RestingFilters();

        void             Startup( AnsiString connectto );
        void             Terminate() { mTerminated = true; }
        void             InitializeConnections();
        void             ShutdownConnections();
        void             ShutdownSystem();
        void             ResetStatevector();
        void             ProcessBCIAndWindowsMessages();
        void             HandleResting();

        HANDLE           mMutex;
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

        // We derive a class _MessageHandler from
        // MessageHandler and create an instance of it as a member of TfMain.
        // All that member does is to call the TfMain message handling member
        // functions as if TfMain directly inherited from MessageHandler.
        // VCL quirks forbid that TfMain directly inherits from MessageHandler,
        // so we use the described construction instead.
        bool HandlePARAM(       std::istream& );
        bool HandleSTATE(       std::istream& );
        bool HandleVisSignal(   std::istream& );
        bool HandleSTATEVECTOR( std::istream& );
        bool HandleSYSCMD(      std::istream& );

        class _MessageHandler;
        friend class _MessageHandler;
        class _MessageHandler : public MessageHandler
        {
          public:
            _MessageHandler( TfMain& parent ) : mParent( parent ) {}
          private:
            virtual bool HandlePARAM(       std::istream& is ) { return mParent.HandlePARAM( is ); }
            virtual bool HandleSTATE(       std::istream& is ) { return mParent.HandleSTATE( is ); }
            virtual bool HandleVisSignal(   std::istream& is ) { return mParent.HandleVisSignal( is ); }
            virtual bool HandleSTATEVECTOR( std::istream& is ) { return mParent.HandleSTATEVECTOR( is ); }
            virtual bool HandleSYSCMD(      std::istream& is ) { return mParent.HandleSYSCMD( is ); }

            TfMain& mParent;
        } mMessageHandler;
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif // UCoreMainH
