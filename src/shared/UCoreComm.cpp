//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "UParameter.h"
#include "UState.h"
#include "UGenericSignal.h"
#include "UCoreComm.h"
#include "UCoreMessage.h"
#include "UBCItime.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall Unit1::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall CORECOMM::CORECOMM(bool CreateSuspended) : TThread(CreateSuspended)
{
 CoreSocket=NULL;
}
//---------------------------------------------------------------------------

__fastcall CORECOMM::~CORECOMM()
{
 try
  {
  if (CoreSocket)
     {
     CoreSocket->Socket->Close();
     CoreSocket->Close();
     delete CoreSocket;
     }
  } catch(...) {CoreSocket=NULL;}

 CoreSocket=NULL;
 // this->TThread::~TThread();
}
//---------------------------------------------------------------------------


TClientWinSocket *CORECOMM::GetSocket() const
{
 return(CoreSocket->Socket);
}


bool CORECOMM::Connected() const
{
 if (!CoreSocket) return(false);
 if (!CoreSocket->Socket->Connected) return(false);

 return(true);
}


int CORECOMM::Initialize(AnsiString destIP, int destPort, TForm *new_tfmain, int new_moduleID)
{
int ret;

 main_form=new_tfmain;
 moduleID=new_moduleID;

 ret=1;
 if (CoreSocket) delete CoreSocket;
 CoreSocket = new TClientSocket(Application);

 // first, try to use destIP as an IP address
 // if this does not work, try to use it as a host name
 CoreSocket->Address=destIP;
 CoreSocket->Port=destPort;
 CoreSocket->ClientType=ctBlocking;
 try{
  CoreSocket->Open();
  } catch(...)
    {
    CoreSocket->Host=destIP;
    try{
     CoreSocket->Open();
     } catch(...)                               // now if using it as a host address (i.e., www.cnn.com), then give up
      {
      if (CoreSocket) delete CoreSocket;
      CoreSocket=NULL;
      ret=0;
      }
    }

 sendingparameters=false;
 FreeOnTerminate=true;                  // this automatically destroys the object on termination
 return(ret);
}


void __fastcall CORECOMM::Execute()
{
TWinSocketStream        *pStream;
COREMESSAGE             *coremessage;

 if (!CoreSocket) return;
 if (!CoreSocket->Socket->Connected) return;

 pStream=new TWinSocketStream(CoreSocket->Socket, 5000);
 while (!Terminated && CoreSocket->Socket->Connected)
  {
  try
   {
    if (pStream->WaitForData(100))
       {
       coremessage=new COREMESSAGE;
       if (coremessage->ReceiveCoreMessage(pStream) != ERRCORE_NOERR)
          Terminate();
       else
          {
          coremessage->ParseMessage();                  // parse whatever the operator sent me
          // send a message to the main thread that will
          // cause it to process the message
          // this might be a little rough ... passing a pointer in an int in a message
          // do this only, if we want to notify a window
          if (main_form)
             SendMessage(main_form->Handle, HANDLE_MESSAGE, (int)coremessage, moduleID);
          }
       if (coremessage) delete coremessage;
       }
    }
   catch(...)
    {
    Terminate();
    }
   }

 if (main_form)
    SendMessage(main_form->Handle, RESET_MESSAGE, 0, moduleID);

 if (pStream) delete pStream;
 ReturnValue=1;
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   StartSendingParameters
// Purpose:    this method has to be called before using SendParameter
//                StartSendingParameters()
//                SendParameter()
//                StopSendingParameters()
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void CORECOMM::StartSendingParameters()
{
 if (sendingparameters) return;
 SendSysCommand("StartOfParameter");
 sendingparameters=true;
}


// **************************************************************************
// Function:   StopSendingParameters
// Purpose:    this method has to be called after using SendParameter
//                StartSendingParameters()
//                SendParameter()
//                StopSendingParameters()
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void CORECOMM::StopSendingParameters()
{
 if (!sendingparameters) return;
 SendSysCommand("EndOfParameter");
 sendingparameters=false;
}


// **************************************************************************
// Function:   PublishParameter
// Purpose:    this method publishes one parameter
// Parameters: param - a pointer to the parameter to be published
// Returns:    1 ... if successful
//             0 ... on error
// **************************************************************************
int CORECOMM::PublishParameter(const PARAM *param) const
{
TWinSocketStream        *pStream;
COREMESSAGE     *coremessage;
char            line[512];
int             i;

 if (!CoreSocket) return(0);
 if (!CoreSocket->Socket->Connected) return(0);
 if (!param)      return(0);

 // we didn't call StartSendingParameters() before
 if (!sendingparameters) return(0);

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_PARAMETER);

 pStream=new TWinSocketStream(CoreSocket->Socket, 5000);
 if (!pStream) return(0);

 strcpy(line, param->GetParamLine());                          // copy its ASCII representation to variable line
 strncpy(coremessage->GetBufPtr(), line, strlen(line));        // copy line into the coremessage
 coremessage->SetLength((unsigned short)strlen(line));         // set the length of the coremessage
 coremessage->SendCoreMessage(pStream);                        // and send it out

 delete coremessage;
 delete pStream;

 return(1);
}


// **************************************************************************
// Function:   PublishParameters
// Purpose:    this method publishes this module's parameters
// Parameters: paramlist - a pointer to the list of parameters
// Returns:    1 ... if successful
//             0 ... on error
// **************************************************************************
int CORECOMM::PublishParameters(const PARAMLIST *paramlist)
{
TWinSocketStream        *pStream;
COREMESSAGE     *coremessage;
char            line[512];
int             i;
PARAM           *cur_param;

 if (!CoreSocket) return(0);
 if (!CoreSocket->Socket->Connected) return(0);
 if (!paramlist) return(0);

 StartSendingParameters();

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_PARAMETER);

 pStream=new TWinSocketStream(CoreSocket->Socket, 5000);
 if (!pStream) return(0);

 for (i=0; i<paramlist->GetNumParameters(); i++)
  {
  cur_param=paramlist->GetParamPtr(i);                          // get the i'th parameter
  strcpy(line, cur_param->GetParamLine());                      // copy its ASCII representation to variable line
  strncpy(coremessage->GetBufPtr(), line, strlen(line));        // copy line into the coremessage
  coremessage->SetLength((unsigned short)strlen(line));         // set the length of the coremessage
  coremessage->SendCoreMessage(pStream);                        // and send it out
  }

 delete coremessage;
 delete pStream;

 StopSendingParameters();
 return(1);
}


// **************************************************************************
// Function:   PublishStates
// Purpose:    this method publishes this module's states
// Parameters: statelist - filled list of states
// Returns:    1 ... if successful
//             0 ... on error
// **************************************************************************
int CORECOMM::PublishStates(const STATELIST *statelist) const
{
TWinSocketStream        *pStream;
COREMESSAGE     *coremessage;
char            line[512];
int             i;
STATE           *cur_state;

 if (!CoreSocket) return(0);
 if (!CoreSocket->Socket->Connected) return(0);
 if (!statelist) return(0);

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_STATE);

 pStream=new TWinSocketStream(CoreSocket->Socket, 5000);
 if (!pStream) return(0);

 // now, let's publish the list of states
 // that the ADC board has requested so far ...
 for (i=0; i<statelist->GetNumStates(); i++)
  {
  cur_state=statelist->GetStatePtr(i);                          // get the i'th state
  strcpy(line, cur_state->GetStateLine());                      // copy its ASCII representation to variable line
  strncpy(coremessage->GetBufPtr(), line, strlen(line));        // copy line into the coremessage
  coremessage->SetLength((unsigned short)strlen(line));         // set the length of the coremessage
  coremessage->SendCoreMessage(pStream);                        // and send it out
  }

 delete pStream;
 delete coremessage;

 // send system command marking the end of the states
 SendSysCommand("EndOfState");

 return(1);
}


// **************************************************************************
// Function:   SendStatus
// Purpose:    sends a status coremessage to the specified socket
// Parameters: line - pointer to a status string
//             Socket - Socket descriptor for connection to the operator
// Returns:    1 ... no error
//             0 ... error
// **************************************************************************
int CORECOMM::SendStatus(const char *line) const
{
TWinSocketStream        *pStream;
COREMESSAGE     *coremessage;

 if (!CoreSocket) return(0);
 if (!CoreSocket->Socket->Connected) return(0);
 if (!line) return(0);

 pStream=new TWinSocketStream(CoreSocket->Socket, 5000);
 if (!pStream) return(0);

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_STATUS);
 coremessage->SetLength((unsigned short)strlen(line));
 strncpy(coremessage->GetBufPtr(), line, strlen(line));
 coremessage->SendCoreMessage(pStream);
 delete coremessage;

 delete pStream;
 return(1);
}


// **************************************************************************
// Function:   SendData2CoreModule
// Purpose:    sends a GenericIntSignal on to another Core Module
// Parameters: my_signal ... signal to send out
//             channellistparam ... pointer to the parameter that defines the channels to be sent, or
//                                  NULL, if all are to be sent
// Returns:    true  ... no error
//             false ... error
// **************************************************************************
bool CORECOMM::SendData2CoreModule(const GenericIntSignal *my_signal, const PARAM *channellistparam) const
{
TWinSocketStream        *pStream;
COREMESSAGE     *coremessage;
unsigned short  *short_dataptr;
short   *valueptr;
BYTE    *dataptr;
int     s, t, channelnum;

 // error and consistency checking
 if (my_signal->Channels > 255)      return(false);       // Channels > 255
 if (my_signal->MaxElements > 65535) return(false);       // samples per channel > 65536
 if (!Connected())                   return(false);       // no connection to the core module
 if ((long)my_signal->Channels*(long)my_signal->MaxElements+9 > COREMESSAGE_MAXBUFFER) return(false);     // data too big for a coremessage

 // if the list does not contain a value (i.e., the first value does not contain a number
 // if (channellistparam)
 //    if ((channellistparam->GetValue(0))[0] == '\0')
 //       return(true);

 pStream=new TWinSocketStream(GetSocket(), 5000);

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_DATA);
 coremessage->SetSuppDescriptor(VISTYPE_GRAPH);
 if (!channellistparam)
    coremessage->SetLength(sizeof(unsigned short)*(unsigned short)my_signal->Channels*(unsigned short)my_signal->MaxElements+5);                      // set the length of the coremessage
 else
    coremessage->SetLength(sizeof(unsigned short)*(unsigned short)channellistparam->GetNumValues()*(unsigned short)my_signal->MaxElements+5);         // set the length of the coremessage

 dataptr=(BYTE *)coremessage->GetBufPtr();
 // construct the header of the core message
 dataptr[0]=0;                          // sourceID is 0 for data transfer
 dataptr[1]=DATATYPE_INTEGER;           // write the datatype into the coremessage
 dataptr[2]=(BYTE)my_signal->Channels;  // write the # of channels into the coremessage
 short_dataptr=(unsigned short *)&dataptr[3];
 *short_dataptr=(unsigned short)my_signal->MaxElements; // write the # of samples into the coremessage
 // write the actual data into the coremessage
 // if no channellistparameter is defined, send everything
 if (!channellistparam)
    {
    for (t=0; t<my_signal->Channels; t++)
     for (s=0; s<my_signal->MaxElements; s++)
      {
      valueptr=(short *)&dataptr[5];
      valueptr[t*my_signal->MaxElements+s]=my_signal->GetValue(t, s);
      }
    }
 else           // if we defined a channellist, only send the channels we are interested in
    {
    for (t=0; t<channellistparam->GetNumValues(); t++)
     {
     channelnum=atoi(channellistparam->GetValue(t))-1;
     for (s=0; s<my_signal->MaxElements; s++)
      {
      valueptr=(short *)&dataptr[5];
      valueptr[t*my_signal->MaxElements+s]=my_signal->GetValue(channelnum, s);
      }
     }
    }

 coremessage->SendCoreMessage(pStream);     // and send it out
 delete coremessage;
 delete pStream;

 return(true);
}


// **************************************************************************
// Function:   SendStateVector2CoreModule
// Purpose:    sends the statevector on to another Core Module
// Parameters: statevector ... pointer to the statevector
// Returns:    true  ... no error
//             false ... error
// **************************************************************************
bool CORECOMM::SendStateVector2CoreModule(const STATEVECTOR *statevector) const
{
TWinSocketStream        *pStream;
COREMESSAGE     *coremessage;

 // error and consistency checking
 if (!Connected())       return(false);       // no connection to the destination core module

 pStream=new TWinSocketStream(GetSocket(), 5000);

 // create a new core message
 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_STATEVECTOR);
 coremessage->SetSuppDescriptor(0);
 coremessage->SetLength(statevector->GetStateVectorLength());         // set the length of the coremessage

 // copy the state vector into the core message
 memcpy(coremessage->GetBufPtr(), statevector->GetStateVectorPtr(), statevector->GetStateVectorLength());

 // and send it off
 coremessage->SendCoreMessage(pStream);     // and send it out
 delete coremessage;
 delete pStream;

 return(true);
}


// **************************************************************************
// Function:   SendStateVector2CoreModule
// Purpose:    sends the statevector on to another Core Module
// Parameters: statevector ... pointer to the statevector
// Returns:    true  ... no error
//             false ... error
// **************************************************************************
void CORECOMM::SendSysCommand(const char *syscmdbuf) const
{
TWinSocketStream        *pStream;
COREMESSAGE             *coremessage;

 pStream=new TWinSocketStream(GetSocket(), 5000);

 // send a system command to the operator
 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_SYSCMD);
 coremessage->SetLength((unsigned short)strlen(syscmdbuf)+1);
 sprintf(coremessage->GetBufPtr(), syscmdbuf, strlen(syscmdbuf));
 coremessage->SendCoreMessage(pStream);

 delete coremessage;
 delete pStream;
}



