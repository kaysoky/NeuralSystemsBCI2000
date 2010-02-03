/////////////////////////////////////////////////////////////////////////////
//
// File: TextFrame.h
//
// Date: Oct 19, 2001
//
// Author: Juergen Mellinger
//
// Description: A frame to display text. This class is the GUI independent
//              interface used by the other presentation classes.
//              All GUI dependent functions are in the TGUITextFrame
//              class.
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef TEXTFRAMEH
#define TEXTFRAMEH

#include <string>

#include "GUITextFrame.h"
#include "PresErrors.h"
#include "Utils/Util.h"

class PARAM;

class TTextFrame : TGUITextFrame
{
    public:
                    TTextFrame();
                    ~TTextFrame();
        TPresError  SetRect(            const Param          *inParamPtr );
        TPresError  SetTextProperties(  const Param          *inParamPtr );
        void        SetText(            const TInputSequence &inText );
        TInputSequence GetText() const;
        void        AddText(    const TInputSequence &inText );
        void        LoadText(   const char           *inFileName );
        void        SaveText(   const char           *inFileName );
        void        ShowCursor();
        void        HideCursor();

        // This is a workaround for a strange problem with static
        // class variables being destructed before the application form
        // on program exit.
        Util::TPath pathBuffer;
};

#endif // TEXTFRAMEH

