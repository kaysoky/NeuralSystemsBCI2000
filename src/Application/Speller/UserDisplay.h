//---------------------------------------------------------------------------

#ifndef UserDisplayH
#define UserDisplayH
//---------------------------------------------------------------------------

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
        void    DisplayMessage(char *message);
        void    HideActiveTargets();
        void    HideStatusBar();
        void    HideCursor();
        void    HideMessage();
        void    InitializeActiveTargetPosition();
        void    InitializeStatusBarPosition();
        void    InitializeCursorPosition();
        void    UpdateCursorPosition(short *);
        void    SetWindowSize(int Top, int Left, int Width, int Height, TColor Color);
        TForm   *form;
        float   StatusBarSize, StatusBarTextHeight, CursorSize, TargetWidth, TargetTextHeight;
};
#endif
