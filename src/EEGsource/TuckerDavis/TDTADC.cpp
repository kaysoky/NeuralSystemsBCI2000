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
