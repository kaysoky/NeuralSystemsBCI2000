#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "UAbout.h"
#include "UMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfAbout *fAbout;
//---------------------------------------------------------------------------
__fastcall TfAbout::TfAbout(TComponent* Owner)
        : TForm(Owner)
{
  Version->Caption = TXT_OPERATOR_VERSION;
}
//---------------------------------------------------------------------------

void __fastcall TfAbout::OKButtonClick(TObject *Sender)
{
 Close();        
}
//---------------------------------------------------------------------------

