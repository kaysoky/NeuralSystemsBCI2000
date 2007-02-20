/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//----------------------------------------------------------------------------
#ifndef UStateDialogH
#define UStateDialogH
//----------------------------------------------------------------------------
#include <vcl\System.hpp>
#include <vcl\Windows.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Classes.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\ExtCtrls.hpp>
#include "CSPIN.h"
//----------------------------------------------------------------------------
class TOKBottomDlg : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TBevel *Bevel1;
        TLabel *Label1;
        TLabel *Label2;
        TCSpinEdit *eStateBox;
        TRadioButton *bBooleanOr;
        TRadioButton *bBooleanAnd;
        TRadioButton *bJustSet;
        TCheckBox *cBit9;
        TCheckBox *cBit10;
        TCheckBox *cBit11;
        TLabel *Label3;
        TCheckBox *cBit12;
        TCheckBox *cBit13;
        TCheckBox *cBit14;
        TCheckBox *cBit15;
        TCheckBox *cBit16;
        TEdit *eBit9;
        TEdit *eBit10;
        TEdit *eBit11;
        TEdit *eBit12;
        TEdit *eBit13;
        TEdit *eBit14;
        TEdit *eBit15;
        TEdit *eBit16;
        TLabel *Label4;
        TLabel *Label5;
        TLabel *Label6;
        TLabel *Label7;
        TLabel *Label8;
        TLabel *Label9;
        TLabel *Label10;
        TLabel *Label11;
        TLabel *Label12;
        void __fastcall OKBtnClick(TObject *Sender);
        void __fastcall CancelBtnClick(TObject *Sender);
        void __fastcall cBit9Click(TObject *Sender);
        void __fastcall cBit10Click(TObject *Sender);
        void __fastcall cBit11Click(TObject *Sender);
        void __fastcall cBit12Click(TObject *Sender);
        void __fastcall cBit13Click(TObject *Sender);
        void __fastcall cBit14Click(TObject *Sender);
        void __fastcall cBit15Click(TObject *Sender);
        void __fastcall cBit16Click(TObject *Sender);
private:
public:
	virtual __fastcall TOKBottomDlg(TComponent* AOwner);
        void RecalculateBits();
};
//----------------------------------------------------------------------------
extern PACKAGE TOKBottomDlg *OKBottomDlg;
//----------------------------------------------------------------------------
#endif    


