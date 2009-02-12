/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef CalibGenHelpH
#define CalibGenHelpH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TfHelp : public TForm
{
__published:	// IDE-managed Components
        TMemo *Memo;
private:	// User declarations
public:		// User declarations
        __fastcall TfHelp(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfHelp *fHelp;
//---------------------------------------------------------------------------
#endif
