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

using namespace std;

//---------------------------------------------------------------------------
Usr::Usr()
: limit_right( 0 ),
  limit_left( 0 ),
  limit_top( 0 ),
  limit_bottom( 0 ),
  mpForm( NULL ),
  mTargetWidth( 0.0 ),
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
  "UsrTask float TargetWidth= 25 0 0 32767 "
    "// Width of Targets",
  "UsrTask float RotateBy= 0 0 0 0 "
    "// Counterclockwise rotation of feedback view in units of 2 Pi",
  "UsrTask int TargetType= 0 0 0 2 "
    "// 0=Targets, 1=YES/NO, 2=Free Choice (enumeration)",
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
  DeleteChoiceTargets();
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
  mTargetWidth = Parameter( "TargetWidth" );
  mRotateBy = Parameter( "RotateBy" );

  // do we want targets or YES/NO ?
  TargetType   = Parameter( "TargetType" );
  YesNoCorrect = Parameter( "YesNoCorrect" );
  YesNoOnTime =  MeasurementUnits::ReadAsTime( Parameter( "YesNoOnTime" ) );
  YesNoOffTime = MeasurementUnits::ReadAsTime( Parameter( "YesNoOffTime" ) );

  TColor choiceColors[] =
  {
    clLime,
    clRed,
    clBlue,
    clFuchsia
  };
  const maxNumTargets = sizeof( choiceColors ) / sizeof( *choiceColors );
  int numChoiceTargets = 0;
  if( TargetType == TYPE_CHOICE )
    numChoiceTargets = Parameter( "NumberTargets" );
  if( numChoiceTargets > maxNumTargets )
  {
    bcierr << "not ready for > "
           << maxNumTargets
           << " target choices."
           << endl;
    numChoiceTargets = 1;
  }

  DeleteChoiceTargets();
  for( int i = 0; i < numChoiceTargets; ++i )
  {
    TShape* target = new TShape( NULL );
    target->Parent = mpForm;
    target->Pen->Assign( mpForm->Target->Pen );
    target->SendToBack();
    target->Top = targy[ i + 1 ];
    target->Left = targx[ i + 1 ];
    target->Height = targsizey[ i + 1 ];
    target->Width = targsizex[ i + 1 ];
    Rotate( target, mRotateBy );
    target->Brush->Color = choiceColors[ i ];
    target->Visible = false;
    mChoiceTargets.push_back( target );
  }

  // define certain things that depend on whether we have targets or YES/NO
  if (TargetType == TYPE_TARGET)
    {
   mpForm->Cursor->Shape=stRectangle;
    }
  else
    {
    mpForm->Cursor->Shape=stCircle;
    }
  mpForm->Target->Hide();
  mpForm->Target2->Hide();

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
  if ((TargetType == TYPE_YESNO) && (Parameter("NumberTargets") != 2))
    bcierr << "Number of targets HAS TO BE 2 in Yes/No mode !!" << endl;

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

  mpForm->tT->AutoSize = true;
  mpForm->tT->Width = mpForm->ClientWidth;
  mpForm->tT->Font->Height=-Wyl*3/4;
  mpForm->Canvas->Font=mpForm->tT->Font;
  while( mpForm->Canvas->TextWidth( mpForm->tT->Caption ) > mpForm->ClientWidth )
    mpForm->Canvas->Font->Height = ( mpForm->Canvas->Font->Height * 9 ) / 10;
  mpForm->tT->Font = mpForm->Canvas->Font;
  mpForm->tT->Left = ( mpForm->ClientWidth - mpForm->Canvas->TextWidth( mpForm->tT->Caption ) ) / 2;
  mpForm->tT->Top = ( mpForm->ClientHeight - mpForm->Canvas->TextHeight( mpForm->tT->Caption ) ) / 2;

  mpForm->tO->Font->Height=-Wyl*3/4;
  mpForm->Canvas->Font=mpForm->tO->Font;
  mpForm->tO->Left=abs(Wxl/2-mpForm->Canvas->TextWidth(mpForm->tO->Caption)/2);
  mpForm->tO->Top=abs(mpForm->tO->Font->Height/8);

  HalfCursorSize= CursorSize / 2;

  limit_top= 0; //CursorSize/2;
  limit_bottom= Wyl; //  - CursorSize/2;
  limit_left= HalfCursorSize;  //  0; CursorSize/2;
  limit_right= Wxl - HalfCursorSize; // - CursorSize/2;

  Scale( (float)0x7fff, (float)0x7fff );
  ComputeTargets();

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

    x *= scale_x;
    y *= scale_y;

    if( y <= limit_top )    y= limit_top;
    if( y >= limit_bottom ) y= limit_bottom;
    if( x <= limit_left )   x= limit_left;
    if( x >= limit_right )  x= limit_right;

    Rotate( mpForm->Cursor, -mRotateBy );
    mpForm->Cursor->Top=  y - HalfCursorSize;
    mpForm->Cursor->Left= x - HalfCursorSize;
    Rotate( mpForm->Cursor, mRotateBy );

    switch( cursorstate )
    {
      case CURSOR_ON:
        switch( TargetType )
        {
          case TYPE_CHOICE:
              color = clYellow;
              break;
            case TYPE_TARGET:
            case TYPE_YESNO:
            default:
            color = clRed;
        }
        break;
        
      case CURSOR_RESULT:
        switch( TargetType )
        {
          case TYPE_TARGET:
          case TYPE_CHOICE:
            color = clYellow;
            break;
          case TYPE_YESNO:
          default:
            color = clBlack;
            break;
        }
        break;
        
       case CURSOR_OFF:
       default:
        color = clBlack;
        break;
      }
        
      mpForm->Cursor->Brush->Color= color;
      // turn make the cursor visible, except if we are
      // in the reward period and have YES/NO targets
      if ((cursorstate == CURSOR_RESULT) && (TargetType == TYPE_YESNO))
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
     if (TargetType == TYPE_YESNO)
       {
       mpForm->tPreRunIntervalText->Font->Height=-Wyl*2/4;
       mpForm->tPreRunIntervalText->Caption=TargetWord.UpperCase();
       }
       else
       {
       mpForm->tPreRunIntervalText->Font->Height=-Wyl*1/6;
       mpForm->tPreRunIntervalText->Caption= LocalizableString( "Get Ready ..." );
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
 if (TargetType != TYPE_YESNO) return;

 // if we have a YES/NO display, flash the selected word
 if (time == 0)
    {
    // turn off targets
    mpForm->Target->Hide();
    mpForm->Target2->Hide();
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
  TColor  color = clBlack;
  bool    visible = false;
  switch( TargetType )

  {

    case TYPE_TARGET:

      visible = true;

      switch( targetstate )

      {
        case TARGET_RESULT:
          color = clYellow;
          break;

        case TARGET_ON:
          color = clRed;
          break;

        case TARGET_OFF:
        default:
          color = clBlack;
          break;
      }
      break;

    case TYPE_CHOICE:
      switch( targetstate )
      {
        case TARGET_RESULT:
          visible = true;
          color = clBlack;
          break;

        case TARGET_ON:
          visible = false;
          break;

        case TARGET_OFF:
        default:
          visible = false;
          break;
      }
      break;

    default:
      ;
  }


  // regular targets ?
  switch( TargetType )
  {
    case TYPE_TARGET:
    case TYPE_CHOICE:
      mpForm->Target->Top = targy[targetnumber];
      mpForm->Target->Left = targx[targetnumber];
      mpForm->Target->Height = targsizey[targetnumber];
      mpForm->Target->Width = targsizex[targetnumber];
      Rotate( mpForm->Target, mRotateBy );
      mpForm->Target->Brush->Color = color;
      mpForm->Target->Visible = visible;
      break;

    // YES/NO targets
    case TYPE_YESNO:
      // turn text on when target comes on (turn it off when display cleared)
      if (targetstate == TARGET_ON)
       {
       // turn on both targets
       mpForm->Target->Top = targy[ 1 ];
       mpForm->Target->Left = targx[ 1 ];
       mpForm->Target->Height = targsizey[ 1 ];
       mpForm->Target->Width = targsizex[ 1 ];
       mpForm->Target->Brush->Color = clGray;
       Rotate( mpForm->Target, mRotateBy );
       mpForm->Target->Visible = true;

       mpForm->Target2->Top = targy[ 2 ];
       mpForm->Target2->Left = targx[ 2 ];
       mpForm->Target2->Height = targsizey[ 2 ];
       mpForm->Target2->Width = targsizex[ 2 ];
       mpForm->Target2->Brush->Color = clGray;
       Rotate( mpForm->Target2, mRotateBy );
       mpForm->Target2->Visible = true;

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
      break;
    default:
      /* do nothing */;
  }  
}

//-----------------------------------------------------------------------
void Usr::Clear( void )
{
  mpForm->Target->Hide();
  mpForm->Target2->Hide();
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

void
Usr::ComputeTargets()
{
  mNTargets = OptionalParameter( 2, "NumberTargets" );
  float y_range= limit_bottom - limit_top;

  for( int i=0;i<mNTargets;i++)
  {
    targx[i+1]=      limit_right - mTargetWidth;
    targsizex[i+1]=  mTargetWidth;
    targy[i+1]=      limit_top + i * y_range/mNTargets;
    targsizey[i+1]=  y_range/mNTargets;
    targy_btm[i+1]=  targy[i+1]+targsizey[i+1];
  }
}

int
Usr::TestCursorLocation( float x, float y, int inCurTarget )
{
  int CurrentOutcome = 0;

  if( x * scale_x > ( limit_right - mTargetWidth ) )
  {
    PutCursor( x, y, CURSOR_RESULT );

    CurrentOutcome= 1;

    y *= scale_y;
    if( y < targy[1] )              y= targy[1];
    if( y > targy_btm[mNTargets] )   y= targy_btm[mNTargets];

    for(int i=0;i<mNTargets;i++)
    {
            if( ( y > targy[i+1] ) && ( y<= targy_btm[i+1] ) )
            {
                    CurrentOutcome= i+1;
            }
    }

    // flash outcome (only) when YES/NO (display the same for hits and misses)
    Outcome(0, CurrentOutcome );

    switch( TargetType )
    {
      case TYPE_CHOICE:
        PutTarget( CurrentOutcome, TARGET_RESULT );
        break;
        
      case TYPE_TARGET:
      case TYPE_YESNO:
      default:
        if( CurrentOutcome == inCurTarget )
          PutTarget( CurrentOutcome, TARGET_RESULT );
        else
          PutTarget( CurrentOutcome, TARGET_OFF );
        break;
    }
  }
  return CurrentOutcome;
}

void
Usr::DeleteChoiceTargets()
{
  for( TargetContainer::const_iterator i = mChoiceTargets.begin(); i != mChoiceTargets.end(); ++i )
    delete *i;
  mChoiceTargets.clear();
}

void
Usr::HideBackground()
{
  for( TargetContainer::const_iterator i = mChoiceTargets.begin(); i != mChoiceTargets.end(); ++i )
    ( *i )->Hide();
}

void
Usr::ShowBackground()
{
  for( TargetContainer::const_iterator i = mChoiceTargets.begin(); i != mChoiceTargets.end(); ++i )
    ( *i )->Show();
}

