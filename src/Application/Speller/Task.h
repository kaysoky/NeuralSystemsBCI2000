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
        CORECOMM        *corecomm;
        int             Wx, Wy, Wxl, Wyl;
        STATEVECTOR     *statevector;
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
          TTask( PARAMLIST*, STATELIST* );
  virtual ~TTask();

  virtual void Initialize( PARAMLIST*, STATEVECTOR*, CORECOMM* = NULL );
  virtual void Process( const GenericSignal* Input, GenericSignal* Output );
};
#endif
