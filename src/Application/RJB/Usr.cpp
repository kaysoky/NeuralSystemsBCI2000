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
}

//------------------------------------------------------------------------

void __fastcall TUser:: Initialize(PARAMLIST *plist, STATELIST *slist)
{

       Wx=  atoi(plist->GetParamPtr("WinXpos")->GetValue());
       Wy=  atoi(plist->GetParamPtr("WinYpos")->GetValue());
       Wxl= atoi(plist->GetParamPtr("WinWidth")->GetValue());
       Wyl= atoi(plist->GetParamPtr("WinHeight")->GetValue());
       CursorSize= atoi(plist->GetParamPtr("CursorSize")->GetValue());
       Cursor->Brush->Color= clBlack;
       
       User->ClientWidth=  Wxl;
       User->ClientHeight= Wyl;
       User->Left=         Wx;
       User->Top=          Wy;
       Cursor->Height=   CursorSize;
       Cursor->Width=    CursorSize;

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
void TUser::PutCursor(float x, float y, TColor color )
{
        x*= scale_x;
        y*= scale_y;

        if( y <= limit_top )    y= limit_top;
        if( y >= limit_bottom ) y= limit_bottom;
        if( x <= limit_left )   x= limit_left;
        if( x >= limit_right )  x= limit_right;

        Cursor->Top=  y - HalfCursorSize;
        Cursor->Left= x - HalfCursorSize;
        Cursor->Brush->Color= color;

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
void TUser::PutTarget(float x, float y, float sizex, float sizey, TColor color )
{
        Target->Top=  y;
        Target->Left= x;
        Target->Height= sizey;
        Target->Width = sizex;
        Target->Brush->Color= color;
}

//-----------------------------------------------------------------------
void TUser::Clear( void )
{
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


