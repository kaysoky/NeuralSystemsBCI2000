//----------------------------------------------------------------------------
#ifndef UAboutH
#define UAboutH
//----------------------------------------------------------------------------
#include <System.hpp>
#include <Windows.hpp>
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Graphics.hpp>
#include <Forms.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
//----------------------------------------------------------------------------
class TfAbout : public TForm
{
__published:
        TPanel *Panel1;
	TImage *ProgramIcon;
	TLabel *ProductName;
	TLabel *Version;
	TLabel *Copyright;
	TButton *OKButton;
        TLabel *Label1;
        TLabel *Label2;
        TLabel *Label3;
        TBevel *Bevel1;
        TLabel *Label4;
        TLabel *Label5;
        TLabel *Label6;
        TImage *Image1;
private:
public:
	virtual __fastcall TfAbout(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TfAbout *fAbout;
//----------------------------------------------------------------------------
#endif
