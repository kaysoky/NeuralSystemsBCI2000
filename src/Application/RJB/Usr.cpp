#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------


#include "Usr.h"
#include "UParameter.h"
#include "Localization.h"
#include "UBCIError.h"
#include "MeasurementUnits.h"

#include <math.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
Usr::Usr()
: mpForm( NULL ),
  mRotateBy( 0.0 )
{
 BEGIN_PARAMETER_DEFINITIONS
  "UsrTask int WinXpos= 400 0 0 2000 "
    "// User Window X location",
  "UsrTask int WinYpos= 5 0 0 2000 "
    "// User Window Y location",
  "UsrTask int WinWidth= 512 0 0 2000 "
    "// User Window Width",
  "UsrTask int WinHeight= 512 0 0 2000 "
    "// User Window Height",
  "UsrTask int CursorSize= 25 0 0 1000 "
    "// User Window Cursor Size",
  "UsrTask float RotateBy= 0 0 0 0 "
    "// Counterclockwise rotation of feedback view in units of 2 Pi",
  "UsrTask int TargetType= 0 0 0 1 "
    "// 0=Targets, 1=YES/NO (enumeration)",
  "UsrTask int YesNoCorrect= 0 0 0 1 "
    "// Yes or No is target word (0=Yes, 1=No) (enumeration)",
  "UsrTask int YesNoOnTime= 1 0 0 0 "
    "// Time Yes/No is visible (in units of SampleBlocks)",
  "UsrTask int YesNoOffTime= 4 0 0 0 "
    "// Time Yes/No is invisible (in units of SampleBlocks)",
 END_PARAMETER_DEFINITIONS
}
//--------------------------------------------------------------
Usr::~Usr()
{
  delete mpForm;
}
//---------------------------------------------------------------------------

void Usr::Initialize()
{
  if( mpForm == NULL )
    Application->CreateForm( __classid( TUser ), &mpForm );

  ApplyLocalizations( mpForm );

  Rotate( mpForm->TargetText1, -mRotateBy );
  Rotate( mpForm->TargetText2, -mRotateBy );
  Rotate( mpForm->ResultText, -mRotateBy );

  Wx=  Parameter("WinXpos");
  Wy=  Parameter("WinYpos");
  Wxl= Parameter("WinWidth");
  Wyl= Parameter("WinHeight");
  CursorSize= Parameter("CursorSize");
  mpForm->Cursor->Brush->Color= clBlack;

  mRotateBy = Parameter( "RotateBy" );

  // do we want targets or YES/NO ?
  TargetType   = Parameter( "TargetType" );
  YesNoCorrect = Parameter( "YesNoCorrect" );
  YesNoOnTime =  MeasurementUnits::ReadAsTime( Parameter( "YesNoOnTime" ) );
  YesNoOffTime = MeasurementUnits::ReadAsTime( Parameter( "YesNoOffTime" ) );


  // define certain things that depend on whether we have targets or YES/NO
  if (TargetType == 0)
    {
    mpForm->Target->Visible=false;
    mpForm->Target2->Visible=false;
    mpForm->Cursor->Shape=stRectangle;
    }
  else
    {
    mpForm->Target->Visible=false;
    mpForm->Target2->Visible=false;
    mpForm->Cursor->Shape=stCircle;
    }

  // define target and no-target words based on whether yes or no is correct
  if (YesNoCorrect == 0)
    {
    TargetWord= LocalizableString( "YES" );
    NoTargetWord= LocalizableString( "NO" );
    }
  else
    {
    TargetWord= LocalizableString( "NO" );
    NoTargetWord= LocalizableString( "YES" );
    }

  // set size of user window
  mpForm->ClientWidth=  Wxl;
  mpForm->ClientHeight= Wyl;
  mpForm->Left=         Wx;
  mpForm->Top=          Wy;
  mpForm->Cursor->Height=     CursorSize;
  mpForm->Cursor->Width=      CursorSize;

  // CHECK # targets here !!! Needs to be 2 !!
  if ((TargetType == 1) && (Parameter("NumberTargets") != 2))
    bcierr << "Number of targets HAS TO BE 2 in Yes/No mode !!" << std::endl;

  // YES/NO text for the upper target
  mpForm->TargetText1->Visible=false;
  mpForm->TargetText1->Font->Height=-Wyl/10;
  mpForm->TargetText1->Top=abs(Wyl/4+mpForm->TargetText1->Font->Height/2);
  Rotate( mpForm->TargetText1, mRotateBy );

  // YES/NO text for the lower target
  mpForm->TargetText2->Visible=false;
  mpForm->TargetText2->Font->Height=-Wyl/10;
  mpForm->TargetText2->Top=abs(Wyl*3/4+mpForm->TargetText2->Font->Height/2);
  Rotate( mpForm->TargetText2, mRotateBy );

  // text for the decision (YES/NO)
  mpForm->ResultText->Visible = false;
  mpForm->ResultText->Font->Height=-Wyl/2;
  mpForm->ResultText->Top=abs(Wyl/2+mpForm->ResultText->Font->Height/2);
  Rotate( mpForm->ResultText, mRotateBy );

  mpForm->tT->Font->Height=-Wyl*3/4;
  mpForm->Canvas->Font=mpForm->tT->Font;
  mpForm->tT->Left=abs(Wxl/2-mpForm->Canvas->TextWidth(mpForm->tT->Caption)/2);
  mpForm->tT->Top=abs(mpForm->tT->Font->Height/8);

  mpForm->tO->Font->Height=-Wyl*3/4;
  mpForm->Canvas->Font=mpForm->tO->Font;
  mpForm->tO->Left=abs(Wxl/2-mpForm->Canvas->TextWidth(mpForm->tO->Caption)/2);
  mpForm->tO->Top=abs(mpForm->tO->Font->Height/8);

  HalfCursorSize= CursorSize / 2;

  limit_top= 0; //CursorSize/2;
  limit_bottom= Wyl; //  - CursorSize/2;
  limit_left= HalfCursorSize;  //  0; CursorSize/2;
  limit_right= Wxl - HalfCursorSize; // - CursorSize/2;

  mpForm->Show();
}
//----------------------------------------------------------------------------
//
//factors (x and y) to transform to normalized areas
//  ( 0x7fff ) y to bottom-top and  x to right-left
//
void Usr::Scale( float x, float y )
{
        if( x != 0.0 ) scale_x= (limit_right-limit_left) / x;
        else scale_x= 1.0;
        if( y != 0.0 ) scale_y= (limit_bottom-limit_top) / y;
        else scale_y= 1.0;
}

//-----------------------------------------------------------------------------
void Usr::GetLimits(float *right, float *left, float *top, float *bottom )
{
        *(right) = limit_right;
        *(left)  = limit_left;
        *(top)   = limit_top;
        *(bottom)= limit_bottom;
}

//----------------------------------------------------------------------------
void Usr::PutCursor(float x, float y, int cursorstate )
{
TColor color;

        x*= scale_x;
        y*= scale_y;

        if( y <= limit_top )    y= limit_top;
        if( y >= limit_bottom ) y= limit_bottom;
        if( x <= limit_left )   x= limit_left;
        if( x >= limit_right )  x= limit_right;

        Rotate( mpForm->Cursor, -mRotateBy );
        mpForm->Cursor->Top=  y - HalfCursorSize;
        mpForm->Cursor->Left= x - HalfCursorSize;
        Rotate( mpForm->Cursor, mRotateBy );

        if (cursorstate == CURSOR_ON)      color=clRed;
        if (cursorstate == CURSOR_OFF)     color=clBlack;
        if (cursorstate == CURSOR_RESULT)
           {
           if (TargetType == 0)            // switch the color to yellow for targets
              color=clYellow;
           if (TargetType == 1)            // leave them red for YES/NO
              color=clBlack;
           }

        mpForm->Cursor->Brush->Color= color;
        // turn make the cursor visible, except if we are
        // in the reward period and have YES/NO targets
        if ((cursorstate == CURSOR_RESULT) && (TargetType == 1))
           mpForm->Cursor->Visible=false;
        else
           mpForm->Cursor->Visible=true;
}


// show this feedback before a run starts
void Usr::PreRunInterval(int time)
{
 if (time == 0)
    {
    Clear();
    if (TargetType == 0)
       {
       mpForm->tPreRunIntervalText->Font->Height=-Wyl*1/6;
       mpForm->tPreRunIntervalText->Caption= LocalizableString( "Get Ready ..." );
       }
    if (TargetType == 1)
       {
       mpForm->tPreRunIntervalText->Font->Height=-Wyl*2/4;
       mpForm->tPreRunIntervalText->Caption=TargetWord.UpperCase();
       }
    mpForm->Canvas->Font=mpForm->tPreRunIntervalText->Font;
    mpForm->tPreRunIntervalText->Left=
      abs(Wxl/2-mpForm->Canvas->TextWidth(mpForm->tPreRunIntervalText->Caption)/2);
    mpForm->tPreRunIntervalText->Top=
      abs(Wyl/2+mpForm->tPreRunIntervalText->Font->Height/2);
    mpForm->tPreRunIntervalText->Visible=true;
    }
}


//----------------------------------------------------------------------------
void Usr::Outcome(int time, int result)
{
 // do not do anything in this period if we have targets
 if (TargetType == 0) return;

 // if we have a YES/NO display, flash the selected word
 if (time == 0)
    {
    // turn off targets
    mpForm->Target->Visible=false;
    mpForm->Target2->Visible=false;
    // turn off target words
    mpForm->TargetText1->Visible = false;
    mpForm->TargetText2->Visible = false;
    // turn on selected word
    mpForm->Canvas->Font=mpForm->ResultText->Font;
    if (result == 1)
       mpForm->ResultText->Caption=mpForm->TargetText1->Caption;
    else
       mpForm->ResultText->Caption=mpForm->TargetText2->Caption;
    Rotate( mpForm->ResultText, -mRotateBy );
    mpForm->ResultText->Left
      =abs(Wxl/2-mpForm->Canvas->TextWidth(mpForm->ResultText->Caption)/2);
    Rotate( mpForm->ResultText, mRotateBy );
    }

 // turn the mpForm->ResultText on or off
 if (time % (YesNoOnTime+YesNoOffTime) == 0) mpForm->ResultText->Visible = true;
 if (time % (YesNoOnTime+YesNoOffTime) == YesNoOnTime) mpForm->ResultText->Visible = false;
}


void Usr::PutT(bool Tstate)
{
 if (Tstate)
    {
    Clear();
    mpForm->tT->Visible=true;
    mpForm->tO->Visible=false;
    }
 else
    mpForm->tT->Visible=false;
}

void Usr::PutO( bool Tstate )
{
        if(Tstate)
        {
                Clear();
                mpForm->tO->Visible=true;
        }
        else
                mpForm->tO->Visible=false;
}

//-----------------------------------------
void Usr::PutTarget(int targetnumber, int targetstate )
{
TColor  color;

 // regular targets ?
 if (TargetType == 0)
    {
    mpForm->Target->Top=  targy[targetnumber];
    mpForm->Target->Left= targx[targetnumber];
    mpForm->Target->Height= targsizey[targetnumber];
    mpForm->Target->Width = targsizex[targetnumber];
    Rotate( mpForm->Target, mRotateBy );
    if (targetstate == TARGET_RESULT) color=clYellow;
    if (targetstate == TARGET_ON)     color=clRed;
    if (targetstate == TARGET_OFF)    color=clBlack;
    mpForm->Target->Brush->Color= color;
    mpForm->Target->Visible=true;
    }

 // YES/NO targets
 if (TargetType == 1)
    {
    // turn text on when target comes on (turn it off when display cleared)
    if (targetstate == TARGET_ON)
       {
       // turn on both targets
       mpForm->Target->Top=  targy[1];
       mpForm->Target->Left= targx[1];
       mpForm->Target->Height= targsizey[1];
       mpForm->Target->Width = targsizex[1];
       mpForm->Target->Brush->Color= clGray;
       Rotate( mpForm->Target, mRotateBy );

       mpForm->Target2->Top=  targy[2];
       mpForm->Target2->Left= targx[2];
       mpForm->Target2->Height= targsizey[2];
       mpForm->Target2->Width = targsizex[2];
       mpForm->Target2->Brush->Color= clGray;
       Rotate( mpForm->Target2, mRotateBy );

       mpForm->Target->Visible=true;
       mpForm->Target2->Visible=true;
       // define YES/NO texts for both targets
       if (targetnumber == 1)
          {
          mpForm->TargetText1->Caption=TargetWord;
          mpForm->TargetText2->Caption=NoTargetWord;
          }
       else
          {
          mpForm->TargetText1->Caption=NoTargetWord;
          mpForm->TargetText2->Caption=TargetWord;
          }
       mpForm->Canvas->Font=mpForm->TargetText1->Font;
       Rotate( mpForm->TargetText1, -mRotateBy );
       mpForm->TargetText1->Left
         =targx[1]+targsizex[1]/2-mpForm->Canvas->TextWidth(mpForm->TargetText1->Caption)/2;
       Rotate( mpForm->TargetText1, mRotateBy );
       Rotate( mpForm->TargetText2, -mRotateBy );
       mpForm->TargetText2->Left
         =targx[2]+targsizex[2]/2-mpForm->Canvas->TextWidth(mpForm->TargetText2->Caption)/2;
       Rotate( mpForm->TargetText2, mRotateBy );
       mpForm->TargetText1->Visible = true;
       mpForm->TargetText2->Visible = true;
       }
    }
}

//-----------------------------------------------------------------------
void Usr::Clear( void )
{
        mpForm->Target->Visible=false;
        mpForm->Target2->Visible=false;
        mpForm->Cursor->Visible=false;
        mpForm->TargetText1->Visible= false;
        mpForm->TargetText2->Visible= false;
        mpForm->ResultText->Visible= false;
        mpForm->tPreRunIntervalText->Visible=false;
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

float Usr::ran1( long *idum )
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

void
Usr::Rotate( TControl* ioControl, float inAngle )
{
  if( inAngle == 0 )
    return;
    
  float x = ioControl->Left,
        y = ioControl->Top,
        width = ioControl->Width,
        height = ioControl->Height,
        xunit = mpForm->ClientWidth,
        yunit = mpForm->ClientHeight;
  x -= xunit / 2;
  y -= yunit / 2;
  float m_xx = ::cos( inAngle * 2 * M_PI ),
        m_xy = ::sin( inAngle * 2 * M_PI ),
        m_yy = m_xx,
        m_yx = -m_xy;
  m_xy *= xunit / yunit;
  m_yx *= yunit / xunit;
  float x2 = m_xx * x + m_xy * y,
        y2 = m_yx * x + m_yy * y,
        width2 = m_xx * width + m_xy * height,
        height2 = m_yx * width + m_yy * height;
  x2 += xunit / 2;
  y2 += yunit / 2;
  if( width2 < 0 )
  {
    width2 = ::fabs( width2 );
    x2 -= width2;
  }
  if( height2 < 0 )
  {
    height2 = ::fabs( height2 );
    y2 -= height2;
  }
  ioControl->Left = ::floor( x2 + 0.5 );
  ioControl->Top = ::floor( y2 + 0.5 );
  ioControl->Width = ::floor( width2 + 0.5 );
  ioControl->Height = ::floor( height2 + 0.5 );
}



