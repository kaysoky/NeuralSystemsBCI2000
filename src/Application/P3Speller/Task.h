#ifndef TaskH
#define TaskH

#include <stdio.h>

#include "UBCITime.h"
#include "UCoreComm.h"
#include "UGenericVisualization.h"
#include "UGenericFilter.h"
#include "UTarget.h"

#include "UserDisplay.h"
#include "UTrialSequence.h"

class TTask
{
private:
        GenericVisualization    *vis;
        TARGETLIST      *targets, oldactivetargets;
        USERDISPLAY     *userdisplay;
        TRIALSEQUENCE   *trialsequence;
        CORECOMM        *corecomm;
        BCITIME         *bcitime;
        int             Wx, Wy, Wxl, Wyl;
        STATEVECTOR     *statevector;
        void            HandleSelected(TARGET *selected);
        BCITIME         *cur_time;
        int             cur_sequence, oldrunning, running;
        int             numberofsequences;              // how many sets of 12 intensifications ?
        int             postsetinterval;                // how much time after each set of numberofsequences sequences ?
        int             presetinterval;                 // how much time before each set of numberofsequences sequences ?
        void            ProcessSigProcResults( short *signals );
        int             responsecount[NUM_STIMULI];
        float           response[NUM_STIMULI];
        void            ResetTaskSequence();
        void            ProcessPostSequence();
        void            ProcessPreSequence();
        bool            postsequence, presequence;
        int             postsequencecount, presequencecount;
        AnsiString      DeterminePredictedCharacter();
        FILE            *logfile;
        int             cur_runnr;
public:
        TTask::TTask(PARAMLIST *plist, STATELIST *slist);
        TTask::~TTask( void );
        void Initialize(PARAMLIST *plist, STATEVECTOR *, CORECOMM *, TApplication *);
        void Process(short * );
};
#endif
