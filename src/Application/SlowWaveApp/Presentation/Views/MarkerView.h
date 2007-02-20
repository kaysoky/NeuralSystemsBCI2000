/////////////////////////////////////////////////////////////////////////////
//
// File: MarkerView.h
//
// Date: Oct 22, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes: Feb 16, 2003, jm: Introduced TGUIMarkerView for
//          Zero Bar / Fixation Cross display.
//          May 13, 2003, jm: Introduced multiple auditory markers
//          at arbitrary temporal offsets.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef MARKER_VIEW_H
#define MARKER_VIEW_H

#include <list>
#include "MidiPlayer.h"
#include "WavePlayer.h"
#include "PresView.h"
#include "GUIMarkerView.h"

class TMarkerView : public TPresView, private TGUIMarkerView
{
  public:
                        TMarkerView( PARAMLIST *inParamList );
    virtual             ~TMarkerView();

    virtual TPresError  Initialize(         PARAMLIST   *inParamList,
                                    const   TGUIRect    &inRect );
    
    // "Event handling" methods
    virtual void        ProcessTrialActive(     const TEventArgs& );
    virtual void        ProcessBeginOfTrial(    const TEventArgs& );
    virtual void        ProcessTaskBegin(       const TEventArgs& ) {}
    virtual void        ProcessFeedbackBegin(   const TEventArgs& ) {}
    virtual void        ProcessFeedbackEnd(     const TEventArgs& ) {}
    virtual void        ProcessFeedback(        const TEventArgs& ) {}
    virtual void        ProcessEndOfClass(      const TEventArgs& ) {}
    virtual void        ProcessSuccess(         const TEventArgs& ) {}
    virtual void        ProcessItiBegin(        const TEventArgs& );
    virtual void        ProcessStopBegin(       const TEventArgs& );

  private:
        int             visMarker;

        struct audMarker
        {
          float        timeOffset;
          TMidiPlayer* midiPlayer;
          TWavePlayer* wavePlayer;
          bool operator<( const audMarker& b ) const { return timeOffset < b.timeOffset; }
        };
        typedef std::list<audMarker> audMarkerContainer;
        audMarkerContainer audMarkers;
        audMarkerContainer::iterator currentMarker;
        float lastTimeOffset;

        void ClearMarkers();
};

#endif // MARKER_VIEW_H

