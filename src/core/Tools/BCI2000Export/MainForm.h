////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Main Form class for a drag and drop converter program that
//   reads BCI2000 compliant EEG files and converts them into other formats.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
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
#include <Menus.hpp>
#include "BCIReader.h"
#include <ActnList.hpp>
#include <StdActns.hpp>
//---------------------------------------------------------------------------
class TImporterForm : public TForm
{
__published:
    TMemo *ChannelNamesMemo;
    TCheckListBox *StatesList;
    TStatusBar *StatusBar;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label4;
    TPanel *DropPanel;
    TComboBox *FormatsBox;
    TMainMenu *MainMenu;
    TMenuItem *File1;
    TMenuItem *FileOpen;
    TMenuItem *FileQuit;
    TMenuItem *Edit1;
    TMenuItem *Cut1;
    TMenuItem *Copy;
    TMenuItem *Paste1;
    TMenuItem *Help1;
    TMenuItem *HelpAbout;
    TLabel *Label5;
    TPanel *Panel;
    TActionList *ActionList;
    TEditCopy *EditCopy;
    TEditCut *EditCut;
    TEditPaste *EditPaste;
    TMenuItem *HelpOpenHelp;
    void __fastcall StatesListKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall HelpAboutClick(TObject *Sender);
    void __fastcall FileOpenClick(TObject *Sender);
    void __fastcall FileQuitClick(TObject *Sender);
    void __fastcall HelpOpenHelpClick(TObject *Sender);
private:
    void ReadSettings();
    void WriteSettings();
public:
    __fastcall TImporterForm(TComponent* Owner);
    __fastcall ~TImporterForm();
    void ProcessFiles( StringSet& inFiles, bool scanOnly );
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
