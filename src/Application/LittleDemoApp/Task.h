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


class TTask : public GenericFilter
{
 private:
        STATEVECTOR           *svect;
        GenericVisualization  *vis;
        int             AcousticMode;
        int             MakeMusic(short controlsignal);
        TForm           *form;
        TProgressBar    *progressbar;
        TChart          *chart;
        TLineSeries     *series;

 public:
          TTask( PARAMLIST*, STATELIST* );
  virtual ~TTask();

  virtual void Initialize( PARAMLIST*, STATEVECTOR*, CORECOMM* );
  virtual void Process( const GenericSignal* Input, GenericSignal* Output );
};

#endif
