/////////////////////////////////////////////////////////////////////////////
//
// File: GUITextFrame.h
//
// Date: Oct 19, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUITextFrame implements the GUI specific details
//              of the text frame class.
//
// Changes:
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef GUI_TEXT_FRAME_H
#define GUI_TEXT_FRAME_H

#include <string>

#include "OSIncludes.h"
#include "TextProperties.h"
#include "GUIView.h"
#include "GUI.h"
#include "PresErrors.h"

class TGUITextFrame : protected TGUIView, protected TTextProperties
{
  public:
                    TGUITextFrame();
                    ~TGUITextFrame();

    virtual void    Paint();
    virtual void    Resized();


  protected:
            void        SetText( const TInputSequence& inText );
            const TInputSequence& GetText() const;
            void        Bell() const;
            TPresError  LoadText( const char *inFileName );
            TPresError  SaveText( const char *inFileName );
            void        ShowCursor();
            void        HideCursor();

  private:
// OS/Library specific members go here.
#ifdef VCL
        TMemo           *memo;
#endif
};

#endif // TEXT_FRAME_H

