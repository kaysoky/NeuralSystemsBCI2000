//---------------------------------------------------------------------------

#ifndef ProcessForm1H
#define ProcessForm1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TProcessForm : public TForm
{
__published:	// IDE-managed Components
        TCheckBox *UseMEM;
        TLabel *Label1;
        TLabel *Label2;
        TLabel *Label3;
        TLabel *Label4;
        TLabel *Label5;
        TLabel *Label6;
        TRadioGroup *Remove;
        TButton *Exit;
        TEdit *vStart;
        TEdit *vEnd;
        TEdit *vDensity;
        TEdit *vBandwidth;
        TEdit *vMemDataLength;
        TEdit *vModel;
        TLabel *Label7;
        TLabel *Label8;
        TEdit *vMemWindows;
        TEdit *vMemBlockSize;
        TRadioGroup *MemWinType;
        void __fastcall ExitClick(TObject *Sender);
        void __fastcall MemWinTypeClick(TObject *Sender);
        void __fastcall vMemWindowsChange(TObject *Sender);
        void __fastcall vMemBlockSizeChange(TObject *Sender);
        void __fastcall vMemDataLengthChange(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TProcessForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TProcessForm *ProcessForm;
//---------------------------------------------------------------------------
#endif
