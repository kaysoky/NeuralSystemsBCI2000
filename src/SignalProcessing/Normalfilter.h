//---------------------------------------------------------------------------

#ifndef NormalFilterH
#define NormalFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

class NormalFilter : public GenericFilter
{
private:
       int      instance;
       float ud_a;
       float ud_b;
       float lr_a;
       float lr_b;
       bool visualize;
       GenericVisualization *vis;
public:
       NormalFilter(PARAMLIST *plist, STATELIST *slist);
       NormalFilter(PARAMLIST *plist, STATELIST *slist, int instance);
  virtual ~NormalFilter();
  virtual void Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
       int UpdateParameters( float, float, float, float );
};
#endif


