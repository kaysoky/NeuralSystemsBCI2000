/////////////////////////////////////////////////////////////////////////////
//
// File: PresTaskModel.h
//
// Date: Oct 18, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes:
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef PRES_TASK_MODEL_H
#define PRES_TASK_MODEL_H

#include "PresModel.h"
#include "PresBroadcasting.h"
#include "StateAccessor.h"
#include "TargetSeq.h"

class TStimulusView;

class TPresTaskModel : public TPresModel, public TPresListener
{
  public:
                        TPresTaskModel( ParamList        *inParamList,
                                        TPresBroadcaster *inBroadcaster );
    virtual             ~TPresTaskModel();

    virtual void        Reset();
    virtual void        NextTarget();

    // "Event handling" methods from TPresListener
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

  private:
    virtual void        DoCleanup();
    virtual TPresError  DoInitialize(   ParamList        *inParamList,
                                        TPresBroadcaster *inBroadcaster );

            int             sequenceType,
                            numberOfTargets;

            TStateAccessor  targetCode;

            TTargetSeq      targetSeq;
            TTargetSeq::const_iterator  targetSeqPos;

            TStimulusView   *stimulusView;
};

#endif // PRES_TASK_MODEL_H
 
