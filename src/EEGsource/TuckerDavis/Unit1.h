//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "RPCOXLib_OCX.h"
#include <OleCtrls.hpp>
//---------------------------------------------------------------------------
class TfRPcoX : public TForm
{
__published:	// IDE-managed Components
        TRPcoX *RPcoX1;
private:	// User declarations
public:		// User declarations
        __fastcall TfRPcoX(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfRPcoX *fRPcoX;
//---------------------------------------------------------------------------
#endif
