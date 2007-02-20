/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UShowStatesH
#define UShowStatesH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#include "UState.h"

//---------------------------------------------------------------------------
class TfShowStates : public TForm
{
__published:	// IDE-managed Components
        TListBox *StateListBox;
        void __fastcall FormShow(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TfShowStates(TComponent* Owner);
        STATELIST  *statelist;
};
//---------------------------------------------------------------------------
extern PACKAGE TfShowStates *fShowStates;
//---------------------------------------------------------------------------
#endif


