////////////////////////////////////////////////////////////////////////////////
//
// File: MainForm.cpp
//
// Date: May 25, 2002
//
// Author: Juergen Mellinger
//
// Description: Main Form class for a drag and drop converter program that
//              reads BCI 2000 compliant EEG files and outputs files needed
//              for data import into the BrainVision Analyzer program.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef MainFormH
#define MainFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <CheckLst.hpp>
#include <ComCtrls.hpp>
#include "BCIReader.h"
//---------------------------------------------------------------------------
class TImporterForm : public TForm
{
__published:	// Von der IDE verwaltete Komponenten
    TMemo *ChannelNamesMemo;
    TCheckListBox *StatesList;
    TStatusBar *StatusBar;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TLabel *Label4;
    TPanel *DropPanel;
    void __fastcall StatesListKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// Anwender-Deklarationen
    void ReadSettings();
    void WriteSettings();
public:		// Anwender-Deklarationen
    __fastcall TImporterForm(TComponent* Owner);
    __fastcall ~TImporterForm();
    void ProcessFiles( TStrSet& inFiles, bool scanOnly );
    void EnableAll();
    void DisableAll();
private:
    void __fastcall DropPanelWindowProc( TMessage& );
    TWndMethod defaultDropPanelWindowProc;
    void __fastcall StatesListWindowProc( TMessage& );
    TWndMethod defaultStatesListWindowProc;
};
//---------------------------------------------------------------------------
extern PACKAGE TImporterForm *ImporterForm;
//---------------------------------------------------------------------------
#endif
