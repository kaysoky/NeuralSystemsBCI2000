//---------------------------------------------------------------------------

#ifndef NormalFilterH
#define NormalFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"

class NormalFilter : public GenericFilter
{
private:
       static const int cNumControlSignals = 2; 
       float ymean;
       float ygain;
       float xmean;
       float xgain;
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


