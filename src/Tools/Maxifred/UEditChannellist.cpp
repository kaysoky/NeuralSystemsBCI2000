//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "defines.h"
#include "UMain.h"
#include "UEditChannellist.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CSPIN"
#pragma resource "*.dfm"

TfEditChannellist *fEditChannellist;

//---------------------------------------------------------------------------
__fastcall TfEditChannellist::TfEditChannellist(TComponent* Owner)
        : TForm(Owner)
{
}

//---------------------------------------------------------------------------
void __fastcall TfEditChannellist::bApplyClick(TObject *Sender)
{
int     from, to, i, reftype;

 from=cFrom->Value-1;
 to=cTo->Value-1;

 reftype=REFTYPE_NORMAL;
 if (rLgLapDisplay->Checked)
    reftype=REFTYPE_LGLAP;
 if (rCARDisplay->Checked)
    reftype=REFTYPE_CAR;

 for (i=0; i<MAX_CHANNELS; i++)
  if ((i >= from) && (i <= to))
     fMain->channellist[i].chanreftype=reftype;

 fMain->UpdateMainChart();
 Application->MessageBox("Reference type successfully applied", "Message", MB_OK);
}


//---------------------------------------------------------------------------
void __fastcall TfEditChannellist::bCloseClick(TObject *Sender)
{
 Close();        
}
//---------------------------------------------------------------------------
