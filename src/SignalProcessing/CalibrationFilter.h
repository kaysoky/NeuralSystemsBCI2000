//---------------------------------------------------------------------------

#ifndef CalibrationFilterH
#define CalibrationFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"

class CalibrationFilter : public GenericFilter
{
private:
       enum
       {
#undef MAXCHAN
         MAXCHAN = 72,
       };
       bool     align;
       bool     visualize;
       float    *offset, *gain;
       int      recordedChans;
       int      transmittedChans;
       float    delta;
       float    w1[MAXCHAN];
       float    w2[MAXCHAN];
       float    old[MAXCHAN];
       class GenericVisualization *vis;
public:
          CalibrationFilter(PARAMLIST *plist, STATELIST *slist);
  virtual ~CalibrationFilter();
  virtual void Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *corecomm);
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif


