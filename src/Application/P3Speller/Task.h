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
        int             Wx, Wy, Wxl, Wyl;
        STATEVECTOR     *statevector;
        void            HandleSelected(TARGET *selected);
        BCITIME         *cur_time;
public:
        TTask::TTask(PARAMLIST *plist, STATELIST *slist);
        TTask::~TTask( void );
        void Initialize(PARAMLIST *plist, STATEVECTOR *, CORECOMM *, TApplication *);
        void Process(short * );
};
#endif
