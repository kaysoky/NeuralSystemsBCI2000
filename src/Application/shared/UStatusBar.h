/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
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
        bool    resulttextvisible;
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
        void    SetResultTextVisibility(bool cur_resulttextvisible);
        void    RenderStatusBar(TForm *form, TRect destrect);
        void    Hide();
        void    Show();
};

#endif


