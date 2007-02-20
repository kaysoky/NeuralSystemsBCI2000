/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UMainH
#define UMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "CGAUGES.h"
#include <Dialogs.hpp>
#include <CheckLst.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

#define MAXCHANS  64

class TfMain : public TForm
{
__published:	// IDE-managed Components
        TLabel *Label2;
        TEdit *eDestinationFile;
        TCGauge *Gauge;
        TOpenDialog *OpenDialog;
        TButton *bOpenFile;
        TButton *bOutputFile;
        TSaveDialog *SaveDialog;
        TButton *bConvert;
        TCheckListBox *cStateListBox;
        TLabel *Label5;
        TLabel *Label6;
        TLabel *Label7;
        TEdit *eITIStateValue;
        TLabel *Label8;
        TLabel *Label9;
        TLabel *Label10;
        TLabel *Label11;
        TLabel *Label12;
        TEdit *eState1aVal;
        TEdit *eState1bVal;
        TEdit *eState2bVal;
        TEdit *eState2aVal;
        TLabel *Label13;
        TLabel *Label14;
        TLabel *Label15;
        TLabel *Label16;
        TLabel *Label17;
        TLabel *Label18;
        TComboBox *ListBox1a;
        TComboBox *ListBox1b;
        TComboBox *ListBox2a;
        TComboBox *ListBox2b;
        TComboBox *ITIstateListBox;
        TRadioButton *rExportMatlab;
        TRadioButton *rExportFile;
        TMemo *mFilenames;
        TButton *bClearList;
        TBevel *Bevel1;
        TRadioGroup *ExportDataType;
        void __fastcall bOpenFileClick(TObject *Sender);
        void __fastcall bOutputFileClick(TObject *Sender);
        void __fastcall ContinueClick(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall bClearListClick(TObject *Sender);
private:	// User declarations
        class BCI2000DATA* bci2000data;
public:		// User declarations
        __fastcall TfMain(TComponent* Owner);
        bool    DefineInput(AnsiString);
        int     Process();
        int     IncrementTrial(int, const class STATEVECTOR*);
        void    UpdateStateListBox();
        bool    SaveSampleOrNot(const class STATEVECTOR*);
        int     InitMatlabEngine();
        void    ShutdownMatlabEngine();
        void    ProcessCommandLineOptions();
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif


