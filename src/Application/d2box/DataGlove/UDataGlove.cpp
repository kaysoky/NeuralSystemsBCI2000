#pragma hdrstop
//---------------------------------------------------------------------------
#include "UDataGlove.h"

#include <stdio.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)


__fastcall DataGlove::DataGlove(): TThread(true)
{
 hCOM=NULL;
 critsec=new TCriticalSection();
 streaming=false;
 resyncerr=false;
 gloveerr=GLOVE_ERR_NOERR;
}


__fastcall DataGlove::~DataGlove()
{
 while (streaming)
  {
  Terminate();
  Sleep(0);
  }
  
 delete critsec;
}


// **************************************************************************
// Function:   GetSensorVal
// Purpose:    Returns the most recent sensor value for the glove
//             during streaming operation
// Parameters: int sensor - sensor number; should be between 0 and 6
// Returns:    sensor value
// **************************************************************************
unsigned char DataGlove::GetSensorVal(int sensor)
{
unsigned char retval;

 if ((sensor < 0) || (sensor >= MAX_GLOVESENSORS))
    gloveerr=GLOVE_ERR_SENSORNUMERR;

 critsec->Acquire();
 retval=(unsigned char)sensorval[sensor];
 critsec->Release();

 return(retval);
}


// **************************************************************************
// Function:   GetSensorValOld
// Purpose:    Returns the previous sensor value for the glove
//             during streaming operation
// Parameters: int sensor - sensor number; should be between 0 and 6
// Returns:    sensor value
// **************************************************************************
unsigned char DataGlove::GetSensorValOld(int sensor)
{
unsigned char retval;

 if ((sensor < 0) || (sensor >= MAX_GLOVESENSORS))
    gloveerr=GLOVE_ERR_SENSORNUMERR;

 critsec->Acquire();
 retval=(unsigned char)sensorval_old[sensor];
 critsec->Release();

 return(retval);
}


// **************************************************************************
// Function:   GetGloveErr
// Purpose:    Returns the current glove status
// Parameters: N/A
// Returns:    GLOVE_ERR_NOERR        - all OK
//             GLOVE_ERR_RESYNCERR    - the glove could not be resynchronized
//             GLOVE_ERR_SENSORNUMERR - sensor number out of range in GetSensorVal
// **************************************************************************
int DataGlove::GetGloveErr()
{
 if (resyncerr)
 gloveerr=GLOVE_ERR_RESYNCERR;

 return(gloveerr);
}


// 1) resets the glove
// 2) sends command to start streaming
// 3) waits until it gets the correct header
void DataGlove::ReSyncGlove()
{
char  buf[256], sendbuf[256];
unsigned char header;
DWORD numread, numwritten;

 CloseComPort();
 if (!OpenComPort(COMport))
    {
    resyncerr=true;
    return;
    }

 gloveerr=GLOVE_ERR_NOERR;

 int count=0;
 header=0;
 // wait until we get the correct header
 while (header != 0x80)
  {
  buf[0]=0;
  buf[1]=0;
  // reset the glove
  strcpy(sendbuf, "A");
  for (int i=0; i<10; i++)
   {
   WriteFile(hCOM, sendbuf, 1, &numwritten, NULL);
   // 'eat' all values until the expected return
   ReadFile(hCOM, buf, 1, &numread, NULL);
   if (buf[0] == 'U') break;
   }
  strcpy(sendbuf, "BA");
  WriteFile(hCOM, sendbuf, 2, &numwritten, NULL);
  ReadFile(hCOM, buf, 1, &numread, NULL);
  // if (buf[0] != 'A') return(false);
  // send command to start streaming
  strcpy(sendbuf, "D");
  WriteFile(hCOM, sendbuf, 1, &numwritten, NULL);

  // this should produce 0x80
  ReadFile(hCOM, &header, 1, &numread, NULL);       // read header
  if (count++ > 10)
     {
     resyncerr=true;
     break;
     }
  }
}

// main loop for glove streaming
void __fastcall DataGlove::Execute()
{
char  buf[256], sendbuf[256];
DWORD numread, numwritten;
unsigned char cur_sensorval;

 buf[0]=0;
 buf[1]=0;

 // reset the glove
 strcpy(sendbuf, "A");
 WriteFile(hCOM, sendbuf, 1, &numwritten, NULL);
 // 'eat' all values until the expected return
 while (buf[0] != 'U')
  ReadFile(hCOM, buf, 1, &numread, NULL);

 // send command to start streaming
 strcpy(sendbuf, "D");
 WriteFile(hCOM, sendbuf, 1, &numwritten, NULL);

 while (!Terminated)
  {
  // read header
  ReadFile(hCOM, buf, 1, &numread, NULL);
  if ((unsigned char)buf[0] != 0x80)
     ReSyncGlove();  // should always be 0x80 (128)
  // read sensor values
  for (int sensor=0; sensor<MAX_GLOVESENSORS; sensor++)
   {
   ReadFile(hCOM, &cur_sensorval, 1, &numread, NULL);
   critsec->Acquire();
   sensorval_old[sensor]=sensorval[sensor];
   sensorval[sensor]=cur_sensorval;
   critsec->Release();
   }
  // 'eat' checksum value
  ReadFile(hCOM, buf, 1, &numread, NULL);
  }

 CloseComPort();
 streaming=false;
}

// **************************************************************************
// Function:   StartStreaming
// Purpose:    Start streaming the signals asynchronously
//             subsequent GetSensorVal() calls can read the sensors' position
// Parameters: COMport - COM port for the glove
// Returns:    true if the glove is present and the streaming started OK
// **************************************************************************
bool DataGlove::StartStreaming(AnsiString cur_COMport)
{
 COMport=cur_COMport;
 // if we are already streaming, we need to turn it off first
 if (streaming)
    {
    Terminate();
    while (streaming)
     Sleep(5);
    }

 if (!GlovePresent(COMport)) return(false);

 Priority = tpLower; // set the priority lower than normal
 FreeOnTerminate = false;

 // open the COM port
 if (!OpenComPort(COMport)) return(false);
 streaming=true;
 Resume();

 return(true);
}


// **************************************************************************
// Function:   GlovePresent
// Purpose:    Tests whether the glove is present on a given COM port
// Parameters: COMport - COM port for the glove
// Returns:    true if the glove is present
// **************************************************************************
bool DataGlove::GlovePresent(AnsiString COMport)
{
char  buf[256], sendbuf[256];
DWORD numread, numwritten;

 if (streaming)
    {
    Terminate();
    while (streaming)
     Sleep(5);
    }

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

 // set the glove to copy mode and send a few characters to check whether we get the right stuff back
 // send an 'A' first
 strcpy(sendbuf, "BA");
 WriteFile(hCOM, sendbuf, 2, &numwritten, NULL);
 ReadFile(hCOM, buf, 1, &numread, NULL);
 if (buf[0] != 'A') return(false);
 // then a '1'
 strcpy(sendbuf, "B1");
 WriteFile(hCOM, sendbuf, 2, &numwritten, NULL);
 ReadFile(hCOM, buf, 1, &numread, NULL);
 if (buf[0] != '1') return(false);
 // finally a 'r'
 strcpy(sendbuf, "Br");
 WriteFile(hCOM, sendbuf, 2, &numwritten, NULL);
 ReadFile(hCOM, buf, 1, &numread, NULL);
 if (buf[0] != 'r') return(false);

 CloseComPort();
 return(true);
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

 if (streaming)
    {
    Terminate();
    while (streaming)
     Sleep(5);
    }

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