/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef CalibGenMainH
#define CalibGenMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include "CSPIN.h"
//---------------------------------------------------------------------------
#define MAX_CHANNELS            512
#define FILEFORMAT_BCI2000      1
#define FILEFORMAT_ALBANYOLD    2


class CALIBCONFIG
{
public:		// User declarations
        bool    valid;
        int     channels;
        float   calibvalue[MAX_CHANNELS];
        short   offset[MAX_CHANNELS];
        float   gain[MAX_CHANNELS];
        short   target;
        AnsiString      filename;
};


class SOURCECONFIG
{
public:		// User declarations
        bool    valid;
        int     channels;
        int     sample_freq;
        int     fileformat;
        int     headerlength;
        int     statevectorlength;
        float   targetvalue;
        long    filesize;
        AnsiString      filename;
        AnsiString      outputfilename;
};


class TfMain : public TForm
{
__published:	// IDE-managed Components
        TLabel *tFilename;
        TLabel *Label1;
        TLabel *Label2;
        TLabel *tCalibFile;
        TLabel *Label3;
        TLabel *Label4;
        TLabel *Label6;
        TLabel *tNumChannelsInput;
        TButton *bSelectInput;
        TButton *bSelectCalib;
        TButton *bGo;
        TOpenDialog *OpenDialog;
        TLabel *Label5;
        TLabel *Label7;
        TButton *bHelp;
        TLabel *Label8;
        TSaveDialog *SaveDialog;
        TEdit *eTargetVal;
        void __fastcall bSelectInputClick(TObject *Sender);
        void __fastcall bSelectCalibClick(TObject *Sender);
        void __fastcall bGoClick(TObject *Sender);
        void __fastcall bHelpClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TfMain(TComponent* Owner);
        void TfMain::get_next_string(char *buf, int *start_idx, char *dest);
        void TfMain::ReadBCI2000Header(SOURCECONFIG *sourceconfig);
        void TfMain::ReadHeader(SOURCECONFIG *sourceconfig);
        void TfMain::ResetGUI();
        long TfMain::filesize(FILE *stream);
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
