#ifndef TaskH
#define TaskH

#include <stdio.h>

#include "UBCITime.h"
#include "UCoreComm.h"
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
        int             DetermineCorrectTargetID();
        BCITIME         *cur_time;
        bool            alternatebackup;
        BYTE            currentbackuppos;
        BYTE            GetCurrentBackupPos();
        AnsiString      DetermineNewResultText(AnsiString resulttext, AnsiString predicted);
        AnsiString      DetermineCurrentPrefix(AnsiString resulttext);
        AnsiString      DetermineDesiredWord(AnsiString resulttext, AnsiString spelledtext);
public:
          TTask();
  virtual ~TTask();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const {}
  virtual void Initialize();
  virtual void Process( const GenericSignal* Input, GenericSignal* Output );
};
#endif
