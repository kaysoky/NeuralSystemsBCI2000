/*************************************************************************
Task.h is the header file for the Right Justified Boxes task
*************************************************************************/
#ifndef TaskH
#define TaskH

#include <vcl.h>
#include <ComCtrls.hpp>

#include <stdio.h>
#include "UBitRate.h"
#include "UCoreComm.h"
#include "UGenericVisualization.h"
#include "UGenericFilter.h"
#include "UBCItime.h"


class TTask
{
private:
        STATEVECTOR           *svect;
        GenericVisualization  *vis;
        CORECOMM        *corecomm;
        BCITIME         *bcitime;
        TApplication    *Applic;
        int             AcousticMode;
        int             MakeMusic(short controlsignal);
        TForm           *form;
        TProgressBar    *progressbar;
public:
        void Initialize(PARAMLIST *plist, STATEVECTOR *, CORECOMM *, TApplication *);
        void Process(short * );
        TTask(PARAMLIST *plist, STATELIST *slist);
        ~TTask( void );
} ;

#endif
