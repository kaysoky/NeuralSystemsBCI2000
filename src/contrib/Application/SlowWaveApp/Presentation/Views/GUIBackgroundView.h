/////////////////////////////////////////////////////////////////////////////
//
// File: GUIBackgroundView.h
//
// Date: Nov 9, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUIBackgroundView class implements the GUI specific details
//              of the TBackgroundView class.
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef GUI_BACKGROUND_VIEW_H
#define GUI_BACKGROUND_VIEW_H

#include "OSIncludes.h"

#include "GUI.h"
#include "GUIView.h"
#include "PresErrors.h"

class TGUIBackgroundView : protected TGUIView
{
  protected:
        // Only derived classes may instantiate this class.
                    TGUIBackgroundView();
                    ~TGUIBackgroundView() {}

  public:
    virtual void    Paint();

  private:
// OS/library specific members go here.
#ifdef VCL
#endif // VCL
};

#endif // GUI_BACKGROUND_VIEW_H


