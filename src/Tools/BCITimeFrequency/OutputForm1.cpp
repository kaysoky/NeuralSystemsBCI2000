//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "OutputForm1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TOutputForm *OutputForm;
//---------------------------------------------------------------------------
__fastcall TOutputForm::TOutputForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TOutputForm::SubGroupsClick(TObject *Sender)
{
        if( SubGroups->Checked == true )
        {
                Label3->Visible= true;
                vCompuMeans->Visible= true;
                Label4->Visible= true;

        }
        else
        {
                Label3->Visible= false;
                vCompuMeans->Visible= false;
                Label4->Visible= false;
        }
}
//---------------------------------------------------------------------------

void __fastcall TOutputForm::OutputOrderClick(TObject *Sender)
{
        if( OutputOrder->ItemIndex == 2 )
        {
                Label5->Visible= true;
                Times->Visible= true;
        }
        else
        {
                Label5->Visible= false;
                Times->Visible= false;
        }

}
//---------------------------------------------------------------------------

