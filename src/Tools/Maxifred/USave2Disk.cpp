//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "USave2Disk.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfSave2Disk *fSave2Disk;
//---------------------------------------------------------------------------
__fastcall TfSave2Disk::TfSave2Disk(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfSave2Disk::bSaveClick(TObject *Sender)
{
 save=true;
 filename=eFilename->Text;
 Close();
}
//---------------------------------------------------------------------------
void __fastcall TfSave2Disk::bCancelClick(TObject *Sender)
{
 save=false;
 Close();
}
//---------------------------------------------------------------------------
