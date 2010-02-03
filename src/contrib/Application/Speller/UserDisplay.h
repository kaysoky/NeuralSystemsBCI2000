/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UserDisplayH
#define UserDisplayH
//---------------------------------------------------------------------------
#include <vector>

#include "UCursor.h"
#include "UStatusBar.h"
#include "UTarget.h"


class USERDISPLAY
{
private:	// User declarations
public:		// User declarations
        USERDISPLAY::USERDISPLAY();
        USERDISPLAY::~USERDISPLAY();
        TARGETLIST      *activetargets;
        STATUSBAR       *statusbar;
        CURSOR          *cursor;
        TLabel          *message;
        void    DisplayActiveTargets();
        void    DisplayStatusBar();
        void    DisplayCursor();
        void    DisplayMessage(const char *message);
        void    HideActiveTargets();
        void    HideStatusBar();
        void    HideCursor();
        void    HideMessage();
        void    InitializeActiveTargetPosition();
        void    InitializeStatusBarPosition();
        void    InitializeCursorPosition();
        void    UpdateCursorPosition(const std::vector<float>&);
        void    SetWindowSize(int Top, int Left, int Width, int Height, TColor Color);
        TForm   *form;
        float   StatusBarSize, StatusBarTextHeight, CursorSize, TargetWidth, TargetTextHeight;
};
#endif
