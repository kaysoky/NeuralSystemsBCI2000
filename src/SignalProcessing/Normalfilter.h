//---------------------------------------------------------------------------

#ifndef NormalFilterH
#define NormalFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"

class NormalFilter : public GenericFilter
{
private:
       float ud_a;
       float ud_b;
       float lr_a;
       float lr_b;
       bool visualize;
       class GenericVisualization *vis;
public:
          NormalFilter();
  virtual ~NormalFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
          int  UpdateParameters( float, float, float, float );
};
#endif


