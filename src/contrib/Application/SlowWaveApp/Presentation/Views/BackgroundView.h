/////////////////////////////////////////////////////////////////////////////
//
// File: BarView.h
//
// Date: Oct 22, 2001
//
// Author: Juergen Mellinger
//
// Description: A view that handles the background of the feedback window.
//
// Changes:
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef BACKGROUND_VIEW_H
#define BACKGROUND_VIEW_H

#include "PresView.h"
#include "GUIBackgroundView.h"

class TBackgroundView : public TPresView, private TGUIBackgroundView
{
  public:
                        TBackgroundView(    ParamList   *inParamList );
    virtual             ~TBackgroundView();

    virtual TPresError  Initialize(         ParamList   *inParamList,
                                    const   TGUIRect    &inRect );
    
    // "Event handling" methods
    virtual void        ProcessTrialActive(     const TEventArgs& ) {}
    virtual void        ProcessBeginOfTrial(    const TEventArgs& ) {}
    virtual void        ProcessTaskBegin(       const TEventArgs& ) {}
    virtual void        ProcessFeedbackBegin(   const TEventArgs& ) {}
    virtual void        ProcessFeedbackEnd(     const TEventArgs& ) {}
    virtual void        ProcessFeedback(        const TEventArgs& ) {}
    virtual void        ProcessEndOfClass(      const TEventArgs& ) {}
    virtual void        ProcessSuccess(         const TEventArgs& ) {}
    virtual void        ProcessItiBegin(        const TEventArgs& ) {}
    virtual void        ProcessStopBegin(       const TEventArgs& ) {}
};

#endif // BACKGROUND_VIEW_H

