/******************************************************************************
 * Program:   EEGsource.EXE                                                   *
 * Module:    UMAIN.CPP                                                       *
 * Comment:   The EEGsource module for BCI2000                                *
 * Version:   0.24                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 02/15/2000 - First start                                           *
 * V0.02 - 02/16/2000 - Sending data to the operator                          *
 * V0.04 - 02/23/2000 - ???                                                   *
 * V0.05 - 03/02/2000 - Added the supplemental byte after the content descr.  *
 * V0.07 - 03/04/2000 - Created test signal processing and application module *
 * V0.08 - 03/08/2000 - Starting to transmit data to signal processing        *
 * V0.09 - 03/16/2000 - Complete communication in between all core modules    *
 * V0.10 - 03/20/2000 - Updated to C++Builder 5.0                             *
 * V0.11 - 03/24/2000 - made the socket connections blocking inst. of non-bl. *
 * V0.12 - 03/29/2000 - stable and documented version                         *
 * V0.13 - 04/19/2000 - test version                                          *
 * V0.14 - 05/15/2000 - implemented GenericADC, GenericDataStorage            *
 *                      and RandomNumberADC                                   *
 * V0.15 - 06/13/2000 - implemented GenericIntSignal + GenericVisualization   *
 * V0.16 - 07/25/2000 - updated RandomNumberADC to a waveform generator       *
 * V0.17 - 08/08/2000 - included visualization that now supports GenericSigs  *
 * V0.18 - 09/21/2000 - command line options added                            *
 * V0.19 - 03/23/2001 - support for reconfiguration (Running 0->1)            *
 * V0.19b- 04/11/2001 - CORECOMM (blocking sockets) for core communication    *
 * V0.20 - 04/13/2001 - using coremessages to receive from the application    *
 *                      and to send out info to signal processing             *
 *                      updated to Storage.cpp that increments run            *
 * V0.21 - 05/11/2001 - determine whether to record or not from Running       *
 * V0.22 - 06/07/2001 - upgraded to shared libraries V0.17                    *
 * V0.23 - 08/14/2001 - added possibility to decimate EEG display             *
 *                      added visualization of round-trip time                *
 * V0.24 - 09/27/2001 - can control RandomNumberGenerator with mouse          *
 * V0.241- 01/11/2002 - fixed minor issue in MainDataAcqLoop() and UStorage   *
 ******************************************************************************/

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <stdlib.h>

#define ADC_DTADC

#include "UCoreComm.h"
#include "..\shared\defines.h"
#include "UBCItime.h"
#include "UGenericVisualization.h"
#include "UCoreMessage.h"
#include "UParameter.h"
#include "GenericADC.h"
#ifdef ADC_DTADC
 #include "DTADC\DTADC.h"
#else
 #include "RandomNumber\RandomNumberADC.h"
#endif
#include "UStorage.h"

#include "UMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

// FILE *termin;

TfMain          *fMain;
STATEVECTOR     *statevector;
TCriticalSection *critsec_statevector=NULL;

// **************************************************************************
// Function:   TReceivingThread
// Purpose:    class definition for TReceivingThread
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
//---------------------------------------------------------------------------
class TReceivingThread : public  TServerClientThread
{
private:
public:
    __fastcall TReceivingThread(bool CreateSuspended, TServerClientWinSocket* ASocket);
    void __fastcall ClientExecute(void);
};


// **************************************************************************
// Function:   TReceivingThread
// Purpose:    construction definition for TReceivingThread
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
//---------------------------------------------------------------------------
__fastcall TReceivingThread::TReceivingThread(bool CreateSuspended, TServerClientWinSocket* ASocket) : TServerClientThread(CreateSuspended, ASocket)
{
}
//---------------------------------------------------------------------------

TReceivingThread  *receiving_thread;

// **************************************************************************
// Function:   ClientExecute
// Purpose:    main loop for TReceivingThread that handles
//             the incoming data from the Application core module
//             it is executed when Application connects
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TReceivingThread::ClientExecute(void)
{
TWinSocketStream        *pStream;
COREMESSAGE             *coremessage;
PARAM   *paramptr;
int     statevectorlength;
int     trycount, ret;

  pStream=NULL;

  trycount=0;
  // if the statevector has not been defined yet, wait a little
  while ((!statevector) && (trycount < 10))
   {
   Sleep(300);
   trycount++;
   }

  // determine the size of the transmitted content by looking at the system parameters
  paramptr=fMain->paramlist.GetParamPtr("StateVectorLength");
  if (!paramptr)
     Terminate();
  else
     {
     statevectorlength=atoi(paramptr->GetValue());
     if (statevector->GetStateVectorLength() != statevectorlength) Terminate();
     }

  pStream=new TWinSocketStream(ClientSocket, 5000);

  while (!Terminated && ClientSocket->Connected)
   {
   try
    {
    if (pStream->WaitForData(1000))
       {
       // read the statevector
       coremessage=new COREMESSAGE();
       ret=coremessage->ReceiveCoreMessage(pStream);
       if (ret == ERRCORE_NOERR)
          {
          coremessage->ParseMessage();
          if (coremessage->GetDescriptor() == COREMSG_SYSCMD)
             {
             if (stricmp(coremessage->syscmd.GetSysCmd(), "Reset") == 0)
                Terminate();
             }
          if (coremessage->GetDescriptor() == COREMSG_STATEVECTOR)
             {
             critsec_statevector->Acquire();
             memcpy(statevector->GetStateVectorPtr(), coremessage->GetBufPtr(), statevector->GetStateVectorLength());
             fMain->statevectorupdate->SetEvent();
             critsec_statevector->Release();
             }
          }
       delete coremessage;
       }
    }
   catch(...)
    {
    Terminate();
    }
   }

 if (pStream) delete pStream;

 // send "Reset" to the operator
 fMain->corecomm->SendSysCommand("Reset");
 receiving_thread=NULL;
 ReturnValue=1;
}
//---------------------------------------------------------------------------


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


void __fastcall TfMain::StartDaqMessage(TMessage &Message)
{
 MainDataAcqLoop();
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


void TfMain::UpdateStateVector()
{
int     i;
STATE   *cur_state;

 for (i=0; i<statelist.GetNumStates(); i++)
  {
  cur_state=statelist.GetStatePtr(i);
  if (cur_state)
     if (cur_state->modified)
        {
        statevector->SetStateValue(cur_state->GetName(), cur_state->GetValue());
        cur_state->modified=false;
        }
  }
}


// **************************************************************************
// Function:   Write2SignalProc
// Purpose:    This sends the passed signal on to SignalProcessing
// Parameters: signal - pointer to the GenericIntSignal to be sent
//             statevector - pointer to the StateVector to be sent
//             socket - pointer to the non-blocking socket connection
//             listprm - pointer to the list-parameter that defines, which channels are sent
//                       if NULL, all channels are sent
// Returns:    0 - error
//             1 - OK
// **************************************************************************
int TfMain::Write2SignalProc(GenericIntSignal *signal, STATEVECTOR *statevector, PARAM *listprm)
{
int  channel, channelnum;
bool ret1, ret2;

 // send the state vector on to SignalProcessing
 ret1=sendingcomm->SendStateVector2CoreModule(statevector);
 // send data on to Signal Processing
 ret2=sendingcomm->SendData2CoreModule(signal, listprm);

 if (!ret1 || !ret2)
    return(0);

 return(1);
}


// **************************************************************************
// Function:   ParametersConsistent
// Purpose:    This function checks certain parameters for consistency
// Parameters: N/A
// Returns:    1 ... parameters are consistent
//             0 ... parameters are inconsistent
// **************************************************************************
int TfMain::ParametersConsistent()
{
PARAM   *transmitchlistprm, *transmitchprm;
int     consistent;

 consistent=1;
 try
  {
  // number of TransmitCh != number of values in the list of channels to transmit
  if (fMain->paramlist.GetParamPtr("TransmitChList")->GetNumValues() != atoi(fMain->paramlist.GetParamPtr("TransmitCh")->GetValue()))
     consistent=0;
  // we do not want to transmit anything to signal processing, if either the transmitchlist is empty, or TransmitCh==0
  // but in these case, the parameters should be called consistent
  /* if (((fMain->paramlist.GetParamPtr("TransmitChList")->GetValue(0))[0] == '\0') || (atoi(fMain->paramlist.GetParamPtr("TransmitCh")->GetValue()) == 0))
     {
     fMain->paramlist.GetParamPtr("TransmitChList")->SetValue("");
     fMain->paramlist.GetParamPtr("TransmitCh")->SetValue("0");
     consistent=1;
     } */
  }
 catch(...)
  {
  consistent=0;
  }

 return(consistent);
}


// **************************************************************************
// Function:   MainDataAcqLoop
// Purpose:    This is the main data acquisition loop
//             it reads data from the ADC board
//             this loop is being called on transitions from the state Running
//             from 0 to 1
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TfMain::SetEEGDisplayProperties()
{
char    cur_buf[256];
int     samplefreq, i, displaysamples;
int     display_min= 0;
int     display_max= 8192;
int     visdecim;

 // unique source ID for this visualization
 vis->SetSourceID(SOURCEID_EEGDISP);
 vis->SendCfg2Operator(SOURCEID_EEGDISP, CFGID_WINDOWTITLE, "Source EEG");
 roundtripvis->SetSourceID(SOURCEID_ROUNDTRIP);
 roundtripvis->SendCfg2Operator(SOURCEID_ROUNDTRIP, CFGID_WINDOWTITLE, "Roundtrip");

 try {
  samplefreq=atoi(paramlist.GetParamPtr("SamplingRate")->GetValue());
  display_min= atoi(paramlist.GetParamPtr("SourceMin")->GetValue());
  display_max= atoi(paramlist.GetParamPtr("SourceMax")->GetValue());
  visdecim=atoi(paramlist.GetParamPtr("VisualizeSourceDecimation")->GetValue());
  } catch(...)
   {
   samplefreq=128;
   visdecim=1;
   }

 // EEG display shall be two seconds long
 displaysamples=2*samplefreq/visdecim;

 // properties for EEG visualization
 sprintf(cur_buf, "%03d", displaysamples);
 vis->SendCfg2Operator(SOURCEID_EEGDISP, CFGID_NUMSAMPLES, cur_buf);
 sprintf(cur_buf, "%d", display_min);
 vis->SendCfg2Operator(SOURCEID_EEGDISP, CFGID_MINVALUE, cur_buf);
 sprintf(cur_buf, "%d", display_max);
 vis->SendCfg2Operator(SOURCEID_EEGDISP, CFGID_MAXVALUE, cur_buf);

 /* for (i=0; i<displaysamples; i++)
  {
  sprintf(cur_buf, "%03d %d", i, (int)(1000*(float)i/(float)samplefreq));
  vis->SendCfg2Operator(SOURCEID_EEGDISP, CFGID_XAXISLABEL, cur_buf);
  } */

 // properties for roundtrip visualization
 roundtripvis->SendCfg2Operator(SOURCEID_ROUNDTRIP, CFGID_NUMSAMPLES, "128");
 roundtripvis->SendCfg2Operator(SOURCEID_ROUNDTRIP, CFGID_MINVALUE, "0");
 roundtripvis->SendCfg2Operator(SOURCEID_ROUNDTRIP, CFGID_MAXVALUE, "50");
}


// **************************************************************************
// Function:   MainDataAcqLoop
// Purpose:    This is the main data acquisition loop
//             it reads data from the ADC board
//             this loop is being called on transitions from the state Running
//             from 0 to 1
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TfMain::MainDataAcqLoop()
{
static unsigned short   oldrunning=0;
unsigned short          sourcetime, stimulustime;
unsigned short          running;
BCITIME                 bcitime;
PARAM                   *visparam;
bool                    visualize;
int                     i, ret, visdecim;
GenericSignal           *roundtripsignal;
int                     x, y;

 // initialize the DataStorage object
 tds->Initialize(&paramlist, &statelist, statevector);
 tds->Resume();
 // the constructor for adc has been called before
 // (when the connection to the operator was established)
 adc->ADInit();				// initialize

 // create the signal for the roundtrip visualization
 roundtripsignal=new GenericSignal(1, 1);

 visualize=false;
 visparam=fMain->paramlist.GetParamPtr("VisualizeSource");
 if (visparam)
    if (atoi(visparam->GetValue()) == 1)
       {
       SetEEGDisplayProperties();             // set the display properties at the operator
       visualize=true;
       }
 visdecim=atoi(fMain->paramlist.GetParamPtr("VisualizeSourceDecimation")->GetValue());

 // don't proceed, if certain parameters are not consistent
 if (ParametersConsistent() == 0)
    {
    corecomm->SendStatus("300 Parameters are inconsistent ...");
    adc->ADShutdown();
    return;
    }

 while ((resetrequest == 0) && (corecomm->Connected()))
  {
  adc->ADReadDataBlock();		// read data from ADC - it won't return, until data is there

  // we have to acquire a lock to the statevector
  // (otherwise, if we are unlucky, the thread that receives the resulting statevector from the
  // application will overwrite the statevector at the same time)
  critsec_statevector->Acquire();
  statevectorupdate->WaitFor(750);
  UpdateStateVector();

  // time stamp the EEG data
  sourcetime=statevector->GetStateValue("SourceTime");
  stimulustime=statevector->GetStateValue("StimulusTime");
  statevector->SetStateValue("SourceTime", bcitime.GetBCItime_ms());

  // get the current value of the state "Running" in the state vector
  // flipping to 0 quits this main data acquisition loop
  running=statevector->GetStateValue("Running");
  // has the operator restarted the system ?
  if ((running == 1) && (oldrunning == 0))
     {
     adc->ADShutdown();                                         // stop data acquisition
     tds->Initialize(&paramlist, &statelist, statevector);      // re-initialize the data storage
     adc->ADInit();
     visualize=false;
     visparam=fMain->paramlist.GetParamPtr("VisualizeSource");  // re-do visualization properties
     if (visparam)
        if (atoi(visparam->GetValue()) == 1)
           {
           SetEEGDisplayProperties();             // set the display properties at the operator
           visualize=true;
           }
     visdecim=atoi(fMain->paramlist.GetParamPtr("VisualizeSourceDecimation")->GetValue());
     adc->ADReadDataBlock();		                        // have to read new data after reconfiguration, too
     }

  // inform the operator, in case the system has been suspended
  if ((running == 0) && (oldrunning == 1))
     corecomm->SendSysCommand("Suspend");

  // depending on what Running is, set Recording accordingly
  if (running == 0) statevector->SetStateValue("Recording", 0);
  if (running == 1) statevector->SetStateValue("Recording", 1);
  oldrunning=running;

  Write2SignalProc(adc->signal, statevector, fMain->paramlist.GetParamPtr("TransmitChList"));

  if ((tds) && (statevector->GetStateValue("Recording") == 1))
     tds->Write2Disk(adc->signal);

  critsec_statevector->Release();

  // store timing info
  if (running == 1)
     {
     roundtripsignal->SetValue(0, 0, (float)bcitime.TimeDiff(sourcetime, stimulustime));
     roundtripvis->Send2Operator(roundtripsignal);
     }

  // send the whole signal to the operator
  if (visualize)
     if (vis)
        vis->Send2Operator(adc->signal, visdecim);

  // give the main thread time to process all the messages,
  // e.g., in order to receive the messages from the operator
  // (the OperatorSocket is non-blocking, and would otherwise not receive data)
  for (i=0; i<25; i++)
   Application->ProcessMessages();

  if (!adc) break;
  }

 // we also have to have a adc->Shutdown() method that stops
 // the data acquisition at this point
 if (adc) adc->ADShutdown();

 if (roundtripsignal) delete roundtripsignal;
 roundtripsignal=NULL;

 // send out "Reset" to Signal Processing and then close the connection
 if (sendingcomm)
    {
    sendingcomm->SendSysCommand("Reset");
    sendingcomm->Terminate();
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
// Purpose:    disables the timer and terminates the receiving thread
//             (which, in turn, shuts down all connections)
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TfMain::ShutdownSystem()
{


   //     termin= fopen("Termin.asc","a");
   //     fprintf(termin,"Begin Shutdown \n");
   //     fclose( termin );

 // delete the adc object
 if (adc) adc->ADShutdown();
 if (adc) delete adc;
 adc=NULL;
 // delete the vis object
 if (vis) delete vis;
 vis=NULL;
 if (roundtripvis) delete roundtripvis;
 roundtripvis=NULL;
 // delete the tds object
 if (tds) delete tds;
 tds=NULL;

 // now, close all the other connections
 ShutdownConnections();

 // delete the communication objects
 // nooooooo ... should free on terminate
 // if (corecomm)    delete corecomm;
 // if (sendingcomm) delete sendingcomm;
 corecomm=sendingcomm=NULL;

 paramlist.ClearParamList();
 statelist.ClearStateList();
 if (statevector) delete statevector;
 statevector=NULL;

    //    termin= fopen("Termin.asc","a");
    //    fprintf(termin,"End Shutdown \n");
    //    fclose( termin );
}


// **************************************************************************
// Function:   TfMain
// Purpose:    the constructor for the TfMain class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner) : TForm(Owner)
{
 statevector=NULL;
 critsec_statevector=new TCriticalSection();

 // this is used to synchronize the system,
 // i.e., the main data acquisiton loop waits until the system has received the
 // updated state vector
 statevectorupdate=new TEvent(NULL, false, false, "StateVectorUpdate");

 adc=NULL;
 vis=NULL;
 roundtripvis=NULL;
 tds=NULL;
 corecomm=NULL;
 sendingcomm=NULL;
}


// **************************************************************************
// Function:   bConnectClick
// Purpose:    when the Connect button is clicked on, start up the
//             data acquistion module
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
//---------------------------------------------------------------------------
void __fastcall TfMain::bConnectClick(TObject *Sender)
{
 StartupDataAcquisition(eOperatorIP->Text);
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   StartupDataAcquisition
// Purpose:    The listening socket
//             is opened and the connection to the operator is established
//             Thereafter, we create an instance of the prefered ADC board
//             we then publish all our parameters and all our states
// Parameters: N/A
// Returns:    0 ... on error (e.g., Operator could not be found)
//             1 ... on success
// **************************************************************************
int TfMain::StartupDataAcquisition(AnsiString connectto)
{
char    paramstring[256];
int     ret;

 // clear all parameters and states first
 paramlist.ClearParamList();
 statelist.ClearStateList();

 // create and initialize CORECOMM object
 // thereby creating connection to the operator
 corecomm=new CORECOMM(true);
 ret=corecomm->Initialize(connectto, 4000, fMain, COREMODULE_OPERATOR);
 if (ret == 0)
    {
    Application->MessageBox("Could not make a connection to the Operator", "Error", MB_OK);
    if (corecomm) corecomm->Terminate();
    return(0);
    }
 else
    corecomm->Resume();                 // start receiving on the operator connection

 // enable the receiving socket
 ReceivingSocket->Open();
 eReceivingPort->Text=AnsiString(ReceivingSocket->Socket->LocalPort);
 eReceivingIP->Text=ReceivingSocket->Socket->LocalHost;

 // create an instance of our ADC board class
 // in this case, we create an instance of the RandomNumberADC class
 // the constructor in this object will fill up the parameter and state lists
 #ifdef ADC_DTADC
  adc=new DTADC(&paramlist, &statelist);
 #else
  adc=new RandomNumberADC(&paramlist, &statelist);
 #endif
 // create an instance of GenericVisualization
 // it will handle the visualization to the operator
 vis=new GenericVisualization(&paramlist, corecomm);
 // visualization of round trip
 roundtripvis=new GenericVisualization(&paramlist, corecomm);
 // construct a new TDataStorage object
 tds=new TDataStorage(&paramlist);

 // add some generic parameters
 sprintf(paramstring, "Visualize int VisualizeSource= 1 1 0 1 // visualize raw EEG (0=no, 1=yes)\n");
 paramlist.AddParameter2List(paramstring, strlen(paramstring));
 sprintf(paramstring, "Visualize int VisualizeSourceDecimation= 1 1 0 1 // decimation factor for raw EEG\n");
 paramlist.AddParameter2List(paramstring, strlen(paramstring));
 sprintf(paramstring, "Visualize int SourceMin= 0 0 -8092 0 // raw EEG vis Min Value\n");
 paramlist.AddParameter2List(paramstring, strlen(paramstring));
 sprintf(paramstring, "Visualize int SourceMax= 8092 8092 0 16386 // raw EEG vsi Max Value\n");
 paramlist.AddParameter2List(paramstring, strlen(paramstring));

 sprintf(paramstring, "Source int TransmitCh=      4 4 1 128        // the number of transmitted channels\n");
 paramlist.AddParameter2List(paramstring, strlen(paramstring));
 sprintf(paramstring, "Source intlist TransmitChList= 4 1 2 3 4 1 1 128        // list of transmitted channels (# of channels MUST equal TransmitCh)\n");
 paramlist.AddParameter2List(paramstring, strlen(paramstring));

 // add parameters for socket connection
 // my receiving socket port number
 sprintf(paramstring, "System string EEGsourcePort= %d 4200 1024 32768 // this module's listening port\n", ReceivingSocket->Socket->LocalPort);
 paramlist.AddParameter2List(paramstring, strlen(paramstring));
 // and IP address
 sprintf(paramstring, "System string EEGsourceIP= %s 127.0.0.1 127.0.0.1 127.0.0.1 // this module's listening IP\n", corecomm->GetSocket()->LocalAddress.c_str());
 paramlist.AddParameter2List(paramstring, strlen(paramstring));

 // finally, publish parameters
 corecomm->PublishParameters(&paramlist);

 // send out the states
 statelist.AddState2List("Running 1 0 0 0");    // published w/default value of 1 (system is suspended)
 statelist.AddState2List("Recording 1 0 0 0");  // published w/default value of 0 (NO recording)
 corecomm->PublishStates(&statelist);

 // send a confirmation message
 corecomm->SendStatus("100 EEGsource waiting for configuration ...");
 return(1);
}


// **************************************************************************
// Function:   bDisconnectClick
// Purpose:    when the Disconnect button is clicked on, the data generating
//             timer is turned off and the receiving thread is terminated
//             (which, in turn, closes all other connections)
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::bDisconnectClick(TObject *Sender)
{
 if (corecomm) corecomm->Terminate();

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
char    destIP[255];
int     destport, res;

 // if both parameters are defined (they should !!), connect to SignalProcessing
 if ((paramlist.GetParamPtr(PARAMNAME_SIGPROCIP)) && (paramlist.GetParamPtr(PARAMNAME_SIGPROCPORT)))
    {
    strcpy(destIP, paramlist.GetParamPtr(PARAMNAME_SIGPROCIP)->GetValue());
    destport=atoi(paramlist.GetParamPtr(PARAMNAME_SIGPROCPORT)->GetValue());
    if (sendingcomm) delete sendingcomm;
    sendingcomm=new CORECOMM(true);
    if (!sendingcomm) return(ERR_NOSOCKCONN);
    res=sendingcomm->Initialize(AnsiString(destIP), destport, (TForm *)fMain, COREMODULE_SIGPROC);
    if (res == 0) return(ERR_NOSOCKCONN);
    rSendingConnected->Checked=true;
    eSendingIP->Text=destIP;
    eSendingPort->Text=AnsiString(destport);
    corecomm->SendStatus("200 EEGsource module initialized ");
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
unsigned short cur_running;
int     i;
STATE   *cur_state;

 // we received a parameter
 if (coremessage->GetDescriptor() == COREMSG_PARAMETER)
    {
    // now, add the parameter to the list of parameters
    if (coremessage->param.valid)
       {
       // if the statevector does exist, only add/modify a parameter, if state "Running" is 0
       if (statevector)
          {
          if (statevector->GetStateValue("Running") == 0)
             paramlist.CloneParameter2List(&(coremessage->param));
          }
       else
          paramlist.CloneParameter2List(&(coremessage->param));
       }
    }

 // we received a state
 if (coremessage->GetDescriptor() == COREMSG_STATE)
    {
    if (coremessage->state.valid)
       {
       // now, add the state to the list of states
       statelist.AddState2List(&(coremessage->state));
       statelist.GetStatePtr(coremessage->state.GetName())->modified=true;
       }
    }

 // the message I got was a state vector
 if (coremessage->GetDescriptor() == COREMSG_STATEVECTOR)
    {
    }

 // the message I got was a system command
 if (coremessage->GetDescriptor() == COREMSG_SYSCMD)
    {
    // is the system command 'Reset' ?
    if (strcmp(coremessage->syscmd.GetSysCmd(), "Reset") == 0)
       resetrequest=1;            // makes the data acquisition stop

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
          Application->MessageBox("Could not initialize connections", "Error", MB_OK);
          }
       }
    // is this to start the system ?
    if (strcmp(coremessage->syscmd.GetSysCmd(), "Start") == 0)
       {
       if (statevector)   // does the state vector already exist ?
          PostMessage(fMain->Handle, STARTDAQ_MESSAGE, 0, 0);
       }
    }
}


// **************************************************************************
// Function:   ReceivingSocketClientDisconnect
// Purpose:    if the receiving socket is being disconnected, turn off
//             the data generating timer and shut down the receiving thread
//             (which will then shut down all the other connections as well)
// Parameters: Socket - the socket handle for the receiving connection to the application
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::ReceivingSocketClientDisconnect(TObject *Sender, TCustomWinSocket *Socket)
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
 if (critsec_statevector) delete critsec_statevector;
 if (statevectorupdate) delete statevectorupdate;
 statevectorupdate=NULL;
 statevector=NULL;
 critsec_statevector=NULL;
 // if (corecomm)    delete corecomm;
 // if (sendingcomm) delete sendingcomm;

    //    termin= fopen("Termin.asc","a");
    //    fprintf(termin,"Closing Form \n");
    //    fclose( termin );
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   ReceivingSocketGetThread
// Purpose:    if Application connects to the EEGsource, a new thread
//             is being spawned that handles the incoming data over
//             a blocking socket connection
// Parameters: ClientSocket - the socket handle for the blocking connection
// Returns:    N/A
// **************************************************************************
void __fastcall TfMain::ReceivingSocketGetThread(TObject *Sender,
      TServerClientWinSocket *ClientSocket,
      TServerClientThread *&SocketThread)
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
     if (StartupDataAcquisition(connectto) == 0)
        Application->Terminate();
     }
  }
}
//---------------------------------------------------------------------------



