//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Usr.h"
#include "UParameter.h"


//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TUser *User;
//---------------------------------------------------------------------------
__fastcall TUser::TUser(TComponent* Owner)
        : TForm(Owner)
{


}
//--------------------------------------------------------------
_fastcall TUser::~TUser()
{
        User->Close();
       
}
//---------------------------------------------------------------------------

void __fastcall TUser::SetUsr( PARAMLIST *plist, STATELIST *slist )
{
        char line[512];

        strcpy(line,"UsrTask int WinXpos= 400 0 0 1 // User Window X location");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"UsrTask int WinYpos= 5 0 0 1 // User Window Y location");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"UsrTask int WinWidth= 512 0 0 1 // User Window Width");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"UsrTask int WinHeight= 512 0 0 1 // User Window Height");
        plist->AddParameter2List(line,strlen(line) );
        strcpy(line,"UsrTask int CursorSize= 25 0 0 1 // User Window Cursor Size");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int TargetType= 0 0 0 1 // 0=Targets, 1=YES/NO");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int YesNoCorrect= 0 0 0 1 // Yes or No is target word (0=Yes, 1=No)");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int YesNoOnTime= 1 0 0 10 // Time Yes/No is visible (in units of SampleBlocks)");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int YesNoOffTime= 4 0 0 10 // Time Yes/No is invisible (in units of SampleBlocks)");
        plist->AddParameter2List(line,strlen(line));
}

//------------------------------------------------------------------------

void __fastcall TUser:: Initialize(PARAMLIST *plist, STATELIST *slist, CORECOMM *corecomm)
{
       Wx=  atoi(plist->GetParamPtr("WinXpos")->GetValue());
       Wy=  atoi(plist->GetParamPtr("WinYpos")->GetValue());
       Wxl= atoi(plist->GetParamPtr("WinWidth")->GetValue());
       Wyl= atoi(plist->GetParamPtr("WinHeight")->GetValue());
       CursorSize= atoi(plist->GetParamPtr("CursorSize")->GetValue());
       Cursor->Brush->Color= clBlack;

       // do we want targets or YES/NO ?
       TargetType=  atoi(plist->GetParamPtr("TargetType")->GetValue());
       YesNoCorrect=atoi(plist->GetParamPtr("YesNoCorrect")->GetValue());
       YesNoOnTime= atoi(plist->GetParamPtr("YesNoOnTime")->GetValue());
       YesNoOffTime=atoi(plist->GetParamPtr("YesNoOffTime")->GetValue());

       // define certain things that depend on whether we have targets or YES/NO 
       if (TargetType == 0)
          {
          Target->Visible=true;
          Target2->Visible=false;
          Cursor->Shape=stRectangle;
          }
       else
          {
          Target->Visible=false;
          Target2->Visible=false;
          Cursor->Shape=stCircle;
          }

       // define target and no-target words based on whether yes or no is correct
       if (YesNoCorrect == 0)
          {
          TargetWord="YES";
          NoTargetWord="NO";
          }
       else
          {
          TargetWord="NO";
          NoTargetWord="YES";
          }

       // set size of user window
       User->ClientWidth=  Wxl;
       User->ClientHeight= Wyl;
       User->Left=         Wx;
       User->Top=          Wy;
       Cursor->Height=     CursorSize;
       Cursor->Width=      CursorSize;

       // CHECK # targets here !!! Needs to be 2 !!
       if ((TargetType == 1) && (atoi(plist->GetParamPtr("NumberTargets")->GetValue()) != 2))
          corecomm->SendStatus("417 Number of targets HAS TO BE 2 in Yes/No mode !!");

       // YES/NO text for the upper target
       TargetText1->Visible=false;
       TargetText1->Font->Height=-Wyl/10;
       TargetText1->Top=abs(Wyl/4+TargetText1->Font->Height/2);

       // YES/NO text for the lower target
       TargetText2->Visible=false;
       TargetText2->Font->Height=-Wyl/10;
       TargetText2->Top=abs(Wyl*3/4+TargetText2->Font->Height/2);

       // text for the decision (YES/NO)
       ResultText->Visible = false;
       ResultText->Font->Height=-Wyl/2;
       ResultText->Top=abs(Wyl/2+ResultText->Font->Height/2);

       tT->Font->Height=-Wyl*3/4;
       Canvas->Font=tT->Font;
       tT->Left=abs(Wxl/2-Canvas->TextWidth(tT->Caption)/2);
       tT->Top=abs(tT->Font->Height/8);

       tO->Font->Height=-Wyl*3/4;
       Canvas->Font=tO->Font;
       tO->Left=abs(Wxl/2-Canvas->TextWidth(tO->Caption)/2);
       tO->Top=abs(tO->Font->Height/8);

       HalfCursorSize= CursorSize / 2;

       limit_top= 0; //CursorSize/2;
       limit_bottom= Wyl; //  - CursorSize/2;
       limit_left= HalfCursorSize;  //  0; CursorSize/2;
       limit_right= Wxl - HalfCursorSize; // - CursorSize/2;

       User->Show();
}
//----------------------------------------------------------------------------
//
//factors (x and y) to transform to normalized areas
//  ( 0x7fff ) y to bottom-top and  x to right-left
//
void TUser::Scale( float x, float y )
{
        if( x != 0.0 ) scale_x= (limit_right-limit_left) / x;
        else scale_x= 1.0;
        if( y != 0.0 ) scale_y= (limit_bottom-limit_top) / y;
        else scale_y= 1.0;
}

//-----------------------------------------------------------------------------
void TUser::GetLimits(float *right, float *left, float *top, float *bottom )
{
        *(right) = limit_right;
        *(left)  = limit_left;
        *(top)   = limit_top;
        *(bottom)= limit_bottom;
}

//----------------------------------------------------------------------------
void TUser::PutCursor(float x, float y, int cursorstate )
{
TColor color;

        x*= scale_x;
        y*= scale_y;

        if( y <= limit_top )    y= limit_top;
        if( y >= limit_bottom ) y= limit_bottom;
        if( x <= limit_left )   x= limit_left;
        if( x >= limit_right )  x= limit_right;

        Cursor->Top=  y - HalfCursorSize;
        Cursor->Left= x - HalfCursorSize;

        if (cursorstate == CURSOR_ON)      color=clRed;
        if (cursorstate == CURSOR_OFF)     color=clBlack;
        if (cursorstate == CURSOR_RESULT)
           {
           if (TargetType == 0)            // switch the color to yellow for targets
              color=clYellow;
           if (TargetType == 1)            // leave them red for YES/NO
              color=clBlack;
           }

        Cursor->Brush->Color= color;
        // turn make the cursor visible, except if we are
        // in the reward period and have YES/NO targets
        if ((cursorstate == CURSOR_RESULT) && (TargetType == 1))
           Cursor->Visible=false;
        else
           Cursor->Visible=true;
}


// show this feedback before a run starts
void TUser::PreRunInterval(int time)
{
 if (time == 0)
    {
    Clear();
    if (TargetType == 0)
       {
       tPreRunIntervalText->Font->Height=-Wyl*1/6;
       tPreRunIntervalText->Caption="Get Ready ...";
       }
    if (TargetType == 1)
       {
       tPreRunIntervalText->Font->Height=-Wyl*2/4;
       tPreRunIntervalText->Caption=TargetWord.UpperCase();
       }
    Canvas->Font=tPreRunIntervalText->Font;
    tPreRunIntervalText->Left=abs(Wxl/2-Canvas->TextWidth(tPreRunIntervalText->Caption)/2);
    tPreRunIntervalText->Top=abs(Wyl/2+tPreRunIntervalText->Font->Height/2);
    tPreRunIntervalText->Visible=true;
    }
}


//----------------------------------------------------------------------------
void TUser::Outcome(int time, int result)
{
 // do not do anything in this period if we have targets
 if (TargetType == 0) return;

 // if we have a YES/NO display, flash the selected word
 if (time == 0)
    {
    // turn off targets
    Target->Visible=false;
    Target2->Visible=false;
    // turn off target words
    TargetText1->Visible = false;
    TargetText2->Visible = false;
    // turn on selected word
    Canvas->Font=ResultText->Font;
    if (result == 1)
       ResultText->Caption=TargetText1->Caption;
    else
       ResultText->Caption=TargetText2->Caption;
    ResultText->Left=abs(Wxl/2-Canvas->TextWidth(ResultText->Caption)/2);
    }

 // turn the ResultText on or off
 if (time % (YesNoOnTime+YesNoOffTime) == 0) ResultText->Visible = true;
 if (time % (YesNoOnTime+YesNoOffTime) == YesNoOnTime) ResultText->Visible = false;
}


void TUser::PutT(bool Tstate)
{
 if (Tstate)
    {
    Clear();
    tT->Visible=true;
    tO->Visible=false;
    }
 else
    tT->Visible=false;
}

void TUser::PutO( bool Tstate )
{
        if(Tstate)
        {
                Clear();
                tO->Visible=true;
        }
        else
                tO->Visible=false;
}

//-----------------------------------------
void TUser::PutTarget(int targetnumber, int targetstate )
{
TColor  color;

 // regular targets ?
 if (TargetType == 0)
    {
    Target->Top=  targy[targetnumber];
    Target->Left= targx[targetnumber];
    Target->Height= targsizey[targetnumber];
    Target->Width = targsizex[targetnumber];
    if (targetstate == TARGET_RESULT) color=clYellow;
    if (targetstate == TARGET_ON)     color=clRed;
    if (targetstate == TARGET_OFF)    color=clBlack;
    Target->Brush->Color= color;
    Target->Visible=true;
    }

 // YES/NO targets
 if (TargetType == 1)
    {
    // turn text on when target comes on (turn it off when display cleared)
    if (targetstate == TARGET_ON)
       {
       // turn on both targets
       Target->Top=  targy[1];
       Target->Left= targx[1];
       Target->Height= targsizey[1];
       Target->Width = targsizex[1];
       Target->Brush->Color= clGray;
       Target2->Top=  targy[2];
       Target2->Left= targx[2];
       Target2->Height= targsizey[2];
       Target2->Width = targsizex[2];
       Target2->Brush->Color= clGray;
       Target->Visible=true;
       Target2->Visible=true;
       // define YES/NO texts for both targets
       if (targetnumber == 1)
          {
          TargetText1->Caption=TargetWord;
          TargetText2->Caption=NoTargetWord;
          }
       else
          {
          TargetText1->Caption=NoTargetWord;
          TargetText2->Caption=TargetWord;
          }
       Canvas->Font=TargetText1->Font;
       TargetText1->Left=targx[1]+targsizex[1]/2-Canvas->TextWidth(TargetText1->Caption)/2;
       TargetText2->Left=targx[2]+targsizex[2]/2-Canvas->TextWidth(TargetText2->Caption)/2;
       TargetText1->Visible = true;
       TargetText2->Visible = true;
       }
    }
}

//-----------------------------------------------------------------------
void TUser::Clear( void )
{
        Target->Visible=false;
        Target2->Visible=false;
        Cursor->Visible=false;
        TargetText1->Visible= false;
        TargetText2->Visible= false;
        ResultText->Visible= false;
        tPreRunIntervalText->Visible=false;
        // Refresh();
}

//--------------------------------------------------------------------
// Random # generator ran1() from Press et. al.
//   - Builder Random # function not good!

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

float TUser::ran1( long *idum )
{
        int j;
        long k;
        static long iy= 0;
        static long iv[NTAB];
        float temp;

        if(*idum <= 0 || !iy )
        {
                if(-(*idum) <1) *idum=1;
                else *idum= -(*idum);
                for(j=NTAB+7;j>=0;j--)
                {
                        k= (*idum)/IQ;
                        *idum= IA*(*idum-k*IQ)-IR*k;
                        if(*idum<0) *idum += IM;
                        if( j < NTAB ) iv[j]= *idum;
                 }
                 iy= iv[0];
         }
         k= (*idum)/IQ;
         *idum= IA*(*idum-k*IQ)-IR*k;
         if( *idum < 0 ) *idum += IM;
         j= iy/NDIV;
         iy= iv[j];
         iv[j]= *idum;
         if((temp=AM*iy) > RNMX) return RNMX;
         else return temp;
}


