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
       ~NormalFilter();
       int Initialize(PARAMLIST *plist, STATEVECTOR *statevector, CORECOMM *);
       int Process(GenericSignal *Input, GenericSignal *Output);
       int UpdateParameters( float, float, float, float );
};
#endif


