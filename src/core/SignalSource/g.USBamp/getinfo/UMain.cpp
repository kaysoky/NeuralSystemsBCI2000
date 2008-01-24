///////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org
// Description: A command line utility to retrieve information about possible
//   configurations of a physically connected g.USBamp.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#pragma hdrstop

#include <stdio.h>
#include <windows.h>
#include "gUSBamp.h"

//---------------------------------------------------------------------------

#define MAX_USBAMPS     32

#pragma argsused

void print_info();

int main(int argc, char* argv[])
{
  printf("*******************************************\r\n");
  printf("BCI2000 Information Tool for g.USBamp\r\n");
  printf("*******************************************\r\n");
  printf("(C)2004 Gerwin Schalk\r\n");
  printf("        Wadsworth Center\r\n");
  printf("        New York State Department of Health\r\n");
  printf("        Albany, NY, USA\r\n");
  printf("*******************************************\r\n");

  print_info();

  return 0;
}
//---------------------------------------------------------------------------


void print_info()
{
HANDLE hdev;

 int firstamp=-1;
 // go through all possible USB connectors to find out what we have connected
 for (int cur_amp=0; cur_amp<MAX_USBAMPS; cur_amp++)
  {
  hdev = GT_OpenDevice(cur_amp);
  if (hdev)
     {
     char buf[1000];
     GT_GetSerial(hdev, (LPSTR)buf, 1000);
     printf("Amp found at USB address %d (S/N: %s)\r\n", cur_amp, buf);
     if (firstamp < 0) firstamp=cur_amp;
     GT_CloseDevice(&hdev);
     }
  }

 // no amp detected
 if (firstamp < 0)
    {
    printf("No g.USBamp detected. Aborting ...\r\n");
    return;
    }

 printf("Printing info for first amp (USB address %d)\r\n", firstamp);
 hdev = GT_OpenDevice(firstamp);
 if (hdev)
    {
    // get filter settings
    int nof;
    FILT *filt;
    GT_GetNumberOfFilter(&nof);
    filt = new _FILT[nof];
    printf("\r\nAvailable bandpass filters\r\n");
    printf("===================================\r\n");
    printf("num| hpfr  | lpfreq |  sfr | or | type\r\n");
    printf("===================================\r\n");
    for (int no_filt=0; no_filt<nof; no_filt++)
     {
     GT_GetFilterSpec(filt);
     printf("%03d| %5.2f | %6.1f | %4.0f | %2.0f | %1.0f\r\n", no_filt, filt[no_filt].fu, filt[no_filt].fo, filt[no_filt].fs, filt[no_filt].order, filt[no_filt].type);
     }
    delete filt;
    // get notch filter settings
    GT_GetNumberOfNotch(&nof);
    filt = new _FILT[nof];
    printf("\r\nAvailable notch filters\r\n");
    printf("===================================\r\n");
    printf("num| hpfr  | lpfreq |  sfr | or | type\r\n");
    printf("===================================\r\n");
    for (int no_filt=0; no_filt<nof; no_filt++)
     {
     GT_GetNotchSpec(filt);
     printf("%03d| %5.2f | %6.1f | %4.0f | %2.0f | %1.0f\r\n", no_filt, filt[no_filt].fu, filt[no_filt].fo, filt[no_filt].fs, filt[no_filt].order, filt[no_filt].type);
     }
    delete filt;

    GT_CloseDevice(&hdev);
    }
}
