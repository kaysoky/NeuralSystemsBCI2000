//---------------------------------------------------------------------------

#ifndef ClassFilterH
#define ClassFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#define MAX_N  128
#define MAX_M  128

class ClassFilter : public GenericFilter
{
private:
       int      instance;
       int samples;
       int n_mat;                   // dimension of filter kernal = TransmitCh
       int m_mat;                   // dimension of filter kernal = output channels
       float mat_ud[MAX_M][MAX_N];  // additive filter kernal matrix
       float xat_ud[MAX_M][MAX_N];  // multiplicative filter matrix
       float mat_lr[MAX_M][MAX_N];  // filter kernal matrix
       bool visualize;
       GenericVisualization *vis;
public:
       ClassFilter(PARAMLIST *plist, STATELIST *slist);
       ClassFilter(PARAMLIST *plist, STATELIST *slist, int instance);
       ~ClassFilter();
       int Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *, int);
       int Process(GenericSignal *Input, GenericSignal *Output);
};
#endif


