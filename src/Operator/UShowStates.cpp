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
int state;

 try{
 StateListBox->Clear();
 for (state=0; state < statelist->GetNumStates(); state++)
  StateListBox->Items->Add(statelist->GetStatePtr(state)->GetName());
 } catch( TooGeneralCatch& ) {;}
}
//---------------------------------------------------------------------------
