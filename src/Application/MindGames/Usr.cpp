//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <extctrls.hpp>

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
int i;

 for (i=0; i<MAX_BRICKS; i++)
  brick[i]=NULL;

}
//--------------------------------------------------------------
_fastcall TUser::~TUser()
{
int i;

       for (i=0; i<MAX_BRICKS; i++)
        if (brick[i]) delete brick[i];

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
        strcpy(line,"UsrTask int CursorSizeX= 75 0 0 1 // User Window Cursor Size");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int CursorSizeY= 20 0 0 1 // User Window Cursor Size");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int NumBricksX= 4 0 0 1000    // Number of Bricks Horizontally");
        plist->AddParameter2List(line,strlen(line));
        strcpy(line,"UsrTask int NumBricksy= 3 0 0 1000    // Number of Bricks Vertically");
        plist->AddParameter2List(line,strlen(line));
}

//------------------------------------------------------------------------

void __fastcall TUser:: Initialize(PARAMLIST *plist, STATELIST *slist)
{
int i, x, y, brickwidth, brickheight;

       Wx=  atoi(plist->GetParamPtr("WinXpos")->GetValue());
       Wy=  atoi(plist->GetParamPtr("WinYpos")->GetValue());
       Wxl= atoi(plist->GetParamPtr("WinWidth")->GetValue());
       Wyl= atoi(plist->GetParamPtr("WinHeight")->GetValue());
       num_x= atoi(plist->GetParamPtr("NumBricksX")->GetValue());
       num_y= atoi(plist->GetParamPtr("NumBricksY")->GetValue());
       Wyl= atoi(plist->GetParamPtr("WinHeight")->GetValue());
       CursorSizeX= atoi(plist->GetParamPtr("CursorSizeX")->GetValue());
       CursorSizeY= atoi(plist->GetParamPtr("CursorSizeY")->GetValue());
       Cursor->Brush->Color= clBlack;

       User->ClientWidth=  Wxl;
       User->ClientHeight= Wyl;
       User->Left=         Wx;
       User->Top=          Wy;
       Cursor->Height=   CursorSizeY;
       Cursor->Width=    CursorSizeX;

       for (i=0; i<MAX_BRICKS; i++)
        if (brick[i]) delete brick[i];

       color[0]=clAqua;
       color[1]=clYellow;
       color[2]=clRed;
       color[3]=clBlue;
       color[4]=clGreen;

       i=0;
       brickwidth=Wxl/num_x;
       brickheight=Wyl/3/num_y;
       for (x=0; x<num_x; x++)
        {
        for (y=0; y<num_y; y++)
         {
         brick[i]=new TShape(this);
         brick[i]->Parent=this;
         brick[i]->Brush->Color=color[i%5];
         brick[i]->Top=10+y*brickheight;
         brick[i]->Left=x*brickwidth;
         brick[i]->Width=brickwidth;
         brick[i]->Height=brickheight;
         brick[i]->Visible=true;
         i++;
         }
        }

       tT->Font->Height=-Wyl/7;
       Canvas->Font=tT->Font;
       tT->Left=abs(Wxl/2-Canvas->TextWidth(tT->Caption)/2);
       tT->Top=abs(tT->Font->Height/8);

       HalfCursorSizeX= CursorSizeX / 2;
       HalfCursorSizeY= CursorSizeY / 2;

       limit_top= 0; //CursorSize/2;
       limit_bottom= Wyl; //  - CursorSize/2;
       limit_left= HalfCursorSizeX;  //  0; CursorSize/2;
       limit_right= Wxl - HalfCursorSizeX; // - CursorSize/2;

       User->Show();
}
//----------------------------------------------------------------------------

void TUser::TestEdgeCollision(float *ball_x_pos, float *ball_y_pos, float *ball_delta_x, float *ball_delta_y)
{
 // right edge
 if (*ball_x_pos > (Wxl-Ball->Width))
    {
    *ball_x_pos=2*(Wxl-Ball->Width)-*ball_x_pos;
    *ball_delta_x=-*ball_delta_x;
    }
 // left edge
 if (*ball_x_pos < 1)
    {
    *ball_x_pos=1-*ball_x_pos;
    *ball_delta_x=-*ball_delta_x;
    }
 // bottom edge
 if (*ball_y_pos > (Wyl-Ball->Height))
    {
    *ball_y_pos=2*(Wyl-Ball->Height)-*ball_y_pos;
    *ball_delta_y=-*ball_delta_y;
    }
 // top edge
 if (*ball_y_pos < 1)
    {
    *ball_y_pos=1-*ball_y_pos;
    *ball_delta_y=-*ball_delta_y;
    }
}


void TUser::TestBrickCollision(float *ball_x_pos, float *ball_y_pos, float *ball_delta_x, float *ball_delta_y)
{
int i;

 for (i=0; i<MAX_BRICKS; i++)
  {
  // does brick exist ?
  if (brick[i])
     {
     // collision between brick and ball
     if ((*ball_x_pos >= brick[i]->Left) && (*ball_x_pos <= brick[i]->Left+brick[i]->Width))
        {
        if ((*ball_y_pos >= brick[i]->Top) && (*ball_y_pos <= brick[i]->Top+brick[i]->Height))
           {
           *ball_delta_y=-*ball_delta_y;
           delete (brick[i]);
           brick[i]=NULL;
           break;
           }
        }
     }
  }
}


void TUser::GetLimits(float *right, float *left, float *top, float *bottom )
{
        *(right) = limit_right;
        *(left)  = limit_left;
        *(top)   = limit_top;
        *(bottom)= limit_bottom-(float)HalfCursorSizeY;
}

//----------------------------------------------------------------------------
void TUser::PutCursor(float x, float y, TColor color )
{

        if( y <= limit_top )    y= limit_top;
        if( y >= limit_bottom ) y= limit_bottom;
        if( x <= limit_left )   x= limit_left;
        if( x >= limit_right )  x= limit_right;

        Cursor->Top=  y - HalfCursorSizeY;
        Cursor->Left= x - HalfCursorSizeX;
        Cursor->Brush->Color= color;

}

void TUser::PutT(bool Tstate)
{
 if (Tstate)
    {
    Clear();
    tT->Visible=true;
    }
 else
    tT->Visible=false;
}

void TUser::PutBall(float x, float y, float sizex, float sizey, TColor color )
{
        Ball->Top=  y;
        Ball->Left= x;
        Ball->Height= sizey;
        Ball->Width = sizex;
        Ball->Brush->Color= color;
}

//-----------------------------------------------------------------------
void TUser::Clear( void )
{
int i;

       for (i=0; i<MAX_BRICKS; i++)
        if (brick[i]) brick[i]->Visible=false;

        Target->Brush->Color= clBlack;
        Cursor->Brush->Color= clBlack;
        Refresh();
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



