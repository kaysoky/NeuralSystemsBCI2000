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
//---------------------------------------------------------------------------

#define MAXCHANS  64

class TfMain : public TForm
{
__published:	// IDE-managed Components
        TEdit *eSourceFile;
        TLabel *Label1;
        TLabel *Label2;
        TEdit *eDestinationFile;
        TCGauge *Gauge;
        TOpenDialog *OpenDialog;
        TButton *bOpenFile;
        TButton *Button1;
        TSaveDialog *SaveDialog;
        TButton *bConvert;
        TLabel *Label3;
        TEdit *frun;
        TLabel *Label4;
        TEdit *lrun;
        TOpenDialog *OpenParameter;
        TButton *bDefineInput;
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
        TOpenDialog *OpenDialog1;
        TOpenDialog *OpenDialog2;
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
        TCheckListBox *cRunListBox;
        void __fastcall bOpenFileClick(TObject *Sender);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall ContinueClick(TObject *Sender);
        void __fastcall bDefineInputClick(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall frunChange(TObject *Sender);
private:	// User declarations
        BCI2000DATA     *bci2000data;
public:		// User declarations
        __fastcall TfMain(TComponent* Owner);
        int     IncrementTrial(int, const STATEVECTOR *);
        void    UpdateStateListBox(int);
        bool    SaveSampleOrNot(const STATEVECTOR *statevector);
        int     InitMatlabEngine();
        void    ShutdownMatlabEngine();
        int     GetNumSamples();
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
