/////////////////////////////////////////////////////////////////////////////
//
// File: GUIFeedbackView.h
//
// Date: Nov 6, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUIFeedbackView class implements the GUI specific details
//              of the TFeedbackView class.
//
// Changes: Feb 16, 2003, jm: Moved "ZeroBar" functionality into a
//              "GUIGridView" class.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef GUI_FEEDBACK_VIEW_H
#define GUI_FEEDBACK_VIEW_H

#include "OSIncludes.h"

#include "GUI.h"
#include "GUIView.h"
#include "PresErrors.h"

class TGUIFeedbackView : protected TGUIView
{
  protected:
        // Only derived classes may instantiate this class.
                    TGUIFeedbackView();
                    ~TGUIFeedbackView();

  public:
    virtual void    Paint();
    virtual void    Resized();

  protected:
        TPresError  InitCursor( const char* inCursorFile = NULL );
        void        SetCursorCoordinates( float inX, float inY, float inBrightness );
        void        ShowCursor();
        void        HideCursor();
        void        InvertCursor();
        void        NormalCursor();

  private:
        void        InvalidateCursorRect();
        
            // Current cursor position in the view rectangle.
            float   cursorX,
                    cursorY,
                    cursorBrightness;
            bool    cursorVisible;
#ifdef PERUVIAN_BRIGHTNESS_HACK
#           define  PERUVIAN_CURSOR ((const char*)-1)
            bool    brightnessCursor;
#endif // PERUVIAN_BRIGHTNESS_HACK

// OS/library specific members go here.
#ifdef VCL
            Graphics::TBitmap   *cursorBitmap,
                                *normalBitmap,
                                *invertedBitmap;
            TRect               cursorTRect;
#endif // VCL
};

#endif // GUI_FEEDBACK_VIEW_H


