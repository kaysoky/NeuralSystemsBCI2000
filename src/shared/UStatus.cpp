#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "UStatus.h"

#include <string.h>
#include <stdlib.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)


int STATUS::ParseStatus(char *line, int length)
{
char    temp[4];

 if ((length > 4) && (length < LENGTH_STATUSLINE))
    {
    strncpy(temp, line, 3);
    temp[3]=0;
    code=atoi(temp);
    strncpy(status, line+4, length-4);
    status[length-4]=0;
    }
 else
    {
    code=0;
    status[0]=0;
    }

 return(ERRSTATUS_NOERR);
}


char *STATUS::GetStatus()
{
 return(status);
}


int STATUS::GetCode()
{
 return(code);
}

