/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "defines.h"
#include "UMain.h"
#include "UStateDialog.h"
//---------------------------------------------------------------------
#pragma link "CSPIN"
#pragma resource "*.dfm"
TOKBottomDlg *OKBottomDlg;

//extern TfMain *fMain;

//---------------------------------------------------------------------
__fastcall TOKBottomDlg::TOKBottomDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
void __fastcall TOKBottomDlg::OKBtnClick(TObject *Sender)
{
 fMain->UserStateEntered=true;
 fMain->UserStateVal=(__int16)eStateBox->Value;
 if (bBooleanOr->Checked) fMain->SetStateType=SETSTATETYPE_BOOLEANOR;
 if (bBooleanAnd->Checked) fMain->SetStateType=SETSTATETYPE_BOOLEANAND;
 if (bJustSet->Checked) fMain->SetStateType=SETSTATETYPE_JUSTSET;
 Close();
}
//---------------------------------------------------------------------------

void __fastcall TOKBottomDlg::CancelBtnClick(TObject *Sender)
{
 fMain->UserStateEntered=false;
 Close();
}
//---------------------------------------------------------------------------


void TOKBottomDlg::RecalculateBits()
{
unsigned __int16 bits;

bits=0;
if (cBit9->Checked)  bits |= 1<<8;
if (cBit10->Checked) bits |= 1<<9;
if (cBit11->Checked) bits |= 1<<10;
if (cBit12->Checked) bits |= 1<<11;
if (cBit13->Checked) bits |= 1<<12;
if (cBit14->Checked) bits |= 1<<13;
if (cBit15->Checked) bits |= 1<<14;
if (cBit16->Checked) bits |= 1<<15;

eStateBox->Value=(int)bits;
}


void __fastcall TOKBottomDlg::cBit9Click(TObject *Sender)
{
 RecalculateBits();
}
//---------------------------------------------------------------------------

void __fastcall TOKBottomDlg::cBit10Click(TObject *Sender)
{
 RecalculateBits();
}
//---------------------------------------------------------------------------

void __fastcall TOKBottomDlg::cBit11Click(TObject *Sender)
{
 RecalculateBits();        
}
//---------------------------------------------------------------------------

void __fastcall TOKBottomDlg::cBit12Click(TObject *Sender)
{
 RecalculateBits();
}
//---------------------------------------------------------------------------

void __fastcall TOKBottomDlg::cBit13Click(TObject *Sender)
{
 RecalculateBits();        
}
//---------------------------------------------------------------------------

void __fastcall TOKBottomDlg::cBit14Click(TObject *Sender)
{
 RecalculateBits();        
}
//---------------------------------------------------------------------------

void __fastcall TOKBottomDlg::cBit15Click(TObject *Sender)
{
 RecalculateBits();        
}
//---------------------------------------------------------------------------

void __fastcall TOKBottomDlg::cBit16Click(TObject *Sender)
{
 RecalculateBits();        
}
//---------------------------------------------------------------------------



