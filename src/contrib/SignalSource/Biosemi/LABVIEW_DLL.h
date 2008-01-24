/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef LABVIEWDLL_H
#define LABVIEWDLL_H
#include <windows.h>
typedef HANDLE (*dOPEN_DRIVER_ASYNC)(void);
typedef bool (*dUSB_WRITE)(HANDLE HAN, PCHAR data);
typedef bool (*dREAD_MULTIPLE_SWEEPS)(HANDLE HAN, PCHAR data, DWORD nBytesToRead);
typedef bool (*dREAD_POINTER)(HANDLE HAN, PDWORD Pointer);
typedef bool (*dCLOSE_DRIVER_ASYNC)(HANDLE HAN);
//typedef unsigned char (*dCLOSE_DRIVER)(HANDLE HAN);

#endif


 
