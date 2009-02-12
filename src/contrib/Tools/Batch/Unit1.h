/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
        TButton *Button1;
        TButton *Button2;
        TEdit *vOperator;
        TEdit *vSource;
        TEdit *vSignalProcessing;
        TEdit *vApplication;
        TEdit *vIp;
        TButton *Save;
        TButton *Get;
        TEdit *vSave;
        TEdit *vGet;
        TOpenDialog *OpenSaved;
        TSaveDialog *SaveFile;
        TButton *Operator;
        TOpenDialog *OpenOperator;
        TButton *EEGsource;
        TOpenDialog *OpenSource;
        TButton *SignalProcess;
        TButton *Application;
        TOpenDialog *OpenSignalProcessing;
        TOpenDialog *OpenApplication;
        TLabel *IP;
        TLabel *Label1;
        TEdit *vDelay;
        TButton *Parmfile;
        TEdit *vParmfile;
        TOpenDialog *OpenParmfile;
        TSaveDialog *RunParmfile;
        TButton *RunParm;
        TEdit *vRunParm;
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall SaveClick(TObject *Sender);
        void __fastcall GetClick(TObject *Sender);
        void __fastcall OperatorClick(TObject *Sender);
        void __fastcall EEGsourceClick(TObject *Sender);
        void __fastcall SignalProcessClick(TObject *Sender);
        void __fastcall ApplicationClick(TObject *Sender);
        void __fastcall ParmfileClick(TObject *Sender);
        void __fastcall RunParmClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
