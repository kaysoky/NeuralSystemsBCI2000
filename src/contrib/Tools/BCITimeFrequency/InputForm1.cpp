/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include "InputForm1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TInputForm *InputForm;
//---------------------------------------------------------------------------
__fastcall TInputForm::TInputForm(TComponent* Owner)
        : TForm(Owner)
{
}

//---------------------------------------------------------------------------
void __fastcall TInputForm::CheckAllChClick(TObject *Sender)
{
        int i;
        char chr[16];

        ChanList->Lines->Clear();

        if( AllCh->Checked == true )
        {
                for(i=0;i<64;i++)
                {
                        ChanList->Lines->Add( itoa(i+1,chr,10) );

                }
        }
}
//---------------------------------------------------------------------------
void __fastcall TInputForm::SpatialFilterClick(TObject *Sender)
{
        OpenSpatialFile->Execute();
        vSpatialFile->Text= OpenSpatialFile->FileName;
}
//---------------------------------------------------------------------------

void __fastcall TInputForm::TemporalFilterClick(TObject *Sender)
{
        OpenTemporalFilter->Execute();
        vTemporalFile->Text= OpenTemporalFilter->FileName;
}
//---------------------------------------------------------------------------


void __fastcall TInputForm::CheckSpatialFilterClick(TObject *Sender)
{
        if(CheckSpatialFilter->Checked == true )
        {
                SpatialFilter->Visible= true;
                vSpatialFile->Visible= true;
                CheckAlign->Visible= true;
        }
        else
        {
                SpatialFilter->Visible= false;
                vSpatialFile->Visible= false;
                CheckAlign->Visible= false;
        }
}
//---------------------------------------------------------------------------

void __fastcall TInputForm::CheckTemporalFilterClick(TObject *Sender)
{
        if(CheckTemporalFilter->Checked == true )
        {
                TemporalFilter->Visible= true;
                vTemporalFile->Visible= true;
        }
        else
        {
                TemporalFilter->Visible= false;
                vTemporalFile->Visible= false;
        }

}
//---------------------------------------------------------------------------

void __fastcall TInputForm::CheckStateListClick(TObject *Sender)
{
        if( CheckStateList->Checked == true )
                StateList->Visible= true;
        else
        {
                StateList->Visible= false;
                StateList->Lines->Clear();
        }
}                

//---------------------------------------------------------------------------

void __fastcall TInputForm::BaselineClick(TObject *Sender)
{
        if( Baseline->ItemIndex == 1 )
        {
                Label2->Visible= true;
                vStartBase->Visible= true;
                Label3->Visible= true;
                vEndBase->Visible= true;
        }
        else
        {
                Label2->Visible= false;
                vStartBase->Visible= false;
                Label3->Visible= false;
                vEndBase->Visible= false;
        }
}
//---------------------------------------------------------------------------

