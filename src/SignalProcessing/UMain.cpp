/******************************************************************************
 * Program:   SignalProcessing.EXE                                            *
 * Module:    UMAIN.CPP                                                       *
 * Comment:   The SignalProcessing module for BCI2000                         *
 * Version:   0.24                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.07 - 03/04/2000 - Created signal processing from generic EEGsource mod. *
 * V0.08 - 03/09/2000 - Created signal processing again from EEGsource module *
 * V0.09 - 03/16/2000 - Complete communication in between all core modules    *
 * V0.10 - 03/20/2000 - Updated to C++Builder 5.0                             *
 * V0.11 - 03/27/2000 - made the receiving socket connection                  *
 *                      blocking inst. of non-blocking                        *
 * V0.12 - 03/30/2000 - stable and documented version                         *
 * V0.13 - 04/19/2000 - test version                                          *
 * V0.14 - 05/23/2000 - prepared for GenericSignal, etc.                      *
 * V0.15 - 06/15/2000 - a couple minor changes to make it compatible with 0.15*
 * V0.16 - 07/26/2000 - added a GenericFilter                                 *
 * V0.17 - 08/09/2000 - visualizing the received channels                     *
 * V0.18 - 09/21/2000 - improved socket handling                              *
 * V0.19b- 04/11/2001 - CORECOMM (blocking sockets) for core communication    *
 * V0.20 - 04/13/2001 - using coremessages to receive from the EEGsource      *
 *                      and to send out info to the application               *
 * V0.21 - 05/15/2001 - removed some bugs in various filters                  *
 * V0.22 - 06/07/2001 - upgraded to shared libraries V0.17                    *
 * V0.23 - 07/16/2001 - accumulated bug fixes of V0.22x                       *
 * V0.24 - 04/11/2002 - updated to Borland C++ Builder 6.0                    *
 *                      improved error handling                               *
 ******************************************************************************/

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "..\shared\defines.h"
#include "UBCI2000Error.h"
#include "CalibrationFilter.h"
#include "UFilterHandling.h"
#include "UGenericVisualization.h"
#include "UCoreMessage.h"
#include "UMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TfMain          *fMain;
STATEVECTOR     *statevector;
FILTERS         *filters;

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
        char    *buf;
        short   *controlsignals;
        int     transmitchannels, samples, statevectorlength, numcontrolsignals;
        int     InitializeSignalProcessing();
        int     HandleEEGSourceMessage(COREMESSAGE *coremessage);
  public:
    __fastcall TReceivingThread( bool CreateSuspended, TServerClientWinSocket* ASocket );
    void __fastcall ClientExecute( void );
};


// **************************************************************************
// Function:   TReceivingThread
// Purpose:    construction definition for TReceivingThread
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
//---------------------------------------------------------------------------
__fastcall TReceivingThread::TReceivingThread(bool CreateSuspended, TServerClientWinSocket* ASocket) : TServerClientThread( CreateSuspended, ASocket )
{
 pStream=NULL;
 buf=NULL;
 controlsignals=NULL;
}
//---------------------------------------------------------------------------

TReceivingThread  *receiving_thread;


// **************************************************************************
// Function:   InitializeSignalProcessing
// Purpose:    initializes Signal Processing every time Running switches from 0 to 1
// Parameters: N/A
// Returns:    1 ... no error
//             0 ... error
// **************************************************************************
int TReceivingThread::InitializeSignalProcessing()
{
char    errmsg[1024];
int     ret, res;

  // determine the size of the transmitted content by looking at the system parameters
  // I do try{}, because for some reason, we could not have requested (or received) all parameters
  ret=1;
  try
   {
   transmitchannels=atoi(fMain->paramlist.GetParamPtr("TransmitCh")->GetValue());
   statevectorlength=atoi(fMain->paramlist.GetParamPtr("StateVectorLength")->GetValue());
   samples=atoi(fMain->paramlist.GetParamPtr("SampleBlockSize")->GetValue());
   numcontrolsignals=atoi(fMain->paramlist.GetParamPtr("NumControlSignals")->GetValue());
   if (!statevector)
      {
      fMain->corecomm->SendStatus("412 Signal Processing: state vector is not defined ! ");
      ret=0;
      }
   if (statevector->GetStateVectorLength() != statevectorlength) ret=0;
   }
  catch(...)
   {
   fMain->corecomm->SendStatus("413 Signal Processing: Exception thrown in TReceivingThread::InitializeSignalProcessing()");
   ret=0;
   }

  // now, let's initialize all filters and all signals
  res=filters->Initialize(&(fMain->paramlist), statevector, fMain->corecomm);
  if (res == 0)
     {
     sprintf(errmsg, "408 %s", filters->error.GetErrorMsg());
     fMain->corecomm->SendStatus(errmsg);
     ret=0;
     }

  if (pStream) delete pStream;
  if (buf) free(buf);
  if (controlsignals) delete [] controlsignals;

  pStream=new TWinSocketStream(ClientSocket, 5000);
  buf=(char *)malloc(sizeof(short)*transmitchannels*samples);
  controlsignals=new short[numcontrolsignals];
  if ((!buf) || (!controlsignals) || (!pStream)) ret=0;

 return(ret);
}


// **************************************************************************
// Function:   ClientExecute
// Purpose:    main loop for TReceivingThread that handles
//             the incoming data from the EEGsource core module
//             it is executed when EEGsource connects
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TReceivingThread::ClientExecute(void)
{
COREMESSAGE     *coremessage;
int     i, ret1, ret2;
int     trycount;

  trycount=0;
  // if the statevector has not been defined yet, wait a little
  while ((!statevector) && (trycount < 10))
   {
   Sleep(300);
   trycount++;
   }

  // initialize signal processing as soon as we are connected
  if (InitializeSignalProcessing() == 0) Terminate();

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
          ret2=HandleEEGSourceMessage(coremessage);
          if (ret2 == 0)        // error ?
             Terminate();
          }
       delete coremessage;
       }
    } // end try
   catch(...)
    {
    Terminate();
    }
   } // end while

 free(buf);
 delete [] controlsignals;
 delete pStream;
 if (filters) delete filters;
 filters=NULL;
 pStream=NULL;
 controlsignals=NULL;
 buf=NULL;

 // shut down connection to the Application
 if (fMain->sendingcomm)
    {
    fMain->sendingcomm->SendSysCommand("Reset");
    fMain->sendingcomm->Terminate();
    }

 ReturnValue=1;
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   HandleEEGSourceMessage
// Purpose:    this method handles incoming messages from the EEGsource
//             it assumes that coremessage->ParseMessage() has been called before
// Parameters: coremessage - a pointer to the actual received message
// Returns:    0 ... on error
//             1 ... everything OK
// **************************************************************************
int TReceivingThread::HandleEEGSourceMessage(COREMESSAGE *coremessage)
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
       if (InitializeSignalProcessing() == 0)
          {
          fMain->corecomm->SendStatus("404 EEGsource dropped connection unexpectedly");
          ret=0;              // if there was an error, return 0
          }
    oldrunning=running;
    }

 if (coremessage->GetDescriptor() == COREMSG_DATA)
    {
    memcpy(buf, coremessage->GetBufPtr()+5, sizeof(short)*transmitchannels*samples);

    // now let the filters do their job (only if we are not suspended)
    if (running == 1)
       {
       res=filters->Process(buf);
       if (res == 0)     // there was an error in any filter
          {
          fMain->corecomm->SendStatus("409 Error while processing filters");
          ret=0;
          }
       }
      else  res= filters->Resting(buf);

    // if the socket is open, then ...
    if ((fMain->sendingcomm->Connected()) && (!Terminated))
       {
       // send the state vector on to SignalProcessing
       ret1=fMain->sendingcomm->SendStateVector2CoreModule(statevector);
       // send data on to Signal Processing
       // create a GenericIntSignal out of a GenericSignal
       OutSignal=new GenericIntSignal(numcontrolsignals, 1);
       for (count=0; count<numcontrolsignals; count++)
        {
        if (running == 1)
           OutSignal->SetValue(count, 0, (short)(filters->SignalF->GetValue(count, 0)));
        else
           OutSignal->SetValue(count, 0, (short)0);
        }
       ret2=fMain->sendingcomm->SendData2CoreModule(OutSignal, NULL);
       delete OutSignal;
       if (!ret1 || !ret2)
          return(0);
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
  } catch(...) {}
 try
  {
  if (ReceivingSocket->Active) ReceivingSocket->Close();
  } catch(...) {}
 try
  {
  if (sendingcomm)
     if (sendingcomm->Connected()) sendingcomm->Terminate();
  } catch(...) {}
 try
  {
  if (corecomm)
     if (corecomm->Connected()) corecomm->Terminate();
  } catch(...) {}
}


// **************************************************************************
// Function:   ShutdownSystem
// Purpose:    Terminates the receiving thread
//             (which, in turn, shuts down all connections)
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TfMain::ShutdownSystem()
{
static inhere=false;

 if (inhere == true) return;
 inhere=true;

 // now, close all the other connections
 ShutdownConnections();

 corecomm=sendingcomm=NULL;

 paramlist.ClearParamList();
 statelist.ClearStateList();
 if (statevector) delete statevector;
 statevector=NULL;

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
        : TForm(Owner)
{
 statevector=NULL;
 filters=NULL;
 corecomm=NULL;
 sendingcomm=NULL;
}


// **************************************************************************
// Function:   bConnectClick
// Purpose:    when the Connect button is clicked on, start up the
//             signal processing module
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
//---------------------------------------------------------------------------
void __fastcall TfMain::bConnectClick(TObject *Sender)
{
 StartupSignalProcessing(eOperatorIP->Text);
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   StartupSignalProcessing
// Purpose:    The listening socket
//             is opened and the connection to the operator is established
//             we then publish all our parameters and all our states
// Parameters: N/A
// Returns:    0 ... on error (e.g., Operator could not be found)
//             1 ... on success
// **************************************************************************
int TfMain::StartupSignalProcessing(AnsiString connectto)
{
char paramstring[255];
char line[2048];
int  res, ret;

 // clear all parameters and states first
 paramlist.ClearParamList();
 statelist.ClearStateList();

 // create and initialize CORECOMM object
 // thereby creating connection to the operator
 corecomm=new CORECOMM(true);
 ret=corecomm->Initialize(connectto, 4001, fMain, COREMODULE_OPERATOR);
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

 // create instances of all the filters we'll be using
 if (filters) delete filters;
 filters=new FILTERS(&paramlist, &statelist);
 if (filters == NULL)
    corecomm->SendStatus("407 Error on constructor of FILTERS() ...");
 else
    if (filters->was_error)
       corecomm->SendStatus("407 Error on constructor of FILTERS() ...");

 // add parameters for socket connection
 // my receiving socket port number
 sprintf(paramstring, "System string SignalProcessingPort= %d 4200 1024 32768 // this module's listening port\n", ReceivingSocket->Socket->LocalPort);
 paramlist.AddParameter2List(paramstring, strlen(paramstring));
 // and IP address
 sprintf(paramstring, "System string SignalProcessingIP= %s 127.0.0.1 127.0.0.1 127.0.0.1 // this module's listening IP\n", corecomm->GetSocket()->LocalAddress.c_str());
 paramlist.AddParameter2List(paramstring, strlen(paramstring));
 // now, publish these parameters
 corecomm->PublishParameters(&paramlist);
 // and the states
 corecomm->PublishStates(&statelist);
 // send a confirmation message
 corecomm->SendStatus("100 SigProc waiting for configuration ...");
 return(1);
}


// **************************************************************************
// Function:   bDisconnectClick
// Purpose:    when the Disconnect button is clicked on, the receiving thread is terminated
//             (which, in turn, closes all other connections)
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::bDisconnectClick(TObject *Sender)
{
 if (corecomm) corecomm->Terminate();

 if (corecomm) delete corecomm;
 corecomm=NULL;
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

 if ((paramlist.GetParamPtr(PARAMNAME_APPLICATIONIP)) && (paramlist.GetParamPtr(PARAMNAME_APPLICATIONPORT)))
    {
    strcpy(destIP, paramlist.GetParamPtr(PARAMNAME_APPLICATIONIP)->GetValue());
    destport=atoi(paramlist.GetParamPtr(PARAMNAME_APPLICATIONPORT)->GetValue());
    if (sendingcomm) delete sendingcomm;
    sendingcomm=new CORECOMM(true);
    if (!sendingcomm) return(ERR_NOSOCKCONN);
    res=sendingcomm->Initialize(AnsiString(destIP), destport, (TForm *)fMain, COREMODULE_APPLICATION);
    if (res == 0) return(ERR_NOSOCKCONN);
    rSendingConnected->Checked=true;
    eSendingIP->Text=destIP;
    eSendingPort->Text=AnsiString(destport);
    corecomm->SendStatus("201 SignalProcessing module initialized");
    }
 else
    return(ERR_NOSOCKPARAM);

return(ERR_NOERR);
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
    if (coremessage->param.valid)
       paramlist.CloneParameter2List(&(coremessage->param));
    }

 // we received a state
 if (coremessage->GetDescriptor() == COREMSG_STATE)
    {
    if (coremessage->state.valid)
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
       // create connection to the user application
       if (InitializeConnections() == ERR_NOERR)
          {
          }
       else
          {
          }
       }
    }

 // delete coremessage;
}

// **************************************************************************
// Function:   ReceivingSocketClientDisconnect
// Purpose:    If the receiving socket (that receives data from EEGsource)
//             is being closed, just shut down the system
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::ReceivingSocketClientDisconnect(TObject *Sender, TCustomWinSocket *Socket)
{
 receiving_thread->Terminate();
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   FormClose
// Purpose:    if we close the window, shut the system down
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::FormClose(TObject *Sender, TCloseAction &Action)
{
 ShutdownSystem();
 if (statevector) delete statevector;
 if (filters)     delete filters;
 if (corecomm)    delete corecomm;
 if (sendingcomm) delete sendingcomm;
 statevector=NULL;
 filters=NULL;
 corecomm=NULL;
 sendingcomm=NULL;
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   ReceivingSocketGetThread
// Purpose:    if EEGsource connects to the SignalProcessing, a new thread
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
     if (StartupSignalProcessing(connectto) == 0)
        Application->Terminate();
     }
  }
}
//---------------------------------------------------------------------------

