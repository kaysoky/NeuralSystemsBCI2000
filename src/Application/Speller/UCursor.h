//---------------------------------------------------------------------------

#ifndef UCursorH
#define UCursorH
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
