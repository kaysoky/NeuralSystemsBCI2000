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

class TTask : public GenericFilter
{
private:
        GenericVisualization    *vis;
        TSTATUS *status;
        TDecider *Decider;
        TTaskManager *TaskManager;
        TSessionManager *SessionManager;
        TClassSequencer *ClassSequencer;

public:
          TTask();
  virtual ~TTask();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const {}
  virtual void Initialize();
  virtual void Process( const GenericSignal* Input, GenericSignal* Output );
} ;

#endif
