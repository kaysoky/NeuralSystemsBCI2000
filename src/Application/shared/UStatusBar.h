//---------------------------------------------------------------------------

#ifndef UStatusBarH
#define UStatusBarH

#include <vcl.h>
//---------------------------------------------------------------------------

class STATUSBAR
{
private: 	// User declarations
        TLabel  *goal, *result;
        TShape  *background, *divider;
public:		// User declarations
        STATUSBAR::STATUSBAR(TForm *form);
        STATUSBAR::~STATUSBAR();
        AnsiString      goaltext;
        AnsiString      resulttext;
        TColor  Color, StatusBarColor;
        int     Top, Left, Width, Height;
        int     GoalPosX, GoalPosY;
        int     ResultPosX, ResultPosY;
        int     TextHeight;
        void    RenderStatusBar(TForm *form, TRect destrect);
        void    Hide();
        void    Show();
};

#endif
