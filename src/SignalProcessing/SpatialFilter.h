//---------------------------------------------------------------------------

#ifndef SpatialFilterH
#define SpatialFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#define MAX_N  128
#define MAX_M  128

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
  virtual ~SpatialFilter();
  virtual void Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif


