/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "OutputForm1.h"
#include "ProcessForm1.h"
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
        int i;

        if( OutputOrder->ItemIndex == 2 )
        {
                if( ProcessForm->UseMEM->Checked )
                {

                        TopoType->Caption= "Frequencies";

                        for(i=0;i<8;i++)
                        {
                                Times->Lines->Strings[i]= (i+2) * 3;
                        }
                }
                else
                {
                        TopoType->Caption= "Time (msec)";
                }

                TopoType->Visible= true;
                Times->Visible= true;
        }
        else
        {
                TopoType->Visible= false;
                Times->Visible= false;
        }

}
//---------------------------------------------------------------------------

