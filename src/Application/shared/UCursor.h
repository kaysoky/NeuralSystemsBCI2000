/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UCursorH
#define UCursorH

#include <vcl.h>
//---------------------------------------------------------------------------
class CURSOR
{
private: 	// User declarations
        TShape  *cursor;
public:		// User declarations
        CURSOR::CURSOR(TForm *form);
        CURSOR::~CURSOR();
        void    RenderCursor(TForm *form, TRect destrect);
        int     Top, Left, Width, Height;
        void    Hide();
        void    Show();
        void    BringToFront();
};
#endif


