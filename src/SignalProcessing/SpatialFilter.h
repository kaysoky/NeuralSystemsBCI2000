//---------------------------------------------------------------------------

#ifndef SpatialFilterH
#define SpatialFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#define MAX_N  256
#define MAX_M  256

class SpatialFilter : public GenericFilter
{
private:
       int      instance;
       int samples;
       int n_mat;                   // dimension of filter kernal = TransmitCh
       int m_mat;                   // dimension of filter kernal = output channels
       float mat[MAX_M][MAX_N];     // filter kernal matrix
       bool visualize;
       GenericVisualization *vis;
public:
       SpatialFilter(PARAMLIST *plist, STATELIST *slist);
       SpatialFilter(PARAMLIST *plist, STATELIST *slist, int instance);
       ~SpatialFilter();
       int Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
       int Process(GenericSignal *Input, GenericSignal *Output);
};
#endif


