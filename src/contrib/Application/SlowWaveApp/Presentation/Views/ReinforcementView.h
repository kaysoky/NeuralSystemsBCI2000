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
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef REINFORCEMENT_VIEW_H
#define REINFORCEMENT_VIEW_H

#include "PresView.h"
#include "MidiPlayer.h"
#include "GUIReinforcementView.h"

class WavePlayer;

class TReinforcementView : public TPresView, protected TGUIReinforcementView
{
  public:
                        TReinforcementView( ParamList *inParamList );
    virtual             ~TReinforcementView();
    
    virtual TPresError  Initialize(         ParamList   *inParamList,
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
            WavePlayer  *wavePlayer;
            MidiPlayer  *midiPlayer;

    static  MidiPlayer::MidiNote  noteSeq[];
};

#endif // REINFORCEMENT_VIEW_H

