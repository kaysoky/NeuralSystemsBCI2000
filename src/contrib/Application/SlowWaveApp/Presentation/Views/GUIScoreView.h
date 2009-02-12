/////////////////////////////////////////////////////////////////////////////
//
// File: GUIScoreView.h
//
// Date: Nov 9, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUIScoreView class implements the GUI specific details
//              of the TScoreView class.
//
// Changes: Mar 17, 2003, jm:
//          Moved buffer update from Paint() into InvalidateBuffer() to
//          adapt to new window invalidation scheme.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef GUI_SCORE_VIEW_H
#define GUI_SCORE_VIEW_H

#include "OSIncludes.h"

#include "GUIView.h"
#include "PresErrors.h"
#include "TextProperties.h"

class TGUIScoreView : protected TGUIView, protected TTextProperties
{
  protected:
        // Only derived classes may instantiate this class.
                    TGUIScoreView();
                    ~TGUIScoreView();

  public:
    virtual void    Paint();
    virtual void    Resized();

  protected:
        void        Show();
        void        Hide();
        void        InvalidateBuffer();
        
        bool        visible;
        int         successTrials,
                    totalTrials;

  private:
// OS/library specific members go here.
#ifdef VCL
    Graphics::TBitmap   *buffer;
#endif // VCL
};

#endif // GUI_SCORE_VIEW_H


