#ifndef TaskH
#define TaskH

#include <vcl.h>
#include <ComCtrls.hpp>

#include <stdio.h>
#include "UBitRate.h"
#include "UCoreComm.h"
#include "UGenericVisualization.h"
#include "UGenericFilter.h"

#include <vcl/series.hpp>


class TTask
{
private:
        STATEVECTOR           *svect;
        GenericVisualization  *vis;
        CORECOMM        *corecomm;
        TApplication    *Applic;
        int             AcousticMode;
        int             MakeMusic(short controlsignal);
        TForm           *form;
        TProgressBar    *progressbar;
        TChart          *chart;
        TLineSeries     *series;
public:
        void Initialize(PARAMLIST *plist, STATEVECTOR *, CORECOMM *, TApplication *);
        void Process(short * );
        TTask(PARAMLIST *plist, STATELIST *slist);
        ~TTask( void );
} ;

#endif
