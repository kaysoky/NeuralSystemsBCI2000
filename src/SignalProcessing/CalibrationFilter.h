//---------------------------------------------------------------------------

#ifndef CalibrationFilterH
#define CalibrationFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#define MAXCHAN 72

class CalibrationFilter : public GenericFilter
{
private:
       bool     align;
       bool     visualize;
       float    *offset, *gain;
       int      recordedChans;
       int      transmittedChans;
       float    delta;
       float    w1[MAXCHAN];
       float    w2[MAXCHAN];
       float    old[MAXCHAN];
       int      instance;
       GenericVisualization *vis;
public:
       CalibrationFilter(PARAMLIST *plist, STATELIST *slist);
       CalibrationFilter(PARAMLIST *plist, STATELIST *slist, int instance);
       ~CalibrationFilter();
       int Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *corecomm);
       int Process(GenericSignal *Input, GenericSignal *Output);
};
#endif


