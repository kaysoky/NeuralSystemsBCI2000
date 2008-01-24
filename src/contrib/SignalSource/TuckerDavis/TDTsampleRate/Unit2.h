/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef Unit2H
#define Unit2H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <math.h>
//---------------------------------------------------------------------------
class TForm2 : public TForm
{
__published:	// IDE-managed Components
	TEdit *desiredSRbox;
	TEdit *realSRbox;
	TEdit *samplePeriodBox;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TButton *calcBut;
	void __fastcall calcButClick(TObject *Sender);
private:	// User declarations
	float TDTsampleRate;
public:		// User declarations
	__fastcall TForm2(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm2 *Form2;
//---------------------------------------------------------------------------
#endif
