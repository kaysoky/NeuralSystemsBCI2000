#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------


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

 message=new TLabel(form);
 message->Parent=form;
 message->Visible=false;

 //VK
 textform=NULL;
}
//---------------------------------------------------------------------------


USERDISPLAY::~USERDISPLAY()
{
 if (message)       delete message;
 if (activetargets) delete activetargets;
 if (statusbar)     delete statusbar;
 if (form)          form->Close();

 message=NULL;
 activetargets=NULL;
 statusbar=NULL;
  // VK Text Window stuff
 if (textwindow)    delete textwindow;
 if (textform)      textform->Close();
 textwindow=NULL;
}


// positions the status bar on the screen
// target coordinates 0..65536
void USERDISPLAY::InitializeStatusBarPosition(bool displayresulttext)
{
 statusbar->Color=clBlack;
 statusbar->StatusBarColor=clLtGray;
 statusbar->Top=0;
 statusbar->Left=0;
 statusbar->Width=65535;
 statusbar->Height=(int)(StatusBarSize*655.36);
 statusbar->GoalPosX=1000;
 statusbar->GoalPosY=(int)(StatusBarSize*655.36/4);
 statusbar->ResultPosX=1000;
 statusbar->ResultPosY=(int)(StatusBarSize*655.36*3/4);
 statusbar->TextHeight=(int)(StatusBarTextHeight*655.36);        // height of text
 statusbar->SetResultTextVisibility(displayresulttext);
}


void USERDISPLAY::DisplayStatusBar()
{
 if (!statusbar) return;

 statusbar->RenderStatusBar(form, TRect(0, 0, form->ClientWidth, form->ClientHeight));
}


void USERDISPLAY::HideStatusBar()
{
 if (!statusbar) return;

 statusbar->Hide();
}

// positions the targets on the screen
// target coordinates 0..65536
void USERDISPLAY::InitializeActiveTargetPosition()
{
TARGET  *cur_target;
int     i, numtargetsx, numtargetsy;
int     matrixwidth, matrixheight, targetspacingx, targetspacingy;
int     numtargets, totalheight;

/*shidong starts
FILE *f;
f = fopen("debug.txt", "w");
/*shidong ends*/
 totalheight=65536-StatusBarSize*655.36;                        // remaining height of the screen
 numtargets=activetargets->GetNumTargets();


/*shidong starts*/
 numtargetsx    =       displayCol;
 numtargetsy    =       displayRow;         //(numtargets-1)/numtargetsx+1;
/*shidong ends*/

 targetspacingx=1000;
 targetspacingy=1000;
     /*shidong starts
     fprintf(f, "numtargetsx is %d.\n", numtargetsx);
     fprintf(f, "numtargetsy is %d.\n", numtargetsy);
     fprintf(f, "numtargets is %d.\n", numtargets);
     /*shidong ends*/ 
 matrixwidth=numtargetsx*(int)(TargetWidth*655.36)+(numtargetsx-1)*targetspacingx;
 matrixheight=numtargetsy*(int)(TargetHeight*655.36)+(numtargetsy-1)*targetspacingy;

 for (i=0; i<numtargets; i++)
  {
  cur_target=activetargets->GetTargetPtr(i+1);
  if (cur_target)
     {
     cur_target->Width=(int)(TargetWidth*655.36 );                  // width of target
     cur_target->Height=(int)(TargetHeight*655.36 );
     cur_target->Top=StatusBarSize*655.36+totalheight/2-matrixheight/2+(i/numtargetsx)*(cur_target->Height+targetspacingy);
     cur_target->Left=65536/2-matrixwidth/2+(i%numtargetsx)*(cur_target->Width+targetspacingx);
     /*shidong starts*/
     cur_target->TextHeight=(int)(TargetTextHeight*655.36*((float)(cur_target->FontSizeFactor)));        // height of text
     /* fprintf(f, "CurTarget ID is %d, caption is %s, displayResult is %s, fontfactor is %f.\n", cur_target->targetID, cur_target->Caption, cur_target->CharDisplayInResult, ((float)(cur_target->FontSizeFactor)));
     shidong ends*/
     }
  }
}


void USERDISPLAY::HideMessage()
{
 message->Visible=false;
}


void USERDISPLAY::DisplayMessage(const char *new_message)
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
/* VK Adding following methods for text window capability */

void USERDISPLAY::SetTextWindowSize(int Top, int Left, int Width, int Height)
{
 textform->Top=Top;
 textform->Left=Left;
 textform->Width=Width;
 textform->Height=Height;
}

void USERDISPLAY::DisplayTextWindow(int FontSize, const char *FontName)
{
  if (!textform)
  {
    textform =  new TForm(Application);
    textform->Caption="Text Window";
    textform->BorderIcons >> biSystemMenu;
    textform->BorderStyle=bsNone;

    textwindow = new TMemo(textform);
    textwindow->Parent = textform;
    textwindow->Color = clWhite;
    textwindow->ScrollBars = ssVertical;
    textwindow->Align = alClient;
    textwindow->Visible = true;
  }
  textwindow->Font->Name = FontName;
  textwindow->Font->Size = FontSize;

}

void USERDISPLAY::DisableTextWindow()
{
  if (textform)
    textform->Visible = false;
}



