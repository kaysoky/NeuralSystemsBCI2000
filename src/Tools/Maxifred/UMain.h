//---------------------------------------------------------------------------
#ifndef UMainH
#define UMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Chart.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <TeEngine.hpp>
#include <TeeProcs.hpp>
#include "CSPIN.h"
#include "CGAUGES.h"
//---------------------------------------------------------------------------

#include "UBCI2000Data.h"
#include <CheckLst.hpp>

class TfMain : public TForm
{
__published:	// IDE-managed Components
        TOpenDialog *OpenDialog;
        TEdit *eFilename;
        TButton *bGetFile;
        TButton *bGoButton;
        TCSpinEdit *eRun;
        TLabel *Label1;
        TLabel *Label20;
        TCSpinEdit *eTrial;
        TUpDown *UpDown;
        TUpDown *LeftRight;
        TButton *bJump;
        TStatusBar *tMainStatusBar;
        TLabel *Label3;
        TCSpinEdit *cNumChannels;
        TLabel *Label4;
        TCSpinEdit *cDisplaySamples;
        TBevel *Bevel1;
        TButton *bEditChannellist;
        TButton *bGetCHDescFile;
        TOpenDialog *OpenDialog1;
        TButton *bPrint;
        TButton *bCopy2Clipboard;
        TCGauge *cMainGauge;
        TChart *MainChart;
        TButton *bSave2Disk;
        TCheckListBox *cStateListBox;
        TLabel *Label2;
        TEdit *eScaling;
        void __fastcall bGoButtonClick(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall LeftRightMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall UpDownMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall FormKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void __fastcall bGetFileClick(TObject *Sender);
        void __fastcall MainChartGetAxisLabel(TChartAxis *Sender,
          TChartSeries *Series, int ValueIndex, AnsiString &LabelText);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall MainChartAfterDraw(TObject *Sender);
        void __fastcall bJumpClick(TObject *Sender);
        void __fastcall MainChartMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall bEditChannellistClick(TObject *Sender);
        void __fastcall bGetCHDescFileClick(TObject *Sender);
        void __fastcall bPrintClick(TObject *Sender);
        void __fastcall bCopy2ClipboardClick(TObject *Sender);
        void __fastcall MainChartMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall MainChartMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
        void __fastcall bSave2DiskClick(TObject *Sender);
        void __fastcall cStateListBoxClickCheck(TObject *Sender);
private:	// User declarations
        void __fastcall CSpEditOnChange( TObject* );
public:		// User declarations
        __int16 UserStateVal;
        bool    UserStateEntered;
        int     SetStateType;
        __fastcall TfMain(TComponent* Owner);
        __fastcall ~TfMain();
        float   Get_CAR_Value(ULONG sample);
        void    UpdateMainChart();
        int     FetchChannelNumber(int displaynumber);
        AnsiString GetStateText(int state);
        void    InitializeGraph();
        void    UpdateStatusBar(AnsiString text);
        CHANNELPROPERTY channellist[MAX_CHANNELS];
        void    get_next_string(char *buf, int *start_idx, char *dest);
        void    TCanvas3DRectangle(int x1, int x2);
        void    Set_State(long samplestart, long sampleend, __int16 state, int setstatetype);
        void    ReadBCI2000Header(char *filename, EEGFILEINFO *EEGfileinfo);
        bool    readonly;
        AnsiString      inputfile;
        BCI2000DATA     *bci2000data;
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif



