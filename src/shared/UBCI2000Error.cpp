//---------------------------------------------------------------------------
#pragma hdrstop

#include <string.h>

#include "UBCI2000Error.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


void BCI2000ERROR::SetErrorMsg(char *new_error)
{
 strncpy(error, new_error, ERR_MAXLENGTH);
 error[ERR_MAXLENGTH-1]=0;
}


void BCI2000ERROR::CopyError(BCI2000ERROR *new_error)
{
 strncpy(error, new_error->GetErrorMsg(), ERR_MAXLENGTH);
 error[ERR_MAXLENGTH-1]=0;
 code=new_error->GetErrorCode();
}


char *BCI2000ERROR::GetErrorMsg()
{
 return(error);
}


int BCI2000ERROR::GetErrorCode()
{
 return(code);
}
