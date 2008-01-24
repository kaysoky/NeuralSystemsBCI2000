/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------


#pragma hdrstop

#include "TDTADC.h"

using namespace std;

__fastcall TDTADC::TDTADC(bool createSuspended) : TThread( createSuspended )
{
	circuit = NULL;
    acquire = new TEvent( NULL, true, false, "" );
    executeSafe = new TEvent( NULL, true, true, "" );
}

__fastcall TDTADC::~TDTADC()
{
	if( isConnected() ){
        halt();
    }
}
//---------------------------------------------------------------------------

#pragma package(smart_init)
