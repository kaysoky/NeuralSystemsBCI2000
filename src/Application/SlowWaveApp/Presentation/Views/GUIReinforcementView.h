/////////////////////////////////////////////////////////////////////////////
//
// File: GUIReinforcementView.h
//
// Date: Nov 9, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUIReinforcementView class implements the GUI specific
//              details of the TReinforcementView class.
//
// Changes:
//
//////////////////////////////////////////////////////////////////////////////

#ifndef GUI_REINFORCEMENT_VIEW_H
#define GUI_REINFORCEMENT_VIEW_H

#include "OSIncludes.h"
#include <vector>
#include <mmsystem.hpp>

#include "GUI.h"
#include "GUIView.h"
#include "PresErrors.h"

class TGUIReinforcementView : protected TGUIView
{
  protected:
        // Only derived classes may instantiate this class.
                    TGUIReinforcementView();
                    ~TGUIReinforcementView();

  public:
    virtual void    Paint();
    virtual void    Resized();

    static const int animFrameLength = 100; // ms

  protected:
        TPresError  InitAnimation( /*char   *inFileName = NULL*/ );
        void        PlayAnimation();

  private:
  // OS/library specific members go here.
#ifdef VCL
    static void     CALLBACK AnimationCallback( UINT    inTimerID,
                                                UINT,
                                                DWORD   inInstance,
                                                DWORD,
                                                DWORD );
                                                
        TRect               animTRect;
        Graphics::TBitmap   *animationBitmap;
        std::vector<Graphics::TBitmap*> animFrames;
        int                 animationCounter,
                            lastAnimationCounter,
                            numAnimationFrames;
        UINT                timerID;
#endif // VCL
};

#endif // GUI_REINFORCEMENT_VIEW_H


