//---------------------------------------------------------------------------
#ifndef UEditChannellistH
#define UEditChannellistH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "CSPIN.h"
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfEditChannellist : public TForm
{
__published:	// IDE-managed Components
        TLabel *Label1;
        TCSpinEdit *cFrom;
        TCSpinEdit *cTo;
        TLabel *Label2;
        TLabel *Label3;
        TRadioGroup *RadioGroup1;
        TRadioButton *rNormalDisplay;
        TRadioButton *rLgLapDisplay;
        TRadioButton *rCARDisplay;
        TButton *bApply;
        TButton *bClose;
        void __fastcall bApplyClick(TObject *Sender);
        void __fastcall bCloseClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TfEditChannellist(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfEditChannellist *fEditChannellist;
//---------------------------------------------------------------------------
#endif
