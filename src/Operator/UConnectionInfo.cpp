//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UConnectionInfo.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfConnectionInfo *fConnectionInfo;
//---------------------------------------------------------------------------
__fastcall TfConnectionInfo::TfConnectionInfo(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
