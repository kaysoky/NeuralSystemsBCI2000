//---------------------------------------------------------------------------

#ifndef UsrH
#define UsrH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <vector>

#include "UEnvironment.h"

//---------------------------------------------------------------------------
class TUser : public TForm
{
__published:	// IDE-managed Components
        TShape *Cursor;
        TShape *Target;
        TShape *Target2;
        TLabel *tT;
        TLabel *tO;
        TLabel *TargetText1;
        TLabel *TargetText2;
        TLabel *ResultText;
        TLabel *tPreRunIntervalText;
public:
        __fastcall TUser(TComponent*)
        : TForm( static_cast<TComponent*>( NULL ) ) {}
};

class Usr : private Environment
{
public:
  enum
  {
    TARGET_OFF = 0,
    TARGET_ON = 1,
    TARGET_RESULT = 2,

    CURSOR_OFF = 0,
    CURSOR_ON = 1,
    CURSOR_RESULT = 2,

    TYPE_TARGET = 0,
    TYPE_YESNO = 1,
    TYPE_CHOICE = 2,

    NTARGS = 8,
  };

private: // User declarations
        TUser* mpForm;
        typedef std::vector<TShape*> TargetContainer;
        TargetContainer mChoiceTargets;
        int Wx;                 // task window x location
        int Wy;                 // task window y location
        int Wxl;                // task window x size
        int Wyl;                // task window y size
        int CursorSize;         // cursor size
        int HalfCursorSize;     // 1/2 of cursor size
        int TargetType;         // Targets or YES/NO or choices
        int YesNoCorrect;
        int YesNoOnTime;
        int YesNoOffTime;
        float cursor_y_start;   // y starting position of cursor
        float cursor_x_start;   // x starting position of cursor
        float limit_top;
        float limit_bottom;
        float limit_left;
        float limit_right;
        float xscalef;
        float yscalef;
        AnsiString      TargetWord, NoTargetWord;
        int   mNTargets;
        float mTargetWidth;
        float mRotateBy;
        void  Rotate( TControl*, float );
        static TColor Brighten( TColor );
public:		// User declarations
        float targx[NTARGS+1];
        float targy[NTARGS+1];
        float targsizex[NTARGS+1];
        float targsizey[NTARGS+1];
        float targy_btm[NTARGS+1];

        float scale_x;
        float scale_y;
public:
        float ran1( long *idem );       // from Press et al
        void GetLimits(float *right, float *left, float *top, float *bottom );
        int  TestCursorLocation( float x, float y, int currentTarget );
        void PutCursor( float y, float x, int );      // self explanatory
        void PutTarget( int targno, int );
        void PutT(bool);
        void PutO(bool);
        void Clear( void );
        void HideBackground();
        void ShowBackground();
        void PreRunInterval(int time);
        void Outcome(int time, int result);
        void Initialize();
        Usr();
        ~Usr();
private:
        void DeleteChoiceTargets();
        void Scale( float , float );
        void ComputeTargets();
};
//---------------------------------------------------------------------------
#endif
