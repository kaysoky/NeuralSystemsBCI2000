/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include "PCHIncludes.h"
#pragma hdrstop

#include <stdio.h>
#include "UserDisplay.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

USERDISPLAY::USERDISPLAY()
{
 activetargets=NULL;

 form=new TForm(static_cast<TComponent*>(NULL));
 form->Top=50;
 form->Left=50;
 form->Width=300;
 form->Height=300;
 form->Caption="Speller";
 form->Color=clDkGray;

 // disable automatic scrollbars
 form->AutoScroll=false;
 form->BorderIcons >> biSystemMenu;
 form->BorderStyle=bsNone;

 message=new TLabel(static_cast<TComponent*>(NULL));
 message->Parent=form;
 message->Visible=false;
}
//---------------------------------------------------------------------------


USERDISPLAY::~USERDISPLAY()
{
 if (message)       delete message;
 if (activetargets) delete activetargets;
 delete form;

 message=NULL;
 activetargets=NULL;
}


// positions the targets on the screen
// target coordinates 0..65536
void USERDISPLAY::InitializeActiveTargetPosition()
{
TARGET  *cur_target;
int     i;
float   numtargets;

 numtargets=(float)activetargets->GetNumTargets();
 // char titi[256];
 // sprintf(titi, "First %d %f", activetargets->GetNumTargets(), numtargets);
 // Application->MessageBox(titi, "", MB_OK);
 // targetheight=totalheight/numtargets;
 for (i=0; i<numtargets; i++)
  {
  cur_target=activetargets->GetTargetPtr(activetargets->GetTargetID(i), i);
  if (cur_target)
     {
     cur_target->Width=(int)(TargetWidth*655.36);                  // width of target
     cur_target->Height=(int)(TargetHeight*655.36);
     cur_target->Top=65536/2-cur_target->Width/2;
     cur_target->Left=65536/2-cur_target->Height/2;
     cur_target->TextHeight=(int)(TargetTextHeight*655.36);        // height of text
     }
  }
}


void USERDISPLAY::HideMessage()
{
 message->Visible=false;
}


void USERDISPLAY::DisplayMessage(char *new_message)
{
int     destwidth, destheight, i;
int     scaledtextsize, scaledtextposx, scaledtextposy;
float   scalex, scaley;

 destwidth=form->ClientWidth;
 destheight=form->ClientHeight;
 scalex=(float)destwidth/(float)65536;
 scaley=(float)destheight/(float)65536;

 scaledtextsize=(int)((float)8*655.36*scaley);
 form->Canvas->Font->Height=-scaledtextsize;
 form->Canvas->Font->Name="Arial";

  // write the congratulations text
 message->Font->Color=clYellow;
 message->Font->Height=-scaledtextsize;
 message->Font->Name="Arial";
 message->Caption=AnsiString(new_message);
 scaledtextposx=(int)((float)32767*scalex-form->Canvas->TextWidth(message->Caption)/2);
 scaledtextposy=(int)((float)32767*scaley-message->Height/2);
 message->Visible=true;
 message->Layout=tlBottom;
 message->Transparent=true;
 message->Left=scaledtextposx;
 message->Top=scaledtextposy;
}


void USERDISPLAY::DisplayActiveTargets()
{
 if (!activetargets) return;

 activetargets->RenderTargets(form, TRect(0, 0, form->ClientWidth, form->ClientHeight));
}


void USERDISPLAY::HideActiveTargets()
{
 if (!activetargets) return;

 activetargets->HideTargets();
}


void USERDISPLAY::SetWindowSize(int Top, int Left, int Width, int Height, TColor Color)
{
 form->Top=Top;
 form->Left=Left;
 form->Width=Width;
 form->Height=Height;
 form->Color=Color;
}





