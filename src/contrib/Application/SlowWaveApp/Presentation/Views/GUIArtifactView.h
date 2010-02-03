/////////////////////////////////////////////////////////////////////////////
//
// File: GUIArtifactView.h
//
// Date: Nov 9, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUIArtifactView class implements the GUI specific details
//              of the TArtifactView class.
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef GUI_ARTIFACT_VIEW_H
#define GUI_ARTIFACT_VIEW_H

#include "OSIncludes.h"

#include "GUI.h"
#include "GUIView.h"
#include "PresErrors.h"

class TGUIArtifactView : protected TGUIView
{
  protected:
        // Only derived classes may instantiate this class.
                    TGUIArtifactView();
                    ~TGUIArtifactView();

  public:
    virtual void    Paint();
    virtual void    Resized();

  protected:
        TPresError  InitIndicator();
        void        ShowIndicator();
        void        HideIndicator();

  private:
            bool    indicatorVisible;

// OS/library specific members go here.
#ifdef VCL
        TRect               indicTRect;
        Graphics::TBitmap   *indicatorBuffer;
#endif // VCL
};

#endif // GUI_ARTIFACT_VIEW_H


