//---------------------------------------------------------------------------

#ifndef UsrH
#define UsrH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "UGenericFilter.h"
#include <ExtCtrls.hpp>


//---------------------------------------------------------------------------
class TUser : public TForm
{
__published:	// IDE-managed Components
        TShape *Cursor;
        TShape *Target;
        TLabel *tT;
        TLabel *tO;
private:	// User declarations
        int Wx;                 // task window x location
        int Wy;                 // task window y location
        int Wxl;                // task window x size
        int Wyl;                // task window y size
        int CursorSize;         // cursor size
        float cursor_y_start;   // y starting position of cursor
        float cursor_x_start;   // x starting position of cursor
        float limit_top;
        float limit_bottom;
        float limit_left;
        float limit_right;
        float xscalef;
        float yscalef;

public:		// User declarations
        int HalfCursorSize;     // 1/2 of cursor size
        float scalex;
        float scaley;
        void Scale( float , float );
        float ran1( long *idem );       // from Press et al
        void GetLimits( float *, float *, float *, float * );
        void GetSize( float *, float *, float *, float *);
       // void TestCursorLocation( float x, float y );
        void PutCursor( float *y, float *x, TColor color );      // self explanitory
        void PutTarget( float x, float y, float sizex, float sizey, TColor color );
        void PutT(bool);
        void PutO(bool);
        void Clear( void );
        void __fastcall SetUsr( PARAMLIST *plist, STATELIST *slist);
        void __fastcall Initialize(PARAMLIST *plist, STATELIST *slist);
        __fastcall TUser(TComponent* Owner);
        __fastcall ~TUser();
};
//---------------------------------------------------------------------------
extern PACKAGE TUser *User;
//---------------------------------------------------------------------------
#endif
