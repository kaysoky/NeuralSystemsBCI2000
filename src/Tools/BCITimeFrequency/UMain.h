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
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

#include "BCIOutput.h"
/**************************************************************
        UMain is the main module of the BCITime application
***************************************************************/

#include "BCIInput.h"
#include "ParmIO.h"



class TfMain : public TForm
{
__published:	// IDE-managed Components
        TButton *Convert;
        TEdit *eSourceFile;
        TLabel *Label1;
        TLabel *Label2;
        TEdit *eDestinationFile;
        TCGauge *Gauge;
        TOpenDialog *OpenDialog;
        TButton *bOpenFile;
        TButton *Button1;
        TSaveDialog *SaveDialog;
        TButton *Button2;
        TEdit *vCalibrationFile;
        TLabel *Label5;
        TOpenDialog *OpenParameter;
        TListBox *FileList;
        TButton *AddFile;
        TButton *AddDirectory;
        TButton *Clear;
        TButton *StateControl;
        TButton *Button3;
        TButton *OutputControl;
        TButton *InputControl;
        TButton *ProcessControl;
        TButton *Button4;
        TEdit *vParmFile;
        TOpenDialog *OpenParameterFile;
        TButton *Button5;
        TLabel *Label3;
        TSaveDialog *SaveParameterFile;
        TCheckBox *FileType;

        void __fastcall ConvertClick(TObject *Sender);
        void __fastcall bOpenFileClick(TObject *Sender);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall AddFileClick(TObject *Sender);
        void __fastcall AddDirectoryClick(TObject *Sender);
        void __fastcall ClearClick(TObject *Sender);
        void __fastcall StateControlClick(TObject *Sender);
        void __fastcall Button3Click(TObject *Sender);
        void __fastcall OutputControlClick(TObject *Sender);
        void __fastcall InputControlClick(TObject *Sender);
        void __fastcall ProcessControlClick(TObject *Sender);
        void __fastcall Button5Click(TObject *Sender);
        void __fastcall Button4Click(TObject *Sender);
private:	// User declarations

        const STATEVECTOR *statevector;
        BCI2000DATA *bci2000data;
        BCIInput *bciinput;
        BCIOutput *bcioutput;
  //      ParIO *pario;
        MEM *mem;
        int     ret, firstrun, lastrun, cur_run, channel;
        ULONG   sample, numsamples;
        __int16 samplingrate, channels;
        // float val;
        FILE *cf;
        bool Process();
        void ApplyParamFile( const char* inParamFile );

public:		// User declarations
        __fastcall TfMain(TComponent* Owner);
        __fastcall ~TfMain( void );
        void ProcessCommandLineOptions();
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
