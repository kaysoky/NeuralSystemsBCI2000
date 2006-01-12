#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "UShowStates.h"
#include "UBCIError.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfShowStates *fShowStates;
//---------------------------------------------------------------------------
__fastcall TfShowStates::TfShowStates(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------


void __fastcall TfShowStates::FormShow(TObject *Sender)
{
 StateListBox->Clear();
 for (size_t state=0; state < statelist->Size(); state++)
  StateListBox->Items->Add((*statelist)[state].GetName());
}
//---------------------------------------------------------------------------
