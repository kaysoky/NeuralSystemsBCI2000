/////////////////////////////////////////////////////////////////////////////
//
// File: FeedbackView.h
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

#ifndef FEEDBACK_VIEW_H
#define FEEDBACK_VIEW_H

#include "PresView.h"
#include "StateAccessor.h"
#include "GUIFeedbackView.h"

class TMidiPlayer;
#ifdef USE_WAVE_SYNTH
class TWaveSynth;
#endif // USE_WAVE_SYNTH

class TFeedbackView : public TPresView, private TGUIFeedbackView
{
  public:
                        TFeedbackView( PARAMLIST *inParamList );
    virtual             ~TFeedbackView();
    virtual TPresError  Initialize(         PARAMLIST   *inParamList,
                                    const   TGUIRect    &inRect );

    // "Event handling" methods
    virtual void        ProcessTrialActive(     const TEventArgs& );
    virtual void        ProcessBeginOfTrial(    const TEventArgs& ) {}
    virtual void        ProcessTaskBegin(       const TEventArgs& ) {}
    virtual void        ProcessFeedbackBegin(   const TEventArgs& );
    virtual void        ProcessFeedbackEnd(     const TEventArgs& );
    virtual void        ProcessFeedback(        const TEventArgs& );
    virtual void        ProcessEndOfClass(      const TEventArgs& ) {}
    virtual void        ProcessSuccess(         const TEventArgs& ) {}
    virtual void        ProcessItiBegin(        const TEventArgs& ) {}
    virtual void        ProcessStopBegin(       const TEventArgs& );

  private:
    void DoFeedback( const TEventArgs& );

  private:
            TStateAccessor  artifact;
            TMidiPlayer*    midiPlayer;
#ifdef USE_WAVE_SYNTH
            TWaveSynth*     waveSynth;
#endif // USE_WAVE_SYNTH
            int             xChannel,
                            yChannel,
                            bChannel,
                            visFBMode,
                            audFBMode,
                            fbAlwaysOn,
                            gmFBCenterNote,
                            lastNote,
                            lastMidiNote,
                            *upScale,
                            *downScale,
                            scaleLength;
            float           gmFBInterval;

    static  int         nullScale[],
                        nullScaleLength,
                        chromaticScale[],
                        chromaticScaleLength,
                        majorScale[],
                        majorScaleLength,
                        melodicMinorUpScale[],
                        melodicMinorDownScale[],
                        melodicMinorScaleLength,
                        harmonicMinorScale[],
                        minorScaleLength,
                        pentatonicScale[],
                        pentatonicScaleLength,
                        diatonicScale[],
                        diatonicScaleLength;
};

#endif // FEEDBACK_VIEW_H

