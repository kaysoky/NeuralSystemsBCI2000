/////////////////////////////////////////////////////////////////////////////
//
// File: PresSpellerModel.h
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

#ifndef PRESSPELLERMODELH
#define PRESSPELLERMODELH

#include <string>

#include "PresModel.h"
#include "PresBroadcasting.h"
#include "SpellerTree.h"
#include "SpellerDict.h"
#include "WavePlayer.h"

class TTextFrame;
class TTargetView;
class TStimulusView;

class TPresSpellerModel : public TPresModel, public TPresListener
{
  public:
                        TPresSpellerModel(  ParamList        *inParamList,
                                            TPresBroadcaster *inBroadcaster );
    virtual             ~TPresSpellerModel();

    virtual void        Reset();
    virtual void        NextTarget();

    // "Event handling" methods
    virtual void        ProcessTrialActive(     const TEventArgs& ) {}
    virtual void        ProcessBeginOfTrial(    const TEventArgs& ) {}
    virtual void        ProcessTaskBegin(       const TEventArgs& );
    virtual void        ProcessFeedbackBegin(   const TEventArgs& ) {}
    virtual void        ProcessFeedbackEnd(     const TEventArgs& ) {}
    virtual void        ProcessFeedback(        const TEventArgs& ) {}
    virtual void        ProcessEndOfClass(      const TEventArgs& );
    virtual void        ProcessSuccess(         const TEventArgs& ) {}
    virtual void        ProcessItiBegin(        const TEventArgs& ) {}
    virtual void        ProcessStopBegin(       const TEventArgs& ) {}

  private:
    virtual void        DoCleanup();
    virtual TPresError  DoInitialize(   ParamList        *inParamList,
                                        TPresBroadcaster *inBroadcaster );

            int                 mode,
                                autoBackspace;

            TTextFrame          *documentFrame,
                                *textEntryFrame;

            TStateAccessor      targetCode,
                                artifact;

            std::list< TTargetView* >   targetViews;

            TSpellerTree        spellerTree;
            TSpellerDict        spellerDict;
            bool                showingProposal,
                                failureReported;
            std::string         lastProposal;
            WavePlayer          spellerWavePlayer;
};

#endif // PRESSPELLERMODELH

 
