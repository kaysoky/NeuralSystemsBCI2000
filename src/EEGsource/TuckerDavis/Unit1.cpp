//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "TDTBCI.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "RPCOXLib_OCX"
#pragma resource "*.dfm"
TfRPcoX *fRPcoX;
//---------------------------------------------------------------------------
__fastcall TfRPcoX::TfRPcoX(TComponent* Owner)
        : TForm(Owner)
{
    if( NULL == RPcoX1){
        MessageBox(NULL,
            "The forms RPcoX1 variable was null in FormCreate.\n \
                Exiting program.","Error", MB_OK);
         abort();
    }
    TDTBCI::RPcoX1 = RPcoX1;
}
//---------------------------------------------------------------------------
