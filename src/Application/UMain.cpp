/******************************************************************************
 * Program:   Application.EXE                                                 *
 * Module:    UMAIN.CPP                                                       *
 * Comment:   The Application module for BCI2000                              *
 * Version:   0.25                                                            *
 * Author:    Gerwin Schalk                                                   *
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
 ******************************************************************************/

//---------------------------------------------------------------------------
#include "PCHIncludes.h"
#pragma hdrstop

#include <stdio.h>
#include <stdlib.h>

#include "..\shared\defines.h"
#include "UGenericFilter.h"
#include "UGenericSignal.h"
#include "UGenericVisualization.h"
#include "UCoreMessage.h"
#include "Task.h"
#include "UMain.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TfMain *fMain;
STATEVECTOR *statevector;
class TReceivingThread *receiving_thread;

// **************************************************************************
// Function:   TReceivingThread
// Purpose:    class definition for TReceivingThread
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
//---------------------------------------------------------------------------
class TReceivingThread : public  TServerClientThread
{
  protected:
        TWinSocketStream        *pStream;
        int     InitializeApplication();
        int     HandleSigProcMessage(COREMESSAGE *coremessage);
        int     numcontrolsignals, statevectorlength;
        GenericSignal* controlsignals;
        void __fastcall ProcessTask(void);
        void __fastcall InitializeTask(void);
  public:
    __fastcall TReceivingThread( bool CreateSuspended, TServerClientWinSocket* ASocket);
    void __fastcall ClientExecute( void );
  private:
    bool errorOccurred;
};


// **************************************************************************
// Function:   TReceivingThread
// Purpose:    construction definition for TReceivingThread
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
//---------------------------------------------------------------------------
__fastcall TReceivingThread::TReceivingThread(bool CreateSuspended, TServerClientWinSocket* ASocket)
: TServerClientThread( CreateSuspended, ASocket ),
  controlsignals( NULL ),
  pStream( NULL ),
  errorOccurred( false )
{
}
//---------------------------------------------------------------------------

// **************************************************************************
// Function:   InitializeApplication
// Purpose:    initializes Application every time Running switches from 0 to 1
// Parameters: N/A
// Returns:    1 ... no error
//             0 ... error
// **************************************************************************
int TReceivingThread::InitializeApplication()
{
const PARAM   *paramptr1, *paramptr2;
int     ret;

  ret=1;
  paramptr1=fMain->paramlist.GetParamPtr("NumControlSignals");
  paramptr2=fMain->paramlist.GetParamPtr("StateVectorLength");
  if ((!paramptr1) || (!paramptr2))
     ret=0;
  else
     {
     numcontrolsignals=atoi(paramptr1->GetValue());
     statevectorlength=atoi(paramptr2->GetValue());
     if (!statevector)
        {
        Application->MessageBox("StateVector not defined", "Error", MB_OK);
        ret=0;
        }
     else
        if (statevector->GetStateVectorLength() != statevectorlength) ret=0;
     }

  Synchronize(InitializeTask);
  if( !errorOccurred )
    fMain->corecomm->SendStatus("202 Application module initialized");

  delete controlsignals;
  delete pStream;

  pStream=new TWinSocketStream(ClientSocket, 5000);
  controlsignals=new GenericSignal( 1, numcontrolsignals );
  if ((!controlsignals) || (!pStream) || errorOccurred ) ret=0;

 return(ret);
}


// **************************************************************************
// Function:   ClientExecute
// Purpose:    main loop for TReceivingThread that handles
//             the incoming data from the Signalprocessing core module
//             it is executed when Signalprocessing connects
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TReceivingThread::ClientExecute(void)
{
COREMESSAGE     *coremessage;
int     ret1, ret2;
int     trycount;

  trycount=0;
  // if the statevector has not been defined yet, wait a little
  while ((!statevector) && (trycount < 10))
   {
   Sleep(300);
   trycount++;
   }

  // initialize application as soon as we are connected
  if (InitializeApplication() == 0) Terminate();

  while(!Terminated && ClientSocket->Connected )
   {
   try
    {
    if (pStream->WaitForData(1000))
       {
       // read the statevector
       coremessage=new COREMESSAGE();
       ret1=coremessage->ReceiveCoreMessage(pStream);
       if (ret1 == ERRCORE_NOERR)
          {
          coremessage->ParseMessage();
          ret2=HandleSigProcMessage(coremessage);
          if (ret2 == 0)        // error ?
             Terminate();
          }
       delete coremessage;
       }
    } // end try
   catch( TooGeneralCatch& )
    {
    Terminate();
    }
   } // end while

 delete controlsignals;
 delete pStream;
 ReturnValue=1;
 // shut down connection to the EEGSource
 if (fMain->sendingcomm)
    {
    fMain->sendingcomm->SendSysCommand("Reset");
    fMain->sendingcomm->Terminate();
    }

 ReturnValue=1;   
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   ProcessTask
// Purpose:    this function calls the process method of the task class
//             can be called by using Synchronize and thereby avoiding
//             synchronization issues with the main VCL thread
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TReceivingThread::ProcessTask(void)
{
 Environment::EnterProcessingPhase( &fMain->paramlist, &fMain->statelist, statevector, fMain->corecomm );
 fMain->Task->Process( controlsignals, NULL );
 Environment::EnterNonaccessPhase();
}


// **************************************************************************
// Function:   InitializeTask
// Purpose:    this function calls the initialize method of the task class
//             can be called by using Synchronize and thereby avoiding
//             synchronization issues with the main VCL thread
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TReceivingThread::InitializeTask(void)
{
  Environment::EnterPreflightPhase( &fMain->paramlist, &fMain->statelist, statevector, fMain->corecomm );
  SignalProperties sp_in( 1, numcontrolsignals ), sp_out;
  fMain->Task->Preflight( sp_in, sp_out );
  errorOccurred = ( __bcierr.flushes() > 0 );
  Environment::EnterNonaccessPhase();
  if( !errorOccurred )
  {
    Environment::EnterInitializationPhase( &fMain->paramlist, &fMain->statelist, statevector, fMain->corecomm );
    fMain->Task->Initialize();
    errorOccurred |= ( __bcierr.flushes() > 0 );
    Environment::EnterNonaccessPhase();
  }
}


// **************************************************************************
// Function:   HandleSigProcMessage
// Purpose:    this method handles incoming messages from Signal Processing
//             it assumes that coremessage->ParseMessage() has been called before
// Parameters: coremessage - a pointer to the actual received message
// Returns:    0 ... on error
//             1 ... everything OK
// **************************************************************************
int TReceivingThread::HandleSigProcMessage(COREMESSAGE *coremessage)
{
GenericIntSignal *OutSignal;
static  short oldrunning=0, running;
int     ret, ret1, ret2, res, count;
float   sigres;

 ret=1;

 if (coremessage->GetDescriptor() == COREMSG_SYSCMD)
    {
    if (stricmp(coremessage->syscmd.GetSysCmd(), "Reset") == 0)
       Terminate();
    }

 if (coremessage->GetDescriptor() == COREMSG_STATEVECTOR)
    {
    memcpy(statevector->GetStateVectorPtr(), coremessage->GetBufPtr(), statevector->GetStateVectorLength());
    running=statevector->GetStateValue("Running");
    if ((running == 1) && (oldrunning == 0))                      // we have just started a run; need to re-configure
       if (InitializeApplication() == 0)
          {
          fMain->corecomm->SendStatus("405 Signal Processing dropped connection unexpectedly");
          ret=0;              // if there was an error, return 0
          }
    oldrunning=running;
    }

 if (coremessage->GetDescriptor() == COREMSG_DATA)
    {
    //memcpy(controlsignals, coremessage->GetBufPtr()+5, sizeof(short)*numcontrolsignals);
    controlsignals->SetChannel( ( short* )( coremessage->GetBufPtr() + 5 ), 0 );

    // now let the application do its job
    // use the thread's synchronize function to avoid any visual artifact
    Synchronize(ProcessTask);

    // if the socket is open, then ...
    if ((fMain->sendingcomm->Connected()) && (!Terminated))
       {
       // send the state vector on to EEGSource
       ret1=fMain->sendingcomm->SendStateVector2CoreModule(statevector);
       if (!ret1) return(0);
       }
    }

 return(ret);
}


void __fastcall TfMain::HandleMessage(TMessage &Message)
{
COREMESSAGE     *coremessage;
int             modID;

 coremessage=(COREMESSAGE *)Message.WParam;
 modID=(int)Message.LParam;

 // handle coremessage only, if it came from Operator
 if (modID == COREMODULE_OPERATOR)
    HandleCoreMessage(coremessage);
}


void __fastcall TfMain::ResetMessage(TMessage &Message)
{
int     modID;

 modID=(int)Message.LParam;

 // reset only, if connection to operator died
 if (modID == COREMODULE_OPERATOR)
    {
    ShutdownSystem();
    Close();
    }
}


// **************************************************************************
// Function:   ShutdownConnections
// Purpose:    shuts down all connections and resets the display on the screen
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TfMain::ShutdownConnections()
{
 eReceivingPort->Text="N/A";
 eReceivingIP->Text="N/A";
 rReceivingConnected->Checked=false;

 eSendingPort->Text="N/A";
 eSendingIP->Text="N/A";
 rSendingConnected->Checked=false;

 try
  {
  if (ReceivingSocket->Socket->ActiveConnections > 0)
     if (ReceivingSocket->Socket->Connections[0])
        ReceivingSocket->Socket->Connections[0]->Close();
  } catch( TooGeneralCatch& ) {}
 try
  {
  if (ReceivingSocket->Active) ReceivingSocket->Close();
  } catch( TooGeneralCatch& ) {}
 try
  {
  if (sendingcomm)
     if (sendingcomm->Connected()) sendingcomm->Terminate();
  } catch( TooGeneralCatch& ) {}
 try
  {
  if (corecomm)
     if (corecomm->Connected()) corecomm->Terminate();
  } catch( TooGeneralCatch& ) {}
}


// **************************************************************************
// Function:   ShutdownSystem
// Purpose:    disables the timer and terminates the receiving thread
//             (which, in turn, shuts down all connections)
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TfMain::ShutdownSystem()
{
static bool inhere=false;
// FILE *fp;

 if (inhere == true) return;
 inhere=true;

 // now, close all the other connections
 ShutdownConnections();

 corecomm=sendingcomm=NULL;

 // paramlist.ClearParamList();
 // statelist.ClearStateList();
 delete statevector;
 statevector=NULL;

 delete Task;
 Task= NULL;

 inhere=false;
}


// **************************************************************************
// Function:   TfMain
// Purpose:    the constructor for the TfMain class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
        : TForm(Owner),
          Task( NULL ),
          sendingcomm( NULL )
{
}

//********************************************************************
//  destructor
//********************************************************************

__fastcall TfMain::~TfMain( void )
{
        delete Task;
        Task= NULL;

}


// **************************************************************************
// Function:   bConnectClick
// Purpose:    when the Connect button is clicked on, the listening socket
//             is opened and the connection to the operator is established
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
//---------------------------------------------------------------------------
void __fastcall TfMain::bConnectClick(TObject *Sender)
{
 StartupApplication(eOperatorIP->Text);
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   StartupApplication
// Purpose:    The listening socket
//             is opened and the connection to the operator is established
//             we then publish all our parameters and all our states
// Parameters: N/A
// Returns:    0 ... on error (e.g., Operator could not be found)
//             1 ... on success
// **************************************************************************
int TfMain::StartupApplication(AnsiString connectto)
{
int     ret;
char    paramstring[255];

 // clear all parameters and states first
 paramlist.ClearParamList();
 statelist.ClearStateList();

 // create and initialize CORECOMM object
 // thereby creating connection to the operator
 corecomm=new CORECOMM(true);
 ret=corecomm->Initialize(connectto, 4002, fMain, COREMODULE_OPERATOR);
 if (ret == 0)
    {
    Application->MessageBox("Could not make a connection to the Operator", "Error", MB_OK);
    if (corecomm) delete corecomm;
    corecomm=NULL;
    return(0);
    }
 else
    corecomm->Resume();                 // start receiving on the operator connection

 // enable the receiving socket
 ReceivingSocket->Open();
 eReceivingPort->Text=AnsiString(ReceivingSocket->Socket->LocalPort);
 eReceivingIP->Text=ReceivingSocket->Socket->LocalHost;

 Environment::EnterConstructionPhase( &paramlist, &statelist, NULL, NULL );
 Task= new TTask;
 Environment::EnterNonaccessPhase();

 // add parameters for socket connection
 // my receiving socket port number
 sprintf(paramstring, "System string ApplicationPort= %d 4200 1024 32768 // this module's listening port\n", ReceivingSocket->Socket->LocalPort);
 paramlist.AddParameter2List(paramstring, strlen(paramstring));
 // and IP address
 sprintf(paramstring, "System string ApplicationIP= %s 127.0.0.1 127.0.0.1 127.0.0. // this module's listening IP\n", corecomm->GetSocket()->LocalAddress.c_str());
 paramlist.AddParameter2List(paramstring, strlen(paramstring));
 // now, publish these parameters
 corecomm->PublishParameters(&paramlist);
 // and the states
 corecomm->PublishStates(&statelist);
 corecomm->SendStatus("100 Waiting for configuration ...");
 return(1);
}


// **************************************************************************
// Function:   bDisconnectClick
// Purpose:    when the Disconnect button is clicked on, system is shut down
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::bDisconnectClick(TObject *Sender)
{
 ShutdownSystem();
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
int TfMain::InitializeConnections()
{
char    destIP[256];
int     destport, res;

 if ((paramlist.GetParamPtr(PARAMNAME_EEGSOURCEIP)) && (paramlist.GetParamPtr(PARAMNAME_EEGSOURCEPORT)))
    {
    strcpy(destIP, paramlist.GetParamPtr(PARAMNAME_EEGSOURCEIP)->GetValue());
    destport=atoi(paramlist.GetParamPtr(PARAMNAME_EEGSOURCEPORT)->GetValue());
    if (sendingcomm) delete sendingcomm;
    sendingcomm=new CORECOMM(true);
    if (!sendingcomm) return(ERR_NOSOCKCONN);
    res=sendingcomm->Initialize(AnsiString(destIP), destport, (TForm *)fMain, COREMODULE_EEGSOURCE);
    if (res == 0) return(ERR_NOSOCKCONN);
    rSendingConnected->Checked=true;
    eSendingIP->Text=destIP;
    eSendingPort->Text=AnsiString(destport);
#if 0
    corecomm->SendStatus("202 Application module initialized");
#endif
    }
 else
    return(ERR_NOSOCKPARAM);

return(ERR_NOERR);
}


// **************************************************************************
// Function:   SendStatus
// Purpose:    sends a status coremessage to the specified socket
// Parameters: line - pointer to a status string
//             Socket - Socket descriptor for connection to the operator
// Returns:    N/A
// **************************************************************************
void TfMain::SendStatus(char *line, TCustomWinSocket *Socket)
{
COREMESSAGE     *coremessage;

 if (Socket)
    if (Socket->Connected)
       {
       coremessage=new COREMESSAGE;
       coremessage->SetDescriptor(COREMSG_STATUS);
       coremessage->SetLength((unsigned short)strlen(line));
       strncpy(coremessage->GetBufPtr(strlen(line)), line, strlen(line));
       coremessage->SendCoreMessage((TCustomWinSocket *)Socket);
       delete coremessage;
       }
}


// **************************************************************************
// Function:   HandleCoreMessage
// Purpose:    this method handles incoming messages from the operator
//             it assumes that coremessage->ParseMessage() has been called before
// Parameters: coremessage - a pointer to the actual message
//             Socket - Socket descriptor for connection to the operator
// Returns:    N/A
// **************************************************************************
void TfMain::HandleCoreMessage(COREMESSAGE *coremessage)
{
 // we received a parameter
 if (coremessage->GetDescriptor() == COREMSG_PARAMETER)
    {
    // now, add the parameter to the list of parameters
    if (coremessage->param.Valid())
       paramlist.CloneParameter2List(&(coremessage->param));
    }

 // we received a state
 if (coremessage->GetDescriptor() == COREMSG_STATE)
    {
    if (coremessage->state.Valid())
       {
       // now, add the state to the list of states
       statelist.AddState2List(&(coremessage->state));
       }
    }

 // the message I got was a state vector
 if (coremessage->GetDescriptor() == COREMSG_STATEVECTOR)
    {
    }

 // the message I got was a system command
 if (coremessage->GetDescriptor() == COREMSG_SYSCMD)
    {
    if (strcmp(coremessage->syscmd.GetSysCmd(), "EndOfState") == 0)
       {
       // set the state vector object
       if (statevector) delete statevector;

       statevector=new STATEVECTOR(&statelist);
       // create connection to Signal Processing
       if (InitializeConnections() == ERR_NOERR)
          {
          }
       else
          {
          }
       }
    }
}


// **************************************************************************
// Function:   ReceivingSocketClientDisconnect
// Purpose:    If the receiving socket is being disconnected, shut down the system
// Parameters: Socket - the socket handle for the receiving connection to signal processing
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::ReceivingSocketClientDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
 receiving_thread->Terminate();
}
//---------------------------------------------------------------------------

// **************************************************************************
// Function:   FormClose
// Purpose:    is being called, if the main form is being closed
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::FormClose(TObject *Sender, TCloseAction &Action)
{
 ShutdownSystem();
 if (statevector) delete statevector;
 if (corecomm)    delete corecomm;
 if (sendingcomm) delete sendingcomm;
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   ReceivingSocketGetThread
// Purpose:    if Signalprocessing connects to the Application, a new thread
//             is being spawned that handles the incoming data over
//             a blocking socket connection
// Parameters: ClientSocket - the socket handle for the blocking connection
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::ReceivingSocketGetThread(TObject *Sender, TServerClientWinSocket *ClientSocket, TServerClientThread *&SocketThread)
{
 rReceivingConnected->Checked=true;
 receiving_thread=new TReceivingThread(false, ClientSocket);
 SocketThread=receiving_thread;
}
//---------------------------------------------------------------------------


void __fastcall TfMain::FormCreate(TObject *Sender)
{
AnsiString  connectto;
int     numparams, i;

 // set the priority to high for the Application
 // does not work for some reason
 // could set the priority CLASS for the main PROCESS
 // but that works only with IDLE, NORMAL, and HIGH
 // be cautious about HIGH
 SetThreadPriority((HANDLE)GetCurrentThreadId(), THREAD_PRIORITY_ABOVE_NORMAL);
 GetThreadPriority((HANDLE)GetCurrentThreadId());

 GUImode=true;

 numparams=ParamCount();
 for (i=0; i<=numparams; i++)
  {
  if (ParamStr(i) == "AUTOSTART")
     {
     if (i+1 <= numparams)
        connectto=ParamStr(i+1);
     else
        connectto="127.0.0.1";
     GUImode=false;
     Application->ShowMainForm=false;
     if (StartupApplication(connectto) == 0)
        Application->Terminate();
     }
  }
}
//---------------------------------------------------------------------------


