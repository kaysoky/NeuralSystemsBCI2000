/////////////////////////////////////////////////////////////////////////////
//
// File: ReinforcementView.h
//
// Date: Oct 22, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes:
//
//////////////////////////////////////////////////////////////////////////////

#ifndef REINFORCEMENT_VIEW_H
#define REINFORCEMENT_VIEW_H

#include "PresView.h"
#include "MidiPlayer.h"
#include "GUIReinforcementView.h"

class TWavePlayer;

class TReinforcementView : public TPresView, protected TGUIReinforcementView
{
  public:
                        TReinforcementView( PARAMLIST *inParamList );
    virtual             ~TReinforcementView();
    
    virtual TPresError  Initialize(         PARAMLIST   *inParamList,
                                    const   TGUIRect    &inRect );
    
    // "Event handling" methods
    virtual void        ProcessTrialActive(     const TEventArgs& ) {}
    virtual void        ProcessBeginOfTrial(    const TEventArgs& );
    virtual void        ProcessTaskBegin(       const TEventArgs& ) {}
    virtual void        ProcessFeedbackBegin(   const TEventArgs& ) {}
    virtual void        ProcessFeedbackEnd(     const TEventArgs& ) {}
    virtual void        ProcessFeedback(        const TEventArgs& ) {}
    virtual void        ProcessEndOfClass(      const TEventArgs& ) {}
    virtual void        ProcessSuccess(         const TEventArgs& );
    virtual void        ProcessItiBegin(        const TEventArgs& ) {}
    virtual void        ProcessStopBegin(       const TEventArgs& );

  private:
            int         visReinforcement,
                        audReinforcement,
                        freqAny,
                        freqCorrect,
                        trialsSinceLastReinforcement,
                        correctTrialsSinceLastReinforcement;
            TWavePlayer *wavePlayer;
            TMidiPlayer *midiPlayer;

    static  TMidiPlayer::TMidiNote  noteSeq[];
};

#endif // REINFORCEMENT_VIEW_H

