//---------------------------------------------------------------------------

#ifndef FIRFilterH
#define FIRFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#include "getfir.h"

#define MAX_N  256
#define MAX_M  64

class TemporalFilter : public GenericFilter
{
private:
       int instance;
       int samples;               // dimension of data matrix
       int channels;              // dimension of data matrix
       int detrend;
       int integrate;             // integration of FIR result
       int hz;
       float coeff[MAX_M][MAX_N];   // matrix of coefficient weights
       int m_coef;                  // number of channels to filter
       int n_coef;                  // order of FIR filter
       float datwin[MAX_M][MAX_N];
       int winlgth;
       int datawindows;
       bool visualize;
       FIR *fir;
       GenericVisualization *vis;
public:
       int nPoints;
       TemporalFilter(PARAMLIST *plist, STATELIST *slist);
       TemporalFilter(PARAMLIST *plist, STATELIST *slist, int instance);
       ~TemporalFilter();
       int Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
       int Process(GenericSignal *Input, GenericSignal *Output);
};
#endif


