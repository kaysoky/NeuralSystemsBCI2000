//---------------------------------------------------------------------------

#ifndef UVisConfigH
#define UVisConfigH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "CSPIN.h"
//---------------------------------------------------------------------------
class TfVisConfig : public TForm
{
__published:	// IDE-managed Components
        TLabel *Label1;
        TCSpinEdit *cSourceID;
        TLabel *Label2;
        TLabel *Label3;
        TCSpinEdit *cTop;
        TLabel *Label4;
        TCSpinEdit *cLeft;
        TLabel *Label5;
        TCSpinEdit *cWidth;
        TLabel *Label6;
        TCSpinEdit *cHeight;
        TLabel *Label7;
        TLabel *Label8;
        TEdit *eDisplayMin;
        TEdit *eDisplayMax;
        TLabel *Label9;
        TLabel *Label10;
        TLabel *Label11;
        TCSpinEdit *CSpinEdit5;
        TCSpinEdit *CSpinEdit6;
        TButton *bSet;
        TLabel *Label12;
        TCSpinEdit *cNumSamples;
        void __fastcall bSetClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TfVisConfig(TComponent* Owner);
        bool    GetVisualPrefs(int sourceID, int vis_type, AnsiString variable, int *value);
        bool    GetVisualPrefs(int sourceID, int vis_type, AnsiString variable, double *value);
        bool    GetVisualPrefs(int sourceID, int vis_type, AnsiString variable, AnsiString *value);
        bool    SetVisualPrefs(int sourceID, int vis_type, AnsiString variable, int value);
        bool    SetVisualPrefs(int sourceID, int vis_type, AnsiString variable, double value);
        bool    SetVisualPrefs(int sourceID, int vis_type, AnsiString variable, AnsiString value);
};
//---------------------------------------------------------------------------
extern PACKAGE TfVisConfig *fVisConfig;
//---------------------------------------------------------------------------
#endif
