/////////////////////////////////////////////////////////////////////////////
//
// File: GUIMarkerView.h
//
// Date: Feb 9, 2003
//
// Author: Juergen Mellinger
//
// Description: The TGUIMarkerView class implements the GUI specific details
//              of the TMarkerView class.
//
// Changes:
//
//////////////////////////////////////////////////////////////////////////////

#ifndef GUI_MARKER_VIEW_H
#define GUI_MARKER_VIEW_H

#include "OSIncludes.h"

#include "GUIView.h"
#include "PresErrors.h"

class TGUIMarkerView : protected TGUIView
{
  protected:
        // Only derived classes may instantiate this class.
                    TGUIMarkerView();
                    ~TGUIMarkerView();

  public:
    virtual void    Paint();

  protected:
        void        SetType( int inType );
        void        Show();
        void        Hide();

  private:
        int         type;
        bool        visible;

        int         lineWidth;
// OS/library specific members go here.
#ifdef VCL
        TColor      color;
#endif // VCL
};

#endif // GUI_MARKER_VIEW_H


