/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef TaskH
#define TaskH

#include <stdio.h>

#include "GenericVisualization.h"
#include "GenericFilter.h"
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
        /*shidong starts*/
        //bool            alternatebackup;
        ///BYTE            currentbackuppos;
        //BYTE            GetCurrentBackupPos();
        FILE            *a;
        bool            debug;
        int             NumberTargets;
        int            IgnoreMistakes;

        bool            CheckTree(int  root) const;
        bool            checkInt(AnsiString input) const;
        /*shidong ends*/

	//VK
	bool		CheckTargetMatrix() const;
        
        AnsiString      DetermineNewResultText(AnsiString resulttext, AnsiString predicted);
        AnsiString      DetermineCurrentPrefix(AnsiString resulttext);
        AnsiString      DetermineDesiredWord(AnsiString resulttext, AnsiString spelledtext);
public:
          TTask();
  virtual ~TTask();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal& Input, GenericSignal& Output );
};
#endif
