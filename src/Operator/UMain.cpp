/******************************************************************************
 * Program:   OPERAT.EXE                                                      *
 * Module:    UMAIN.CPP                                                       *
 * Comment:   The main module of the operator program in BCI2000              *
 * Version:   0.25                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 02/10/2000 - First start                                           *
 * V0.02 - 02/16/2000 - Keepin on workin                                      *
 * V0.03 - 02/21/2000 - Assembling lists of parameters and states             *
 * V0.04 - 02/23/2000 - Sending all parameters and states back                *
 * V0.05 - 02/24/2000 - Constructing and publishing of the initial state vec. *
 * V0.06 - 03/02/2000 - Added the supplemental byte after the content descr.  *
 *         03/04/2000 - Removed a couple bugs related to the state vector     *
 * V0.07 - 03/04/2000 - Created signal processing and application modules     *
 * V0.08 - 03/09/2000 - Receiving status messages from core                   *
 * V0.09 - 03/16/2000 - Complete communication in between all core modules    *
 * V0.10 - 03/20/2000 - Updated to C++ Builder 5.0                            *
 * V0.11 - 03/24/2000 - made the socket connections blocking inst. of non-bl. *
 * V0.12 - 03/29/2000 - stable and commented version                          *
 * V0.13 - 04/19/2000 - test version                                          *
 * V0.14 - 05/16/2000 - added ability to set state 'Running'                  *
 * V0.15 - 06/09/2000 - implementing parameter visualization (for config)     *
 *         06/13/2000 - implemented data visualization                        *
 * V0.16 - 07/25/2000 - removed the main chart                                *
 * V0.17 - 08/08/2000 - being able to display visualization of float          *
 * V0.18 - 09/21/2000 - removed bug in visualization                          *
 *                      added parameter saving of visualization parameters    *
 * V0.19 - 12/15/2000 - added editable matrices                               *
 * V0.20 - 04/16/2001 - minor changes to work with coremodules V0.20          *
 *         04/30/2001 - allows for changes to parameters while cfg wind. open *
 * V0.21 - 05/11/2001 - timer at operator; introduced user levels             *
 * V0.211- 05/24/2001 - filtering capabilities for loading/saving parameters  *
 *         06/01/2001 - added system log                                      *
 *         06/05/2001 - added scripting capability                            *
 * V0.22 - 06/07/2001 - upgraded to shared libraries V0.17                    *
 *         06/26/2001 - added function buttons                                *
 *         06/28/2001 - dramatically sped up charting by replacing            *
 *                      TeeChart with our own code                            *
 * V0.23 - 08/15/2001 - added color scheme to output graph                    *
 * V0.24 - 01/03/2002 - minor stability improvements                          *
 * V0.25 - 01/11/2002 - fixed concurrency issues (i.e., can't send or receive *
 *                      to core modules at the same time)                     *
 * V0.26 - 04/11/2002 - Updated to Borland C++ Builder 6.0                    *
 *                      improved error handling                               *
 ******************************************************************************/

#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include <stdio.h>
#include <Registry.hpp>

#include "..\shared\defines.h"
#include "operator.h"

#include "USysStatus.h"
#include "USysLog.h"
#include "UScript.h"
#include "UAbout.h"
#include "UShowStates.h"
#include "UCoreMessage.h"
#include "UOperatorCfg.h"
#include "UPreferences.h"
#include "UConnectionInfo.h"
#include "UVisConfig.h"
#include "UOperatorUtils.h"

#include "UMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "Trayicon"
#pragma link "trayicon"
#pragma resource "*.dfm"

TfMain *fMain;

SYSSTATUS       sysstatus;
TCriticalSection *sendrecv_critsec=NULL;

//---------------------------------------------------------------------------
class TCoreRecvThread : public  TServerClientThread
{
  protected:
        int     coretype;
  public:
    __fastcall TCoreRecvThread(bool CreateSuspended, TServerClientWinSocket* ASocket, int coretype);
    void __fastcall ClientExecute( void );
};

TCoreRecvThread *source_thread, *sigproc_thread, *application_thread;

//---------------------------------------------------------------------------
__fastcall TCoreRecvThread::TCoreRecvThread(bool CreateSuspended, TServerClientWinSocket* ASocket, int new_coretype)  : TServerClientThread( CreateSuspended, ASocket )
{
 coretype=new_coretype;
}
//---------------------------------------------------------------------------

void __fastcall TCoreRecvThread::ClientExecute( void )
{
int                     ret;
TWinSocketStream        *pStream;
COREMESSAGE             *coremessage;

  pStream=new TWinSocketStream(ClientSocket, 5000);
  while(!Terminated && ClientSocket->Connected )
   {
   try
    {
    if (pStream->WaitForData(1000))
       {
       coremessage=new COREMESSAGE;
       // receive Core Message
       // (do not receive while we send; concurrency problems)
       sendrecv_critsec->Acquire();
       ret=coremessage->ReceiveCoreMessage(pStream);
       sendrecv_critsec->Release();
       if (ret != ERRCORE_NOERR)
          {
          delete coremessage;
          if (!Terminated) Terminate();
          }
       else
          {
          coremessage->ParseMessage();
          fMain->cur_coremessage=coremessage;
          fMain->cur_coretype=coretype;
          try {
          fMain->HandleCoreMessage(coremessage, coretype);
          // Synchronize(fMain->HandleCoreMessage);
          } catch( TooGeneralCatch& ) {;}
          delete coremessage;
          if (coretype == COREMODULE_EEGSOURCE)   sysstatus.NumMessagesRecv1++;
          if (coretype == COREMODULE_SIGPROC)     sysstatus.NumMessagesRecv2++;
          if (coretype == COREMODULE_APPLICATION) sysstatus.NumMessagesRecv3++;
          }
       }
    }
   catch( TooGeneralCatch& )
    {
    if (!Terminated) Terminate();
    }
   }

  if (coretype == COREMODULE_EEGSOURCE)
     {
     source_thread=NULL;
     // Synchronize();
     if (fMain->SourceSocket->Socket->ActiveConnections > 0)
        if (fMain->SourceSocket->Socket->Connections[0])
           fMain->SourceSocket->Socket->Connections[0]->Close();
     }
  if (coretype == COREMODULE_SIGPROC)
     {
     sigproc_thread=NULL;
     if (fMain->SigProcSocket->Socket->ActiveConnections > 0)
        if (fMain->SigProcSocket->Socket->Connections[0])
           fMain->SigProcSocket->Socket->Connections[0]->Close();
      }
  if (coretype == COREMODULE_APPLICATION)
      {
      application_thread=NULL;
      if (fMain->ApplicationSocket->Socket->ActiveConnections > 0)
         if (fMain->ApplicationSocket->Socket->Connections[0])
            fMain->ApplicationSocket->Socket->Connections[0]->Close();
      }

  delete pStream;
}
//---------------------------------------------------------------------------


void TfMain::SendSysCommand(char *syscmdbuf, TCustomWinSocket *socket)
{
FILE *fp;
COREMESSAGE *coremessage;

 // (do not receive while we send; concurrency problems)
 sendrecv_critsec->Acquire();

 // send a system command to a module
 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_SYSCMD);
 coremessage->SetLength((unsigned short)strlen(syscmdbuf)+1);
 sprintf(coremessage->GetBufPtr(), syscmdbuf, strlen(syscmdbuf));
 coremessage->SendCoreMessage(socket);
 delete coremessage;

 sendrecv_critsec->Release();
}


void TfMain::ShutdownSystem()
{
 if (fMain->SourceSocket->Socket->ActiveConnections > 0)
    if (fMain->SourceSocket->Socket->Connections[0])
       fMain->SourceSocket->Socket->Connections[0]->Close();
 if (fMain->SigProcSocket->Socket->ActiveConnections > 0)
    if (fMain->SigProcSocket->Socket->Connections[0])
       fMain->SigProcSocket->Socket->Connections[0]->Close();
 if (fMain->ApplicationSocket->Socket->ActiveConnections > 0)
    if (fMain->ApplicationSocket->Socket->Connections[0])
       fMain->ApplicationSocket->Socket->Connections[0]->Close();
}

//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
: TForm(Owner),
  syslog( NULL )
{
 sendrecv_critsec=new TCriticalSection();
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   ApplicationSocketClientDisconnect
// Purpose:    is called when the application core module disconnects
// Parameters: Socket - pointer to the socket that connects to the client
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::ApplicationSocketClientDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
 if (sysstatus.ApplicationConnected)
    {
    sysstatus.ApplicationConnected=false;
    fConnectionInfo->cApplicationConnected->Checked=false;
    fConnectionInfo->tApplicationConnected->Caption="N/A";
    }
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   SigProcSocketClientDisconnect
// Purpose:    is called when the signal processing core module disconnects
// Parameters: Socket - pointer to the socket that connects to the client
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::SigProcSocketClientDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
 if (sysstatus.SigProcConnected)
    {
    sysstatus.SigProcConnected=false;
    fConnectionInfo->cSigProcConnected->Checked=false;
    fConnectionInfo->tSigProcConnected->Caption="N/A";
    }
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   SourceSocketClientDisconnect
// Purpose:    is called when the EEGsource core module disconnects
// Parameters: Socket - pointer to the socket that connects to the client
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::SourceSocketClientDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
 if (sysstatus.SourceConnected)
    {
    sysstatus.SourceConnected=false;
    fConnectionInfo->cSourceConnected->Checked=false;
    fConnectionInfo->tSourceConnected->Caption="N/A";
    }
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   BroadcastParameters
// Purpose:    sends all parameters (i.e., param lines) to all connected core modules
// Parameters: N/A
// Returns:    always ERR_NOERR
// **************************************************************************
int TfMain::BroadcastParameters()
{
COREMESSAGE     *coremessage;
int             num_param, i, ret;
PARAM           *cur_param;
AnsiString      paramline;
FILE            *fp;
TWinSocketStream        *pSourceStream, *pSigProcStream, *pApplicationStream;

 if ((!sysstatus.SourceConnected) || (!sysstatus.SigProcConnected) || (!sysstatus.ApplicationConnected))
    return(ERR_NOERR);

 // (do not receive while we send; concurrency problems)
 sendrecv_critsec->Acquire();

 pSourceStream=new TWinSocketStream(sysstatus.SourceSocket, 5000);
 pSigProcStream=new TWinSocketStream(sysstatus.SigProcSocket, 5000);
 pApplicationStream=new TWinSocketStream(sysstatus.ApplicationSocket, 5000);

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_PARAMETER);

 num_param=paramlist.GetNumParameters();
 for (i=0; i<num_param; i++)
  {
  cur_param=paramlist.GetParamPtr(i);
  if (cur_param)
     {
     paramline=cur_param->GetParamLine();

     coremessage->SetLength((unsigned short)strlen(paramline.c_str()));
     strncpy(coremessage->GetBufPtr(), paramline.c_str(), strlen(paramline.c_str()));

     coremessage->SendCoreMessage(pSourceStream);
     coremessage->SendCoreMessage(pSigProcStream);
     coremessage->SendCoreMessage(pApplicationStream);

     sysstatus.NumMessagesSent1++;
     sysstatus.NumParametersSent1++;
     sysstatus.NumMessagesSent2++;
     sysstatus.NumParametersSent2++;
     sysstatus.NumMessagesSent3++;
     sysstatus.NumParametersSent3++;
     }
  }

 sendrecv_critsec->Release();

 // at the very end, send EndOfParameter to terminate this phase
 // broadcast this final state to all the modules that are connected
 if (sysstatus.SourceConnected)
    {
    SendSysCommand("EndOfParameter", sysstatus.SourceSocket);
    sysstatus.NumMessagesSent1++;
    }
 if (sysstatus.SigProcConnected)
    {
    SendSysCommand("EndOfParameter", sysstatus.SigProcSocket);
    sysstatus.NumMessagesSent2++;
    }
 if (sysstatus.ApplicationConnected)
    {
    SendSysCommand("EndOfParameter", sysstatus.ApplicationSocket);
    sysstatus.NumMessagesSent3++;
    }

 delete pSourceStream;
 delete pSigProcStream;
 delete pApplicationStream;
 delete coremessage;

 return(ERR_NOERR);
}


// **************************************************************************
// Function:   BroadcastStates
// Purpose:    sends all states (i.e., state lines) to all connected core modules
// Parameters: N/A
// Returns:    always ERR_NOERR
// **************************************************************************
int TfMain::BroadcastStates()
{
COREMESSAGE     *coremessage;
STATE           *cur_state;
int             num_state, i;
char            statelinebuf[LENGTH_STATELINE];

 // (do not receive while we send; concurrency problems)
 sendrecv_critsec->Acquire();

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_STATE);

 num_state=statelist.GetNumStates();
 for (i=0; i<num_state; i++)
  {
  cur_state=statelist.GetStatePtr(i);
  if (cur_state)
     {
     sprintf(statelinebuf, "%s\r\n", cur_state->GetStateLine());
     coremessage->SetLength((unsigned short)strlen(statelinebuf));
     strncpy(coremessage->GetBufPtr(), statelinebuf, strlen(statelinebuf));

     // broadcast the states to all the modules that are connected
     // (should be all of them anyways)
     if (sysstatus.SourceConnected)
        {
        coremessage->SendCoreMessage(sysstatus.SourceSocket);
        sysstatus.NumMessagesSent1++;
        sysstatus.NumStatesSent1++;
        }
     if (sysstatus.SigProcConnected)
        {
        coremessage->SendCoreMessage(sysstatus.SigProcSocket);
        sysstatus.NumMessagesSent2++;
        sysstatus.NumStatesSent2++;
        }
     if (sysstatus.ApplicationConnected)
        {
        coremessage->SendCoreMessage(sysstatus.ApplicationSocket);
        sysstatus.NumMessagesSent3++;
        sysstatus.NumStatesSent3++;
        }
     }
  }

 delete coremessage;

 sendrecv_critsec->Release();

 // at the very end, send EndOfState to terminate this phase
 // broadcast this final state to all the modules that are connected
 if (sysstatus.SourceConnected)
    {
    SendSysCommand("EndOfState", sysstatus.SourceSocket);
    sysstatus.NumMessagesSent1++;
    sysstatus.NumStatesSent1++;
    }
 if (sysstatus.SigProcConnected)
    {
    SendSysCommand("EndOfState", sysstatus.SigProcSocket);
    sysstatus.NumMessagesSent2++;
    sysstatus.NumStatesSent2++;
    }
 if (sysstatus.ApplicationConnected)
    {
    SendSysCommand("EndOfState", sysstatus.ApplicationSocket);
    sysstatus.NumMessagesSent3++;
    sysstatus.NumStatesSent3++;
    }

 return(ERR_NOERR);
}


// **************************************************************************
// Function:   BroadcastStateVector
// Purpose:    sends the initial state vector to all connected core modules
// Parameters: N/A
// Returns:    always ERR_NOERR
// **************************************************************************
int TfMain::BroadcastStateVector(STATEVECTOR *my_state_vector)
{
COREMESSAGE     *coremessage;

 // (do not receive while we send; concurrency problems)
 sendrecv_critsec->Acquire();

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_STATEVECTOR);

 coremessage->SetLength((unsigned short)my_state_vector->GetStateVectorLength());
 strncpy(coremessage->GetBufPtr(), (char *)my_state_vector->GetStateVectorPtr(), my_state_vector->GetStateVectorLength());

 // broadcast the states to all the modules that are connected
 // (should be all of them anyways)
 if (sysstatus.SourceConnected)
    {
    coremessage->SendCoreMessage(sysstatus.SourceSocket);
    sysstatus.NumMessagesSent1++;
    sysstatus.NumStateVecsSent1++;
    }
 if (sysstatus.SigProcConnected)
    {
    coremessage->SendCoreMessage(sysstatus.SigProcSocket);
    sysstatus.NumMessagesSent2++;
    sysstatus.NumStateVecsSent2++;
    }
 if (sysstatus.ApplicationConnected)
    {
    coremessage->SendCoreMessage(sysstatus.ApplicationSocket);
    sysstatus.NumMessagesSent3++;
    sysstatus.NumStateVecsSent3++;
    }

 sendrecv_critsec->Release();

 delete coremessage;
 return(ERR_NOERR);
}


// **************************************************************************
// Function:   UpdateState
// Purpose:    Send an updated state to the EEGsource
// Parameters: statename - name of the state to modify
//             newvalue - new value for this state
// Returns:    ERR_NOERR - if everything OK
//             ERR_STATENOTFOUND - if the state was not found
//             ERR_SOURCENOTCONNECTED - the EEGsource is not connected
// **************************************************************************
int TfMain::UpdateState(char *statename, unsigned short newvalue)
{
int ret;

 if (sysstatus.SourceConnected)
    {
    // (do not receive while we send; concurrency problems)
    sendrecv_critsec->Acquire();
    ret=OperatorUtils::UpdateState(&statelist,statename, newvalue, sysstatus.SourceSocket);
    // enable reception
    sendrecv_critsec->Release();
    sysstatus.NumMessagesSent1++;
    sysstatus.NumStatesSent1++;
    }
 else
    ret=ERR_SOURCENOTCONNECTED;

return(ret);
}


// to be called using Synchronize
void __fastcall TfMain::HandleCoreMessage(void)
{
 HandleCoreMessage(cur_coremessage, cur_coretype);
}


// **************************************************************************
// Function:   HandleCoreMessage
// Purpose:    handles a message that it received from a core module
//             this procedure assumes that message->ParseMessage() has been
//             called first to pre-process the data in the message
//             (e.g., in case of a parameter, parse the parameter line)
// Parameters: message - a pointer to the coremessage
//             module  - descriptor, describing which core module it came from (defined in defines.h)
// Returns:    always ERR_NOERR
// **************************************************************************
int TfMain::HandleCoreMessage(COREMESSAGE *message, int module)
{
AnsiString              section, type, name;
STATEVECTOR             *initial_state_vector;
PARAM   *temp_param;
VISUAL  *vis_ptr;
char    buf[255];
int     sample, channel, i, j;

 // it is a message containing visualization data
 if (message->GetDescriptor() == COREMSG_DATA)
    {
    /* // it is visualization configuration
    if (message->GetDescriptor() == VISTYPE_VISCFG)
       {
       // if the configuration contains cfg re. a graph, open a window, if necessary
       vis_ptr=viscfglist.GetVisCfgPtr(message->visualization.GetSourceID());
       if (vis_ptr == NULL)
          SendMessage(fMain->Handle, WINDOW_OPEN, message->visualization.GetSourceID(), VISTYPE_GRAPH);
       } */

    // it is a graph that was coming in
    if (message->GetSuppDescriptor() == VISTYPE_GRAPH)
       {
       // the graph contains integers
       if ((message->visualization.GetDataType() == DATATYPE_INTEGER) || (message->visualization.GetDataType() == DATATYPE_FLOAT))
          {
          // if we did not receive anything from this source yet, open another window
          // we send a message to the main thread that will open the window
          // we have to send a message, since this code is executed in a separate thread,
          // and VCL components have to be dealt with in the main thread
          // it only works, if I open the window in a message handler
          // I think that it should also work, if I used Synchronize(OpenVisual()), but haven't tried it yet
          vis_ptr=viscfglist.GetVisCfgPtr(message->visualization.GetSourceID());
          if (vis_ptr == NULL)
             SendMessage(fMain->Handle, WINDOW_OPEN, message->visualization.GetSourceID(), VISTYPE_GRAPH);
          vis_ptr=viscfglist.GetVisCfgPtr(message->visualization.GetSourceID());
          if (vis_ptr != NULL)
             {
             // if visual object exists but window is closed, reopen it
             if (!vis_ptr->form->Visible) vis_ptr->form->Visible=true;
             if (message->visualization.GetDataType() == DATATYPE_INTEGER)
                vis_ptr->RenderData(message->visualization.GetIntSignal());
             if (message->visualization.GetDataType() == DATATYPE_FLOAT)
                vis_ptr->RenderData(message->visualization.GetSignal());
#ifndef NEW_DOUBLEBUF_SCHEME
             // now, send a message to the main thread
             // which will update the actual image; can use PostMessage so that it returns right away
             // I've tried forever to get the blitting to run within RenderData and couldn't get it to work
             // even though I used Critical Sections and canvas->Lock; have no idea why
             // that's why I have to use this crazy method
             PostMessage(fMain->Handle, WINDOW_RENDER, message->visualization.GetSourceID(), 0);
#endif // NEW_DOUBLEBUF_SCHEME
             }
          }
       }

    // it is a memo that was coming in
    if (message->GetSuppDescriptor() == VISTYPE_MEMO)
       {
       // if we did not receive anything from this source yet, open another window
       // we send a message to the main thread that will open the window
       // since this is not the main thread, opening a VCL object needs to be synchronized
       // could potentially use Synchronize(), but here I use a windows message
       vis_ptr=viscfglist.GetVisCfgPtr(message->visualization.GetSourceID());
       if (vis_ptr == NULL)
          SendMessage(fMain->Handle, WINDOW_OPEN, message->visualization.GetSourceID(), VISTYPE_MEMO);
       vis_ptr=viscfglist.GetVisCfgPtr(message->visualization.GetSourceID());
       if (vis_ptr != NULL)
          {
          // if visual object exists but window is closed, reopen it
          if (!vis_ptr->form->Visible) vis_ptr->form->Visible=true;
          vis_ptr->RenderMemo(message->visualization.GetMemoText());
          }
       }

    // it comes from the EEG source module
    if (module == COREMODULE_EEGSOURCE)
       sysstatus.NumDataRecv1++;
    // it comes from the signal processing module
    if (module == COREMODULE_SIGPROC)
       sysstatus.NumDataRecv2++;
    // it comes from the application module
    if (module == COREMODULE_APPLICATION)
       sysstatus.NumDataRecv3++;
    }
 // it is a parameter message
 if (message->GetDescriptor() == COREMSG_PARAMETER)
    {
    // now, add the parameter to the list of parameters
    // (if it is a valid parameter
    if (message->param.Valid())
       {
       paramlist.CloneParameter2List(&(message->param));
       // refresh this parameter on the screen (in case the cfg window is open)
       try
        {
        fConfig->RenderParameter(paramlist.GetParamPtr(message->param.GetName()));
        } catch ( TooGeneralCatch& ) {};
       }

    // it comes from the EEG source module
    if (module == COREMODULE_EEGSOURCE)
       sysstatus.NumParametersRecv1++;
    // it comes from the signal processing module
    if (module == COREMODULE_SIGPROC)
       sysstatus.NumParametersRecv2++;
    // it comes from the application module
    if (module == COREMODULE_APPLICATION)
       sysstatus.NumParametersRecv3++;
    }


 // it is a status message
 if (message->GetDescriptor() == COREMSG_STATUS)
    {
    // if we receive a warning message, add a line to the system log and bring it to front
    if ((message->status.GetCode() >= 300) && (message->status.GetCode() < 400))
       {
       syslog->AddSysLogEntry(message->status.GetStatus(), SYSLOGENTRYMODE_WARNING);
       syslog->ShowSysLog();
       }
    // if we receive an error message, add a line to the system log and bring it to front
    if ((message->status.GetCode() >= 400) && (message->status.GetCode() < 500))
       {
       syslog->AddSysLogEntry(message->status.GetStatus(), SYSLOGENTRYMODE_ERROR);
       syslog->ShowSysLog();
       }
    if (module == COREMODULE_EEGSOURCE)
       {
       sysstatus.EEGsourceStatus=message->status.GetStatus();
       sysstatus.EEGsourceStatusReceived=true;
       }
    if (module == COREMODULE_SIGPROC)
       {
       sysstatus.SigProcStatus=message->status.GetStatus();
       sysstatus.SigProcStatusReceived=true;
       }
    if (module == COREMODULE_APPLICATION)
       {
       sysstatus.ApplicationStatus=message->status.GetStatus();
       sysstatus.ApplicationStatusReceived=true;
       }

    // if the operator received successful status messages from
    // all core modules, then this is the end of the initialization phase
    if ((sysstatus.SystemState == STATE_INITIALIZATION) && (message->status.GetCode() < 300))
       {
       if (module == COREMODULE_EEGSOURCE)
          {
          sysstatus.EEGsourceINI=true;
          syslog->AddSysLogEntry("Source confirmed new parameters ...");
          }
       if (module == COREMODULE_SIGPROC)
          {
          sysstatus.SigProcINI=true;
          syslog->AddSysLogEntry("Signal Processing confirmed new parameters ...");
          }
       if (module == COREMODULE_APPLICATION)
          {
          sysstatus.ApplicationINI=true;
          syslog->AddSysLogEntry("User Application confirmed new parameters ...");
          }

       if ((sysstatus.EEGsourceINI) && (sysstatus.SigProcINI) && (sysstatus.ApplicationINI))
          {
          bRunSystem->Enabled=true;
          bReset->Enabled=true;
          // UpdateState("Running", 1);                    // this sends the state running with 1 to the EEGsource and starts operation
          sysstatus.SystemState=STATE_RUNNING;
          }
       }
    }

#if 0
    // Anyway, we want a working Quit button if any message arrived from one of the modules,
    // and we need to go back to STATE_INFORMATION if there was an error.
    if( sysstatus.SystemState == STATE_INITIALIZATION
        && sysstatus.EEGsourceStatusReceived
        && sysstatus.SigProcStatusReceived
        && sysstatus.ApplicationStatusReceived
        && !( sysstatus.EEGsourceINI && sysstatus.SigProcINI && sysstatus.ApplicationINI ) )
    {
      bRunSystem->Enabled = false;
      bReset->Enabled = true;
      sysstatus.SystemState = STATE_INFORMATION;
    }
#endif

 // it is a state message
 if (message->GetDescriptor() == COREMSG_STATE)
    {
    if (message->state.Valid())
       {
       // if we received any state and we are in STATE_IDLE, switch to publishing phase
       if (sysstatus.SystemState == STATE_IDLE)
          sysstatus.SystemState=STATE_PUBLISHING;

       // it comes from the source module
       if (module == COREMODULE_EEGSOURCE)
          sysstatus.NumStatesRecv1++;
       // it comes from the signal processing module
       if (module == COREMODULE_SIGPROC)
          sysstatus.NumStatesRecv2++;
       // it comes from the application module
       if (module == COREMODULE_APPLICATION)
          sysstatus.NumStatesRecv3++;

       statelist.AddState2List(&(message->state));  // now, add the state to the list of states
       }
    }

 // it is a system command
 if (message->GetDescriptor() == COREMSG_SYSCMD)
    {
    if (stricmp(message->syscmd.GetSysCmd(), "Reset") == 0)
       SendMessage(fMain->Handle, RESET_OPERATOR, 0, 0);

    if (stricmp(message->syscmd.GetSysCmd(), "Suspend") == 0)
       {
       if (sysstatus.SystemState != STATE_SUSPENDED)
          StartSuspendSystem(false);
       }

    // if we received 'EndOfParameter', i.e., all parameters,
    // then let's sort the parameters in the parameter list by name
    if (strcmp(message->syscmd.GetSysCmd(), "EndOfParameter") == 0)
       paramlist.Sort();

    // the operator receiving 'EndOfState' marks the end of the publishing phase
    if (strcmp(message->syscmd.GetSysCmd(), "EndOfState") == 0)
       {
       if (module == COREMODULE_EEGSOURCE)   sysstatus.EEGsourceEOS=true;
       if (module == COREMODULE_SIGPROC)     sysstatus.SigProcEOS=true;
       if (module == COREMODULE_APPLICATION) sysstatus.ApplicationEOS=true;
       // if we received EndOfStates from all the modules, then make the transition to the next system state
       if ((sysstatus.EEGsourceEOS) && (sysstatus.SigProcEOS) && (sysstatus.ApplicationEOS) && (sysstatus.SystemState == STATE_PUBLISHING))
          {
          // execute the script after all modules are connected ...
          if (AnsiString(preferences.Script_AfterModulesConnected).Trim() != "")
             {
             syslog->AddSysLogEntry("Executing script after all modules connected ...");
             script.Initialize(&paramlist, &statelist, syslog, sysstatus.SourceSocket);
             script.ExecuteScript(preferences.Script_AfterModulesConnected);
             }

          sysstatus.SystemState=STATE_INFORMATION;
          // create the initial state vector and set the locations in each state
          initial_state_vector=new STATEVECTOR(&statelist);
          // add the parameter StateVectorLength to the list of parameters
          sprintf(buf, "%d", initial_state_vector->GetStateVectorLength());
          temp_param=new PARAM("StateVectorLength", "System", "int", buf, "16", "1", "30", "length of the state vector in bytes");
          paramlist.MoveParameter2List(temp_param);
          paramlist.Sort();
          
          // now there is the possibility to configure the system
          // therefore, 'turn' the config button on
          bConfig->Enabled=true;
          bSetConfig->Enabled=true;

          delete initial_state_vector;
          }
       }
    }

 return(ERR_NOERR);
}


// **************************************************************************
// Function:   ScrUpdateTimerTimer
// Purpose:    this is the routine for the screen update timer
//             it is called every 60ms to update the screen
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::ScrUpdateTimerTimer(TObject *Sender)
{
 fConnectionInfo->tNumMessagesRecv1->Caption=AnsiString(sysstatus.NumMessagesRecv1);
 fConnectionInfo->tNumMessagesRecv2->Caption=AnsiString(sysstatus.NumMessagesRecv2);
 fConnectionInfo->tNumMessagesRecv3->Caption=AnsiString(sysstatus.NumMessagesRecv3);
 fConnectionInfo->tNumParametersRecv1->Caption=AnsiString(sysstatus.NumParametersRecv1);
 fConnectionInfo->tNumParametersRecv2->Caption=AnsiString(sysstatus.NumParametersRecv2);
 fConnectionInfo->tNumParametersRecv3->Caption=AnsiString(sysstatus.NumParametersRecv3);
 fConnectionInfo->tNumStatesRecv1->Caption=AnsiString(sysstatus.NumStatesRecv1);
 fConnectionInfo->tNumStatesRecv2->Caption=AnsiString(sysstatus.NumStatesRecv2);
 fConnectionInfo->tNumStatesRecv3->Caption=AnsiString(sysstatus.NumStatesRecv3);
 fConnectionInfo->tNumDataRecv1->Caption=AnsiString(sysstatus.NumDataRecv1);
 fConnectionInfo->tNumDataRecv2->Caption=AnsiString(sysstatus.NumDataRecv2);
 fConnectionInfo->tNumDataRecv3->Caption=AnsiString(sysstatus.NumDataRecv3);

 fConnectionInfo->tNumMessagesSent1->Caption=AnsiString(sysstatus.NumMessagesSent1);
 fConnectionInfo->tNumMessagesSent2->Caption=AnsiString(sysstatus.NumMessagesSent2);
 fConnectionInfo->tNumMessagesSent3->Caption=AnsiString(sysstatus.NumMessagesSent3);
 fConnectionInfo->tNumParametersSent1->Caption=AnsiString(sysstatus.NumParametersSent1);
 fConnectionInfo->tNumParametersSent2->Caption=AnsiString(sysstatus.NumParametersSent2);
 fConnectionInfo->tNumParametersSent3->Caption=AnsiString(sysstatus.NumParametersSent3);
 fConnectionInfo->tNumStatesSent1->Caption=AnsiString(sysstatus.NumStatesSent1);
 fConnectionInfo->tNumStatesSent2->Caption=AnsiString(sysstatus.NumStatesSent2);
 fConnectionInfo->tNumStatesSent3->Caption=AnsiString(sysstatus.NumStatesSent3);
 fConnectionInfo->tNumStateVecsSent1->Caption=AnsiString(sysstatus.NumStateVecsSent1);
 fConnectionInfo->tNumStateVecsSent2->Caption=AnsiString(sysstatus.NumStateVecsSent2);
 fConnectionInfo->tNumStateVecsSent3->Caption=AnsiString(sysstatus.NumStateVecsSent3);

 if (sysstatus.SystemState == STATE_IDLE)
    StatusBar->Panels->Items[PANEL_SYSSTATUS]->Text = "System Status: < idle >";
 if (sysstatus.SystemState == STATE_PUBLISHING)
    StatusBar->Panels->Items[PANEL_SYSSTATUS]->Text = "System Status: Publishing Phase ...";
 if (sysstatus.SystemState == STATE_INFORMATION)
    StatusBar->Panels->Items[PANEL_SYSSTATUS]->Text = "System Status: Information Phase ...";
 if (sysstatus.SystemState == STATE_INITIALIZATION)
    StatusBar->Panels->Items[PANEL_SYSSTATUS]->Text = "System Status: Initialization Phase ...";
 if (sysstatus.SystemState == STATE_RUNNING)
    StatusBar->Panels->Items[PANEL_SYSSTATUS]->Text = "System Status: System Running ...";

 StatusBar->Panels->Items[PANEL_EEGSOURCE]->Text=sysstatus.EEGsourceStatus;
 StatusBar->Panels->Items[PANEL_SIGPROC]->Text=sysstatus.SigProcStatus;
 StatusBar->Panels->Items[PANEL_APPLICATION]->Text=sysstatus.ApplicationStatus;
}
//---------------------------------------------------------------------------



void __fastcall TfMain::OpenVisual(TMessage &Message)
{
int     sourceID, windowtype;

 sourceID=Message.WParam;
 windowtype=Message.LParam;

 // if we wanted to open a visualization window, we have to do this in the main thread
 // otherwise, windows message processing gets screwed up
 // could also use the Synchronize method
 if (viscfglist.GetVisCfgPtr(sourceID) == NULL)
     viscfglist.Add(new VISUAL(sourceID, windowtype));
}


#ifndef NEW_DOUBLEBUF_SCHEME
void __fastcall TfMain::Render(TMessage &Message)
{
int     sourceID, windowtype;
VISUAL  *vis_ptr;

 sourceID=Message.WParam;
 vis_ptr=viscfglist.GetVisCfgPtr(sourceID);
 if (!vis_ptr) return;

 vis_ptr->form->Canvas->Lock();
 vis_ptr->form->Canvas->CopyRect(Rect(0, 0, vis_ptr->form->ClientWidth, vis_ptr->form->ClientHeight), vis_ptr->bitmap->Canvas, Rect(0, 0, vis_ptr->form->ClientWidth, vis_ptr->form->ClientHeight));
 vis_ptr->form->Canvas->Unlock();
}
#endif // NEW_DOUBLEBUF_SCHEME


void __fastcall TfMain::DoResetOperator(TMessage &Message)
{
 ResetOperator();
 Close();
}


void TfMain::ResetOperator()
{
int     i;
GenericVisualization   *cur_vis;

 if (source_thread)  source_thread->Terminate();
 if (sigproc_thread) sigproc_thread->Terminate();
 if (application_thread) application_thread->Terminate();
 sysstatus.SourceConnected=false;
 fConnectionInfo->cSourceConnected->Checked=false;
 fConnectionInfo->tSourceConnected->Caption="N/A";
 sysstatus.SigProcConnected=false;
 fConnectionInfo->cSigProcConnected->Checked=false;
 fConnectionInfo->tSigProcConnected->Caption="N/A";
 sysstatus.ApplicationConnected=false;
 fConnectionInfo->cApplicationConnected->Checked=false;
 fConnectionInfo->tApplicationConnected->Caption="N/A";

 // wait a little, so that remaining messages, which are still 'on the way'
 // arrive at the operator ProcessMessages() then ensures that they'll be
 // processed, before we delete everything
 Sleep(500);
 Application->ProcessMessages();

 bRunSystem->Caption="Start";
 bRunSystem->Enabled=false;

 // close the parameter configuration window
 bConfig->Enabled=false;
 bSetConfig->Enabled=false;
 fConfig->Close();

 // delete all the windows currently visualized
 viscfglist.DeleteAllVisuals();

 paramlist.ClearParamList();
 statelist.ClearStateList();

 sysstatus.ResetSysStatus();
}


void TfMain::QuitOperator()
{
int             ret;

 ret=Application->MessageBox("Do you really want to quit BCI2000 ?", "Question", MB_YESNO);
 if (ret != ID_YES) return;

 // execute the on-exit script...
 if (AnsiString(preferences.Script_OnExit).Trim() != "")
    {
    syslog->AddSysLogEntry("Executing on-exit script ...");
    script.Initialize(&paramlist, &statelist, syslog, sysstatus.SourceSocket);
    script.ExecuteScript(preferences.Script_OnExit);
    }

 // turn off the screen update time
 ActiveTimer->Enabled=false;

 if (syslog) syslog->Close( true );
 syslog=NULL;

 // store the settings in the ini file
 preferences.SetDefaultSettings();

 // if the system is not running, just reset the operator
 if (sysstatus.SystemState != STATE_RUNNING)
    {
    ResetOperator();
    Close();
    }

 // send an system command 'Reset' to the EEGsource
 // will force the system to stop
 if (sysstatus.SourceConnected)
    {
    SendSysCommand("Reset", sysstatus.SourceSocket);
    sysstatus.NumMessagesSent1++;
    sysstatus.NumStatesSent1++;
    }
}


void __fastcall TfMain::bResetClick(TObject *Sender)
{
 QuitOperator();
}
//---------------------------------------------------------------------------



void __fastcall TfMain::SourceSocketClientError(TObject *Sender,
      TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
 Application->MessageBox("ERROR: ERROR IN COMMUNICATION W/SOURCE", "ERROR", ID_YES);
}
//---------------------------------------------------------------------------

void __fastcall TfMain::SigProcSocketClientError(TObject *Sender,
      TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
 Application->MessageBox("ERROR: ERROR IN COMMUNICATION W/SIGPROC", "ERROR", ID_YES);
}
//---------------------------------------------------------------------------

void __fastcall TfMain::ApplicationSocketClientError(TObject *Sender,
      TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
 Application->MessageBox("ERROR: ERROR IN COMMUNICATION W/APPLICATION", "ERROR", ID_YES);
}
//---------------------------------------------------------------------------


void __fastcall TfMain::SourceSocketGetThread(TObject *Sender,
      TServerClientWinSocket *ClientSocket,
      TServerClientThread *&SocketThread)
{
 source_thread=new TCoreRecvThread(false, ClientSocket, COREMODULE_EEGSOURCE);
 SocketThread=source_thread;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::SourceSocketAccept(TObject *Sender,
      TCustomWinSocket *Socket)
{
 sysstatus.SourceConnected=true;
 sysstatus.SourceSocket=Socket;
 fConnectionInfo->cSourceConnected->Checked=true;
 fConnectionInfo->tSourceConnected->Caption=Socket->RemoteAddress+":"+AnsiString(Socket->RemotePort);
}
//---------------------------------------------------------------------------

void __fastcall TfMain::SigProcSocketGetThread(TObject *Sender,
      TServerClientWinSocket *ClientSocket,
      TServerClientThread *&SocketThread)
{
 sigproc_thread=new TCoreRecvThread(false, ClientSocket, COREMODULE_SIGPROC);
 SocketThread=sigproc_thread;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::SigProcSocketAccept(TObject *Sender,
      TCustomWinSocket *Socket)
{
 sysstatus.SigProcConnected=true;
 sysstatus.SigProcSocket=Socket;
 fConnectionInfo->cSigProcConnected->Checked=true;
 fConnectionInfo->tSigProcConnected->Caption=Socket->RemoteAddress+":"+AnsiString(Socket->RemotePort);
}
//---------------------------------------------------------------------------

void __fastcall TfMain::ApplicationSocketAccept(TObject *Sender,
      TCustomWinSocket *Socket)
{
 sysstatus.ApplicationConnected=true;
 sysstatus.ApplicationSocket=Socket;
 fConnectionInfo->cApplicationConnected->Checked=true;
 fConnectionInfo->tApplicationConnected->Caption=Socket->RemoteAddress+":"+AnsiString(Socket->RemotePort);
}
//---------------------------------------------------------------------------

void __fastcall TfMain::ApplicationSocketGetThread(TObject *Sender,
      TServerClientWinSocket *ClientSocket,
      TServerClientThread *&SocketThread)
{
 application_thread=new TCoreRecvThread(false, ClientSocket, COREMODULE_APPLICATION);
 SocketThread=application_thread;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::FormClose(TObject *Sender, TCloseAction &Action)
{
 if( syslog && !syslog->Close() )
 {
   Action = caNone;
   return;
 }
 delete syslog;
 syslog = NULL;
 
 ShutdownSystem();

 if (sendrecv_critsec) delete sendrecv_critsec;
 sendrecv_critsec=NULL;
}
//---------------------------------------------------------------------------


void TfMain::StartSuspendSystem(bool update)
{
unsigned short     suspended, running;

 if (sysstatus.SystemState == STATE_RUNNING)
    {
    bRunSystem->Caption="Suspend";
    bSetConfig->Enabled=false;
    running=1;
    sysstatus.SystemState=STATE_OPERATING;
    syslog->AddSysLogEntry("Operator started operation");

    // execute the on-start script...
    if (AnsiString(preferences.Script_OnStart).Trim() != "")
       {
       syslog->AddSysLogEntry("Executing on-start script ...");
       script.Initialize(&paramlist, &statelist, syslog, sysstatus.SourceSocket);
       script.ExecuteScript(preferences.Script_OnStart);
       }
    }
 else
    {
    if (sysstatus.SystemState == STATE_SUSPENDED)
       {
       syslog->AddSysLogEntry("Operator resumed operation");

       // execute the on-resume script...
       if (AnsiString(preferences.Script_OnResume).Trim() != "")
          {
          syslog->AddSysLogEntry("Executing on-resume script ...");
          script.Initialize(&paramlist, &statelist, syslog, sysstatus.SourceSocket);
          script.ExecuteScript(preferences.Script_OnResume);
          }

       bRunSystem->Caption="Suspend";
       running=1;
       bSetConfig->Enabled=false;
       sysstatus.SystemState=STATE_OPERATING;
       }
    else
       {
       if (sysstatus.SystemState == STATE_OPERATING)
          {
          syslog->AddSysLogEntry("Operator suspended operation");

          // execute the on-suspend script...
          if (AnsiString(preferences.Script_OnSuspend).Trim() != "")
             {
             syslog->AddSysLogEntry("Executing on-suspend script ...");
             script.Initialize(&paramlist, &statelist, syslog, sysstatus.SourceSocket);
             script.ExecuteScript(preferences.Script_OnSuspend);
             }

          bRunSystem->Caption="Resume";
          running=0;
          bSetConfig->Enabled=true;
          sysstatus.SystemState=STATE_SUSPENDED;
          }
       }
    }

 starttime=TDateTime::CurrentDateTime();
 if (update)
    UpdateState("Running", running);       // suspend or re-start the system
}


void __fastcall TfMain::bRunSystemClick(TObject *Sender)
{
 StartSuspendSystem(true);
}
//---------------------------------------------------------------------------


void __fastcall TfMain::bConfigClick(TObject *Sender)
{
 // now there is the possibility to configure the system
 fConfig->Initialize(&paramlist, &preferences);
 fConfig->Show();
}
//---------------------------------------------------------------------------


void __fastcall TfMain::bSetConfigClick(TObject *Sender)
{
 // disable the set config and start button
 bSetConfig->Enabled=false;

 // if the config window is visible, update the parameters for the current tab sheet
 if (fConfig->Visible == true)
    fConfig->Close();

 if (sysstatus.SystemState == STATE_INFORMATION)
    bReset->Enabled=false;

 syslog->AddSysLogEntry("Operator set configuration");

 BroadcastParameters();

 // only publish the states in the information phase
 if (sysstatus.SystemState == STATE_INFORMATION)
    {
    // after broadcasting the states, we are now in the Initialization Phase
    sysstatus.SystemState=STATE_INITIALIZATION;
#if 0
    sysstatus.EEGsourceStatusReceived = false;
    sysstatus.SigProcStatusReceived = false;
    sysstatus.ApplicationStatusReceived = false;
#endif
    BroadcastStates();
    // send an system command 'Start' to the EEGsource
    SendSysCommand("Start", sysstatus.SourceSocket);
    sysstatus.NumMessagesSent1++;
    sysstatus.NumStatesSent1++;
    }

 // enable the set config
 bSetConfig->Enabled=true;
}
//---------------------------------------------------------------------------



void __fastcall TfMain::ApplicationEvents1Idle(TObject *Sender, bool &Done)
{
 Application->ProcessMessages();
}
//---------------------------------------------------------------------------


void __fastcall TfMain::FormCreate(TObject *Sender)
{
 firsttime=true;
 Application->OnMinimize = SetTrayIcon;
 Application->OnRestore = RemoveTrayIcon;

 preferences.GetDefaultSettings();
}
//---------------------------------------------------------------------------


void TfMain::SetFunctionButtons()
{
 if ((preferences.Button1_Name[0] != '\0') && (preferences.Button1_Cmd[0] != '\0'))
    {
    bFunction1->Enabled=true;
    bFunction1->Caption=preferences.Button1_Name;
    }
 else
    {
    bFunction1->Enabled=false;
    bFunction1->Caption="Function 1";
    }
 if ((preferences.Button2_Name[0] != '\0') && (preferences.Button2_Cmd[0] != '\0'))
    {
    bFunction2->Enabled=true;
    bFunction2->Caption=preferences.Button2_Name;
    }
 else
    {
    bFunction2->Enabled=false;
    bFunction2->Caption="Function 2";
    }
 if ((preferences.Button3_Name[0] != '\0') && (preferences.Button3_Cmd[0] != '\0'))
    {
    bFunction3->Enabled=true;
    bFunction3->Caption=preferences.Button3_Name;
    }
 else
    {
    bFunction3->Enabled=false;
    bFunction3->Caption="Function 3";
    }
 if ((preferences.Button4_Name[0] != '\0') && (preferences.Button4_Cmd[0] != '\0'))
    {
    bFunction4->Enabled=true;
    bFunction4->Caption=preferences.Button4_Name;
    }
 else
    {
    bFunction4->Enabled=false;
    bFunction4->Caption="Function 4";
    }
}


void __fastcall TfMain::FormShow(TObject *Sender)
{
 if (firsttime)
    {
    // fConnectionInfo->Show();
    // fVisConfig->Show();
    syslog=new SYSLOG;
    syslog->AddSysLogEntry("BCI2000 started");
    firsttime=false;

    SetFunctionButtons();
    }
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bShowConnectionInfoClick(TObject *Sender)
{
 fConnectionInfo->Show();
}
//---------------------------------------------------------------------------



void __fastcall TfMain::SetTrayIcon(TObject *Sender)
{
 // TrayIcon->Visible=true;
}

void __fastcall TfMain::RemoveTrayIcon(TObject *Sender)
{
 // TrayIcon->Visible=false;
}


void __fastcall TfMain::ActiveTimerTimer(TObject *Sender)
{
TDateTime       cur_time, delay;
AnsiString      caption;

 // calculate the seconds the system has been idle/running
 cur_time=TDateTime::CurrentDateTime();
 delay=cur_time-starttime;

 caption=AnsiString(TXT_WINDOW_CAPTION)+" "+AnsiString(TXT_OPERATOR_VERSION);

 if (sysstatus.SystemState == STATE_OPERATING)
    caption += " - Running "+delay.FormatString("nn:ss")+" s";
 if (sysstatus.SystemState == STATE_SUSPENDED)
    caption += " - Suspended "+delay.FormatString("nn:ss")+" s";

 fMain->Caption=caption;
}
//---------------------------------------------------------------------------



void __fastcall TfMain::Exit1Click(TObject *Sender)
{
 QuitOperator();
}
//---------------------------------------------------------------------------


void __fastcall TfMain::N1Click(TObject *Sender)
{
 fPreferences->preferences=&preferences;
 fPreferences->ShowModal();
 SetFunctionButtons();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::About1Click(TObject *Sender)
{
 fAbout->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::States1Click(TObject *Sender)
{
 fShowStates->statelist=&statelist;
 fShowStates->ShowModal();
}
//---------------------------------------------------------------------------


void __fastcall TfMain::OperatorLog1Click(TObject *Sender)
{
 syslog->ShowSysLog();        
}
//---------------------------------------------------------------------------


void __fastcall TfMain::bFunction1Click(TObject *Sender)
{
 script.Initialize(&paramlist, &statelist, syslog, sysstatus.SourceSocket);
 script.ExecuteCommand(preferences.Button1_Cmd);
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bFunction2Click(TObject *Sender)
{
 script.Initialize(&paramlist, &statelist, syslog, sysstatus.SourceSocket);
 script.ExecuteCommand(preferences.Button2_Cmd);
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bFunction3Click(TObject *Sender)
{
 script.Initialize(&paramlist, &statelist, syslog, sysstatus.SourceSocket);
 script.ExecuteCommand(preferences.Button3_Cmd);
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bFunction4Click(TObject *Sender)
{
 script.Initialize(&paramlist, &statelist, syslog, sysstatus.SourceSocket);
 script.ExecuteCommand(preferences.Button4_Cmd);
}
//---------------------------------------------------------------------------





