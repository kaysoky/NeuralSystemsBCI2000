//---------------------------------------------------------------------------

#ifndef UAboutH
#define UAboutH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TfAbout : public TForm
{
__published:	// IDE-managed Components
        TPanel *Panel1;
        TImage *ProgramIcon;
        TLabel *Version;
        TLabel *Copyright;
        TLabel *Label1;
        TBevel *Bevel1;
        TButton *OKButton;
        TLabel *Label2;
        TLabel *Label3;
        TLabel *Label4;
        void __fastcall OKButtonClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TfAbout(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfAbout *fAbout;
//---------------------------------------------------------------------------
#endif
