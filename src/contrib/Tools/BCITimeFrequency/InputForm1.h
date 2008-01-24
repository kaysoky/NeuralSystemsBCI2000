/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef InputForm1H
#define InputForm1H

#define MAXCHANS  65
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TInputForm : public TForm
{
__published:	// IDE-managed Components
        TLabel *Label1;
        TMemo *ChanList;
        TCheckBox *AllCh;
        TButton *SpatialFilter;
        TEdit *vSpatialFile;
        TButton *TemporalFilter;
        TEdit *vTemporalFile;
        TOpenDialog *OpenSpatialFile;
        TOpenDialog *OpenTemporalFilter;
        TCheckBox *CheckSpatialFilter;
        TCheckBox *CheckTemporalFilter;
        TCheckBox *CheckAlign;
        TCheckBox *CheckStateList;
        TMemo *StateList;
        TRadioGroup *Baseline;
        TLabel *Label2;
        TEdit *vStartBase;
        TLabel *Label3;
        TEdit *vEndBase;
        void __fastcall CheckAllChClick(TObject *Sender);
        void __fastcall SpatialFilterClick(TObject *Sender);
        void __fastcall TemporalFilterClick(TObject *Sender);
        void __fastcall CheckSpatialFilterClick(TObject *Sender);
        void __fastcall CheckTemporalFilterClick(TObject *Sender);
        void __fastcall CheckStateListClick(TObject *Sender);
        void __fastcall BaselineClick(TObject *Sender);
private:	// User declarations

public:		// User declarations
       
        __fastcall TInputForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TInputForm *InputForm;
//---------------------------------------------------------------------------
#endif
