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
#include <vector>
//---------------------------------------------------------------------------

class TfMain : public TForm
{
__published:	// IDE-managed Components
        TButton *bConvert;
        TEdit *eSourceFile;
        TLabel *Label1;
        TLabel *Label2;
        TEdit *eDestinationFile;
        TCGauge *Gauge;
        TOpenDialog *OpenDialog;
        TButton *bOpenFile;
        TButton *Button1;
        TSaveDialog *SaveDialog;
        TButton *Continue;
        TLabel *Label3;
        TEdit *frun;
        TLabel *Label4;
        TEdit *lrun;
        TButton *Button2;
        TEdit *ParameterFile;
        TLabel *Label5;
        TOpenDialog *OpenParameter;

        void __fastcall bConvertClick(TObject *Sender);
        void __fastcall bOpenFileClick(TObject *Sender);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall ContinueClick(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
private:	// User declarations
        BCI2000DATA *bci2000data;
        FILE    *fp;
        int     ret, firstrun, lastrun, cur_run, channel;
        ULONG   sample, numsamples;
        __int16 samplingrate, channels, dummy, cur_value, cur_state;
        float val;
        FILE *cf;
        std::vector<float> offset, gain;
        const short* gabTargets;
        bool CalibFromFile() { return gain.size() != 0; }
        void __fastcall CheckCalibrationFile( void );
        void __fastcall DoStartupProcessing( TObject*, bool& );
        bool autoMode;
      
public:		// User declarations
        __fastcall TfMain(TComponent* Owner);
        __int16 ConvertState(BCI2000DATA *bci2000data);
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
