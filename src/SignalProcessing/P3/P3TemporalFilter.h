//---------------------------------------------------------------------------

#ifndef TemporalFilterH
#define TemporalFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#include "getmem.h"

// #define MAX_N   72
// #define MAX_M  128

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
       int datawindows;         // number of data segments per spectra
       int detrend;
       int hz;

       float datwin[MAXDATA*8];    // data window buffer
       int winlgth;
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


