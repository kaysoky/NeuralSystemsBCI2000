//---------------------------------------------------------------------------

#ifndef SpatialFilterH
#define SpatialFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"

class SpatialFilter : public GenericFilter
{
private:
       enum
       {
#undef MAX_N
         MAX_N = 128,
#undef MAX_M
         MAX_M = 128,
       };
       int samples;
       int n_mat;                   // dimension of filter kernal = TransmitCh
       int m_mat;                   // dimension of filter kernal = output channels
       float mat[MAX_M][MAX_N];     // filter kernal matrix
       bool visualize;
       class GenericVisualization *vis;
public:
          SpatialFilter(PARAMLIST *plist, STATELIST *slist);
  virtual ~SpatialFilter();
  virtual void Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif


