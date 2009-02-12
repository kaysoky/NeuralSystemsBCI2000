/////////////////////////////////////////////////////////////////////////////
//
// File: ScoreView.h
//
// Date: Oct 22, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes:
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef SCORE_VIEW_H
#define SCORE_VIEW_H

#include "PresView.h"
#include "GUIScoreView.h"
#include "StateAccessor.h"

class TScoreView : public TPresView, private TGUIScoreView
{
  public:
                        TScoreView( ParamList *inParamList );
    virtual             ~TScoreView();
    
    virtual TPresError  Initialize(         ParamList   *inParamList,
                                    const   TGUIRect    &inRect );
    
            TPresError  SetRect( const Param *inParamPtr );
            TPresError  SetTextProperties( const Param *inParamPtr );

    // "Event handling" methods
    virtual void        ProcessTrialActive(     const TEventArgs& ) {}
    virtual void        ProcessBeginOfTrial(    const TEventArgs& );
    virtual void        ProcessTaskBegin(       const TEventArgs& ) {}
    virtual void        ProcessFeedbackBegin(   const TEventArgs& ) {}
    virtual void        ProcessFeedbackEnd(     const TEventArgs& ) {}
    virtual void        ProcessFeedback(        const TEventArgs& ) {}
    virtual void        ProcessEndOfClass(      const TEventArgs& );
    virtual void        ProcessSuccess(         const TEventArgs& );
    virtual void        ProcessItiBegin(        const TEventArgs& );
    virtual void        ProcessStopBegin(       const TEventArgs& ) {}

  private:
    int                 scoreViewMode;
    TStateAccessor      artifact;
};

#endif // SCORE_VIEW_H

 
