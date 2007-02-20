/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
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
        void    HideActiveTargets();
        void    DisplayActiveTargets();
        void    DisplayMessage(char *message);
        void    HideMessage();
        void    InitializeActiveTargetPosition();
        void    SetWindowSize(int Top, int Left, int Width, int Height, TColor Color);
        TForm   *form;
        float   TargetWidth, TargetHeight, TargetTextHeight;
};
#endif


