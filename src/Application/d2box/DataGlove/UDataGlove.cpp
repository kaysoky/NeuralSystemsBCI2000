#pragma hdrstop
//---------------------------------------------------------------------------
#include "UDataGlove.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


DataGlove::DataGlove()
{
 hCOM=NULL;
}


DataGlove::~DataGlove()
{
 CloseComPort();
}


// **************************************************************************
// Function:   DataGlove
// Purpose:
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
unsigned char DataGlove::GetSensorVal(int sensor)
{
 if ((sensor < 0) || (sensor >= MAX_GLOVESENSORS))
    throw;

 return((unsigned char)sensorval[sensor]);
}


// **************************************************************************
// Function:   DataGlove
// Purpose:
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
bool DataGlove::ReadSensorsFromGlove(AnsiString COMport)
{
char  buf[256], sendbuf[256];
DWORD numread, numwritten;

 buf[0]=0;
 buf[1]=0;

 // open the COM port
 if (!OpenComPort(COMport))
    return(false);

 // reset the glove
 strcpy(sendbuf, "A");
 WriteFile(hCOM, sendbuf, 1, &numwritten, NULL);
 // 'eat' all values until the expected return
 while (buf[0] != 'U')
  ReadFile(hCOM, buf, 1, &numread, NULL);

 // get one block of current sensor values
 strcpy(sendbuf, "C");
 WriteFile(hCOM, sendbuf, 1, &numwritten, NULL);
 ReadFile(hCOM, buf, 1, &numread, NULL);
 if ((unsigned char)buf[0] != 0x80)
    return(false);

 for (int sensor=0; sensor<MAX_GLOVESENSORS; sensor++)
  ReadFile(hCOM, &sensorval[sensor], 1, &numread, NULL);
 // 'eat' checksum value
 ReadFile(hCOM, buf, 1, &numread, NULL);

 CloseComPort();
 return(true);
}



// open the COM-Port
bool DataGlove::OpenComPort(AnsiString COMport)
{
SECURITY_ATTRIBUTES sa;
DWORD dwError=0;

 sa.nLength=sizeof(SECURITY_ATTRIBUTES);
 sa.lpSecurityDescriptor=NULL;
 sa.bInheritHandle=false; //true; // geht in nt

 // open the COM-port
 hCOM=CreateFile(COMport.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

 if (hCOM == INVALID_HANDLE_VALUE)
    return false;
 if (!BuildCommDCB("baud=19200 parity=N data=8 stop=1",&dcb))
    return false;

 dcb.fDtrControl = DTR_CONTROL_DISABLE;
 dcb.fRtsControl = RTS_CONTROL_DISABLE;
 /* dcb.fOutxCtsFlow = FALSE;
 dcb.fDtrControl = DTR_CONTROL_DISABLE;
 dcb.fDsrSensitivity = FALSE;
 dcb.fOutX = FALSE;
 dcb.fInX = FALSE;
 dcb.fErrorChar = FALSE;
 dcb.fNull = FALSE;
 dcb.fAbortOnError = FALSE;  */

 if (!SetCommState(hCOM, &dcb))
    return false;
    
 ClearCommError(hCOM,&dwError,NULL);

return true;
}


// close the COM-Port
bool DataGlove::CloseComPort()
{
 if (hCOM)
    {
    PurgeComm(hCOM, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
    CloseHandle(hCOM);
    }

 hCOM=NULL;
 return true;
}