//---------------------------------------------------------------------------

#ifndef FIRFilterH
#define FIRFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#include "getfir.h"

class TemporalFilter : public GenericFilter
{
private:
       enum
       {
#undef MAX_N
         MAX_N = 256,
#undef MAX_M
         MAX_M = 64,
       };
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
  virtual ~TemporalFilter();
  virtual void Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif


