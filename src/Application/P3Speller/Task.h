#ifndef TaskH
#define TaskH

#include <stdio.h>
#include <vector>

#include "UBCITime.h"
#include "UGenericVisualization.h"
#include "UGenericFilter.h"
#include "UTarget.h"

#include "UserDisplay.h"
#include "UTrialSequence.h"

class TTask : public GenericFilter
{
 private:
        GenericVisualization mVis;
        TARGETLIST      *targets, oldactivetargets;
        USERDISPLAY     *userdisplay;
        TRIALSEQUENCE   *trialsequence;
        int             Wx, Wy, Wxl, Wyl;
        void            HandleSelected(TARGET *selected);
        BCITIME         *cur_time,
                        *bcitime;
        int             cur_sequence, oldrunning, running;
        int             numberofsequences;              // how many sets of 12 intensifications ?
        int             postsetinterval;                // how much time after each set of numberofsequences sequences ?
        int             presetinterval;                 // how much time before each set of numberofsequences sequences ?
        void            ProcessSigProcResults( const std::vector<float>& signals );
        int             responsecount[NUM_STIMULI];
        float           response[NUM_STIMULI];
        void            ResetTaskSequence();
        void            ProcessPostSequence();
        void            ProcessPreSequence();
        bool            postsequence, presequence, postpostsequence;
        int             postsequencecount, presequencecount;
        AnsiString      DeterminePredictedCharacter();
        FILE            *logfile;
        int             cur_runnr;
 public:
          TTask();
  virtual ~TTask();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const {}
  virtual void Initialize();
  virtual void Process( const GenericSignal* Input, GenericSignal* Output );
};
#endif
