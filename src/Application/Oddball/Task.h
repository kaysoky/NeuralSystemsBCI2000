/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef TaskH
#define TaskH

#include "UBCITime.h"
#include "UGenericVisualization.h"
#include "UGenericFilter.h"
#include "UTarget.h"

#include "UserDisplay.h"
#include "UTargetSequence.h"
#include "UTrialSequence.h"

class TTask : public GenericFilter
{
 private:
        GenericVisualization    *vis;
        TARGETLIST      *targets, oldactivetargets;
        USERDISPLAY     *userdisplay;
        TARGETSEQUENCE  *targetsequence;
        TRIALSEQUENCE   *trialsequence;
        int             Wx, Wy, Wxl, Wyl;
        void            HandleSelected(TARGET *selected);
        BCITIME         *cur_time;

 public:
          TTask();
  virtual ~TTask();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal* Input, GenericSignal* Output );
};
#endif


