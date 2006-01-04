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
        TLabel          *message;
        STATUSBAR       *statusbar;
        void    HideActiveTargets();
        void    DisplayActiveTargets();
        void    DisplayMessage(const char *message);
        void    DisplayStatusBar();
        void    HideStatusBar();
        void    HideMessage();
        void    InitializeActiveTargetPosition();
        void    InitializeStatusBarPosition(bool);
        void    SetWindowSize(int Top, int Left, int Width, int Height, TColor Color);
        TForm   *form;
        float   TargetWidth, TargetHeight, TargetTextHeight;
        float   StatusBarSize, StatusBarTextHeight;
        /*shidong starts*/
        int     displayRow;
        int     displayCol;
        /*shidong ends*/
	/* VK Text Window Stuff*/
	TForm	*textform;
	TMemo	*textwindow;
	void	DisplayTextWindow(int FontSize, const char *FontName);
	void    SetTextWindowSize(int Top, int Left, int Width, int Height);
	void	DisableTextWindow();
};
#endif
