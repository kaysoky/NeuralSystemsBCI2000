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
#include <jpeg.hpp>
//---------------------------------------------------------------------------
class TfAbout : public TForm
{
__published:	// IDE-managed Components
        TButton *OKButton;
        TImage *ProgramIcon;
        TLabel *Copyright;
        TLabel *Label1;
        TLabel *Label3;
        TLabel *Label4;
        TLabel *Label2;
        TLabel *Label5;
        TLabel *Label7;
        TLabel *Label6;
        TLabel *Version;
        void __fastcall OKButtonClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TfAbout(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfAbout *fAbout;
//---------------------------------------------------------------------------
#endif
