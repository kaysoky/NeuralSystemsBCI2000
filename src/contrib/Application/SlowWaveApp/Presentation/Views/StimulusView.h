/////////////////////////////////////////////////////////////////////////////
//
// File: StimulusView.h
//
// Date: Nov 16, 2001
//
// Author: Juergen Mellinger
//
// Description: A view that presents a visual or auditory stimulus
//              when ProcessTaskBegin() is called.
//
// Changes:
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef STIMULUS_VIEW_H
#define STIMULUS_VIEW_H

#include "PresView.h"
#include "TargetSeq.h"
#include "GUIStimulusView.h"

class WavePlayer;

class TStimulusView : public TPresView, private TGUIStimulusView
{
  public:
                        TStimulusView(  ParamList   *inParamList );
    virtual             ~TStimulusView();

    virtual TPresError  Initialize(         ParamList   *inParamList,
                                    const   TGUIRect    &inRect );

            void        AttachStimuli( const TTargetSeqEntry &inTargetEntry );


    // "Event handling" methods
    virtual void        ProcessTrialActive(     const TEventArgs& ) {}
    virtual void        ProcessBeginOfTrial(    const TEventArgs& ) {}
    virtual void        ProcessTaskBegin(       const TEventArgs& );
    virtual void        ProcessFeedbackBegin(   const TEventArgs& ) {}
    virtual void        ProcessFeedbackEnd(     const TEventArgs& ) {}
    virtual void        ProcessFeedback(        const TEventArgs& ) {}
    virtual void        ProcessEndOfClass(      const TEventArgs& ) {}
    virtual void        ProcessSuccess(         const TEventArgs& ) {}
    virtual void        ProcessItiBegin(        const TEventArgs& ) {}
    virtual void        ProcessStopBegin(       const TEventArgs& );

  private:
            WavePlayer  *wavePlayer;
            int         visStimDuration;
};

#endif // STIMULUS_VIEW_H
