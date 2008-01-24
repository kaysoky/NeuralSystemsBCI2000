/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop

#include "USysStatus.h"

#pragma package(smart_init)

SYSSTATUS::SYSSTATUS()
{
  ResetSysStatus();
}

void SYSSTATUS::ResetSysStatus()
{
  SystemState = Idle;
  for( int i = 0; i < numModules; ++i )
  {
    Version[ i ] = ProtocolVersion::None();
    Status[ i ] = "no status available";
    Address[ i ] = "";
    EOS[ i ] = false;
    INI[ i ] = false;
    suspendConfirmed[ i ] = false;
    runningConfirmed[ i ] = false;
    NumMessagesRecv[ i ] = 0;
    NumStatesRecv[ i ] = 0;
    NumStateVecsRecv[ i ] = 0;
    NumParametersRecv[ i ] = 0;
    NumDataRecv[ i ] = 0;
    NumMessagesSent[ i ] = 0;
    NumStatesSent[ i ] = 0;
    NumStateVecsSent[ i ] = 0;
    NumParametersSent[ i ] = 0;
  }
}

