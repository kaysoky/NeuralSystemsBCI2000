//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UCursor.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

CURSOR::CURSOR(TForm *form)
{
 cursor=new TShape(Application);
 cursor->Parent=form;
 Width=20;
 Height=20;
 cursor->Top=form->ClientHeight/2-Height/2;
 cursor->Left=form->ClientWidth/2-Width/2;
 cursor->Shape=stEllipse;
 cursor->Enabled=true;
 Hide();
}


CURSOR::~CURSOR()
{
 if (cursor) delete cursor;
 cursor=NULL;
}


void CURSOR::BringToFront()
{
 cursor->BringToFront();
}

void CURSOR::Show()
{
 cursor->Visible=true;
}

void CURSOR::Hide()
{
 cursor->Visible=false;
}


// **************************************************************************
// Function:   RenderCursor
// Purpose:    This function renders the cursor into the specified window
// Parameters: form     - pointer to the form that will hold the target
//             destrect - part of the form the status bar will be rendered into
// Returns:    N/A
// **************************************************************************
void CURSOR::RenderCursor(TForm *form, TRect destrect)
{
int     destwidth, destheight;
int     scaledtop, scaledleft, scaledbottom, scaledright;
int     scaledtextsize, scaledtextposx, scaledtextposy;
float   scalex, scaley;

 destwidth=destrect.Width();
 destheight=destrect.Height();
 scalex=(float)destwidth/(float)65536;
 scaley=(float)destheight/(float)65536;

 scaledtop=(int)((float)Top*scaley+(float)destrect.Top);
 scaledbottom=(int)((float)(Top+Height)*scaley+(float)destrect.Top);
 scaledleft=(int)((float)Left*scalex+(float)destrect.Left);
 scaledright=(int)((float)(Left+Width)*scalex+(float)destrect.Left);

 // position the cursor
 cursor->Brush->Color=clAqua;
 cursor->Left=scaledleft;
 cursor->Top=scaledtop;
 cursor->Width=scaledright-scaledleft;
 cursor->Height=scaledbottom-scaledtop;
 cursor->Visible=true;
 cursor->Enabled=true;
}



