/////////////////////////////////////////////////////////////////////////////
//
// File: TargetView.h
//
// Date: Oct 22, 2001
//
// Author: Juergen Mellinger
//
// Description: View for a single target. Instantiate TTargetView once for
//              each target you need a visual or auditory representation for.
//
// Changes: Thilo Hinterberger, Aug 21, 2002:
//          Added continuous auditive result.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TARGETVIEWH
#define TARGETVIEWH

#include "PresView.h"
#include "GUITargetView.h"

#include <string>
#include <cassert>

class TMidiPlayer;
class TWavePlayer;

class TTargetView : public TPresView, private TGUITargetView
{
  public:
                        TTargetView(    PARAMLIST   *inParamList,
                                        int         inTargetCode );
    virtual             ~TTargetView();

    virtual TPresError  Initialize(         PARAMLIST   *inParamList,
                                    const   TGUIRect    &inRect );

            TPresError  SetTextProperties( const PARAM  *inParamPtr );
            
        unsigned short  GetTargetCode() const;

            void        SetLabel( const std::string&    inLabel );
            void        SetLabel( const char*           inLabel );
    
    // "Event handling" methods
    virtual void        ProcessTrialActive(     const TEventArgs& ) {}
    virtual void        ProcessBeginOfTrial(    const TEventArgs& );
    virtual void        ProcessTaskBegin(       const TEventArgs& );
    virtual void        ProcessFeedbackBegin(   const TEventArgs& ) {}
    virtual void        ProcessFeedbackEnd(     const TEventArgs& ) {}
    virtual void        ProcessFeedback(        const TEventArgs& );
    virtual void        ProcessEndOfClass(      const TEventArgs& );
    virtual void        ProcessSuccess(         const TEventArgs& ) {}
    virtual void        ProcessItiBegin(        const TEventArgs& );
    virtual void        ProcessStopBegin(       const TEventArgs& );

  private:
            void        DoAuditiveResult();
            void        DoContAuditiveResult();
            
            int         targetCode,
                        visTaskMode,
                        visResultMode,
                        audTaskMode,
                        audResultMode,
                        continuousAudResult;
            bool        visGoal;

            TMidiPlayer *taskMidiPlayer,
                        *resultMidiPlayer;
            TWavePlayer *taskWavePlayer,
                        *resultWavePlayer,
                        *contResultWavePlayer;
};

inline
unsigned short
TTargetView::GetTargetCode() const
{
    assert( targetCode >= 0 );
    return targetCode;
}

inline
void
TTargetView::SetLabel( const std::string&   inLabel )
{
    TGUITargetView::SetLabel( inLabel.c_str() );
}

inline
void
TTargetView::SetLabel( const char*  inLabel )
{
    TGUITargetView::SetLabel( inLabel );
}

#endif // TARGETVIEWH

