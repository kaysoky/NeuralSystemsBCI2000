//---------------------------------------------------------------------------

#ifndef OutputForm1H
#define OutputForm1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TOutputForm : public TForm
{
__published:	// IDE-managed Components
        TRadioGroup *OutputOrder;
        TLabel *Label1;
        TLabel *Label2;
        TEdit *vStart;
        TEdit *vEnd;
        TCheckBox *OverlapMode;
        TLabel *Label3;
        TEdit *vCompuMeans;
        TLabel *Label4;
        TRadioGroup *Statistics;
        TCheckBox *SubGroups;
        TLabel *Label5;
        TMemo *Times;
        TLabel *Label6;
        TEdit *vDecimate;
        void __fastcall SubGroupsClick(TObject *Sender);
        void __fastcall OutputOrderClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TOutputForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TOutputForm *OutputForm;
//---------------------------------------------------------------------------
#endif
