//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include "UserDisplay.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

USERDISPLAY::USERDISPLAY()
{
 activetargets=NULL;

 form=new TForm(Application);
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

 statusbar=new STATUSBAR(form);
 cursor=new CURSOR(form);
 message=new TLabel(form);
 message->Parent=form;
 message->Visible=false;
}
//---------------------------------------------------------------------------


USERDISPLAY::~USERDISPLAY()
{
 if (message)       delete message;
 if (activetargets) delete activetargets;
 if (statusbar)     delete statusbar;
 if (cursor)        delete cursor;
 if (form)          form->Close();

 message=NULL;
 activetargets=NULL;
 statusbar=NULL;
 cursor=NULL;
}


// positions the targets on the screen
// target coordinates 0..65536
void USERDISPLAY::InitializeActiveTargetPosition()
{
TARGET  *cur_target;
int     i;
float   totalheight, numtargets, targetheight;

 totalheight=65536-StatusBarSize*655.36;                        // remaining height of the screen
 numtargets=(float)activetargets->GetNumTargets();
 // char titi[256];
 // sprintf(titi, "First %d %f", activetargets->GetNumTargets(), numtargets);
 // Application->MessageBox(titi, "", MB_OK);
 targetheight=totalheight/numtargets;
 for (i=0; i<numtargets; i++)
  {
  cur_target=activetargets->GetTargetPtr(activetargets->GetTargetID(i), i);
  if (cur_target)
     {
     cur_target->Width=(int)(TargetWidth*655.36);                  // width of target
     cur_target->Height=(int)targetheight;
     cur_target->Top=(int)(StatusBarSize*655.36+targetheight*(float)cur_target->targetposition);
     cur_target->Left=(int)(65536-cur_target->Width);
     cur_target->TextHeight=(int)(TargetTextHeight*655.36);        // height of text
     }
  }
}


// positions the status bar on the screen
// target coordinates 0..65536
void USERDISPLAY::InitializeStatusBarPosition()
{
 statusbar->Top=0;
 statusbar->Left=0;
 statusbar->Width=65535;
 statusbar->Height=(int)(StatusBarSize*655.36);
 statusbar->GoalPosX=1000;
 statusbar->GoalPosY=(int)(StatusBarSize*655.36/4);
 statusbar->ResultPosX=1000;
 statusbar->ResultPosY=(int)(StatusBarSize*655.36*3/4);
 statusbar->TextHeight=(int)(StatusBarTextHeight*655.36);        // height of text
}


// positions the cursor int the middle of the screen
// target coordinates 0..65536
void USERDISPLAY::InitializeCursorPosition()
{
 cursor->Left=0;
 cursor->Top=(int)((CursorSize*655.36+65536)/2-CursorSize*655.36/2);    // in the middle of the remaining space
 cursor->Width=(int)(CursorSize*655.36);
 cursor->Height=(int)(CursorSize*655.36);
}


// positions the position of the cursor
// target coordinates 0..65536
void USERDISPLAY::UpdateCursorPosition(short *controlsignal)
{
 // advance the cursor in pixels (the control signal defines the cursor advancement in pixels)
 // thus, re-scale the control signal (in pixels) into screen coordinates 0..65535
 cursor->Left += (int)((float)controlsignal[1]/(float)form->ClientWidth*(65535-TargetWidth*655.36));
 cursor->Top += (int)((float)controlsignal[0]/(float)form->ClientHeight*(65535-StatusBarSize*655.36));

 // higher than the top of the display area ?
 if (cursor->Top < (int)(StatusBarSize*655.36-CursorSize*655.36/2+1))
    cursor->Top=(int)(StatusBarSize*655.36-CursorSize*655.36/2+1);
 // lower than the bottom of the display area ?
 if (cursor->Top > (int)(65536-CursorSize*655.36/2-1))
    cursor->Top=(int)(65536-CursorSize*655.36/2-1);

 // farther right than the ride edge of the screen ?
 if (cursor->Left > (int)(65536-CursorSize*655.36/2))
    cursor->Left=(int)(65536-CursorSize*655.36/2);
 // farther left than the left edge of the screen ?
 if (cursor->Left < (int)(-CursorSize*655.36/2))
    cursor->Left=(int)(-CursorSize*655.36/2);
}


void USERDISPLAY::DisplayCursor()
{
 if (!cursor) return;

 cursor->RenderCursor(form, TRect(0, 0, form->ClientWidth, form->ClientHeight));
 cursor->BringToFront();
}


void USERDISPLAY::DisplayStatusBar()
{
 if (!statusbar) return;

 statusbar->RenderStatusBar(form, TRect(0, 0, form->ClientWidth, form->ClientHeight));
}


/* void  USERDISPLAY::DisplayMessage(char *message)
{
}  */


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


void USERDISPLAY::HideStatusBar()
{
 if (!statusbar) return;

 statusbar->Hide();
}


void USERDISPLAY::HideCursor()
{
 if (!cursor) return;

 cursor->Hide();
}


void USERDISPLAY::SetWindowSize(int Top, int Left, int Width, int Height, TColor Color)
{
 form->Top=Top;
 form->Left=Left;
 form->Width=Width;
 form->Height=Height;
 form->Color=Color;
}



