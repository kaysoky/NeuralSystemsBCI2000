/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef NormalFilterH
#define NormalFilterH
//---------------------------------------------------------------------------

#include "GenericFilter.h"

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
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal& Input, GenericSignal& Output );
          int  UpdateParameters( float, float, float, float );
};
#endif


