#ifndef LABVIEWDLL_H
#define LABVIEWDLL_H
#include <windows.h>
typedef HANDLE (*dOPEN_DRIVER_ASYNC)(void);
typedef BOOL (*dUSB_WRITE)(HANDLE HAN, PCHAR data);
typedef BOOL (*dREAD_MULTIPLE_SWEEPS)(HANDLE HAN, PCHAR data, DWORD nBytesToRead);
typedef BOOL (*dREAD_POINTER)(HANDLE HAN, PDWORD Pointer);
typedef BOOL (*dCLOSE_DRIVER_ASYNC)(HANDLE HAN);

#endif



