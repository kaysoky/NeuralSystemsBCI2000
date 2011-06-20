#include "lpt.h"

#include <stdio.h>
#include <windows.h>
typedef UINT (CALLBACK* LPFNDLLFUNC1)(INT,INT);

HINSTANCE hDLL = NULL;
LPFNDLLFUNC1 Out32 = NULL;

void InitLPT(void)
{
	Out32 = NULL;
	hDLL = LoadLibrary("inpout32"); 
    if(hDLL == NULL) fprintf(stderr, "failed to load inpout32.dll\n");
	else {
		Out32 = (LPFNDLLFUNC1)GetProcAddress(hDLL, "Out32"); 
		if (Out32 == NULL) { 
			fprintf(stderr, "failed to load function Out32 from Inpout32.dll\n");
			FreeLibrary(hDLL); 
	    }
	}
	//printf("inited\n");
}
int CheckLPT(void)
{
	return (Out32 != NULL);
}
void CleanupLPT(void)
{
	Out32 = NULL;
	if(hDLL) FreeLibrary(hDLL); 
}

void LPTOut(int val, int port)
{
	if(Out32 == NULL) fprintf(stderr, "no LPT output function loaded\n");
	Out32(port, val);
}
