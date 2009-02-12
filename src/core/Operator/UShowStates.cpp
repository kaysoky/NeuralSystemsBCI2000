/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "UShowStates.h"
#include "StateList.h"
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


void __fastcall TfShowStates::FormShow(TObject*)
{
 StateListBox->Clear();
 for (int state=0; state < statelist->Size(); state++)
  StateListBox->Items->Add((*statelist)[state].Name().c_str());
}
//---------------------------------------------------------------------------
