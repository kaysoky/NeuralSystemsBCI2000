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
    Status[ i ] = "no status available";
    Address[ i ] = "";
    EOS[ i ] = false;
    INI[ i ] = false;
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

