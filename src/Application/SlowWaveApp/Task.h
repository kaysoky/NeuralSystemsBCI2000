/*************************************************************************
Task.h is the header file for the Right Justified Boxes task
*************************************************************************/
#ifndef TaskH
#define TaskH

#include "UBitRate.h"
#include "UCoreComm.h"
#include "UGenericVisualization.h"
#include "UGenericFilter.h"
#include "AppManager.h"
#include "SWUser.h"

#define NTARGS  8

class TTask
{
private:
        STATEVECTOR             *svect;
        GenericVisualization    *vis;
        CORECOMM        *corecomm;
        TSTATUS *status;
        TDecider *Decider;
        TTaskManager *TaskManager;
        TSessionManager *SessionManager;
        TClassSequencer *ClassSequencer;

public:
        void Initialize(PARAMLIST *plist, STATEVECTOR *, CORECOMM *, TApplication *applic);
        void Process(short * );
        TTask(PARAMLIST *plist, STATELIST *slist);
        ~TTask( void );
} ;

#endif
