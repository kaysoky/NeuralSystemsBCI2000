//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "USysStatus.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

SYSSTATUS::SYSSTATUS()
{
 ResetSysStatus();
}

void SYSSTATUS::ResetSysStatus()
{
 SourceConnected=false;
 SigProcConnected=false;
 ApplicationConnected=false;
 NumMessagesRecv1=NumMessagesRecv2=NumMessagesRecv3=0;
 NumStatesRecv1=NumStatesRecv2=NumStatesRecv3=0;
 NumStateVecsRecv1=NumStateVecsRecv2=NumStateVecsRecv3=0;
 NumParametersRecv1=NumParametersRecv2=NumParametersRecv3=0;
 NumDataRecv1=NumDataRecv2=NumDataRecv3=0;
 NumMessagesSent1=NumMessagesSent2=NumMessagesSent3=0;
 NumStatesSent1=NumStatesSent2=NumStatesSent3=0;
 NumStateVecsSent1=NumStateVecsSent2=NumStateVecsSent3=0;
 NumParametersSent1=NumParametersSent2=NumParametersSent3=0;
 SystemState=STATE_IDLE;
 EEGsourceEOS=SigProcEOS=ApplicationEOS=false;
 EEGsourceINI=SigProcINI=ApplicationINI=false;
 EEGsourceStatus=SigProcStatus=ApplicationStatus="no status available";
 EEGsourceStatusReceived=SigProcStatusReceived=ApplicationStatusReceived=false;
}

