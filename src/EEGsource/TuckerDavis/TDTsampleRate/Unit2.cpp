/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit2.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm2 *Form2;
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
	: TForm(Owner)
{
	TDTsampleRate = 24414.0625;
}
//---------------------------------------------------------------------------



void __fastcall TForm2::calcButClick(TObject *Sender)
{
	float sr = atof(desiredSRbox->Text.c_str());

    int per = floor(TDTsampleRate / sr);

    realSRbox->Text = TDTsampleRate / per;
    samplePeriodBox->Text = per;

}
//---------------------------------------------------------------------------


