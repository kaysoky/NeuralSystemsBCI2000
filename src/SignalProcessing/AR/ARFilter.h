//---------------------------------------------------------------------------

#ifndef ARFilterH
#define ARFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#include "getmem.h"

#define MAX_N  128
#define MAX_M  128

class TemporalFilter : public GenericFilter
{
private:
       int instance;
       int samples;               // dimension of data matrix
       int channels;              // dimension of data matrix
       float start;
       float stop;
       float delta;
       float bandwidth;
       int modelorder;
       int detrend;
       int hz;
       float datwin[MAX_M][MAX_N*8];
       int winlgth;
       int datawindows;
       bool visualize;
       MEM *mem;
       GenericVisualization *vis;
public:
       int nBins;
       TemporalFilter(PARAMLIST *plist, STATELIST *slist);
       TemporalFilter(PARAMLIST *plist, STATELIST *slist, int instance);
       ~TemporalFilter();
       int Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
       int Process(GenericSignal *Input, GenericSignal *Output);
};
#endif


