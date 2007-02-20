/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef ARFilterH
#define ARFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#include "getmem.h"

class ARTemporalFilter : public GenericFilter
{
 private:
  enum
  {
#undef MAX_N
   MAX_N = 256,
#undef MAX_M
   MAX_M = 256,
  };
  int instance;
  int samples;               // dimension of data matrix
  int channels;              // dimension of data matrix
  float start;
  float stop;
  float delta;
  float bandwidth;
  int modelorder;
  int detrend;
  int hz;
  float datwin[MAX_M][MAX_N*8];
  int winlgth;
  int datawindows;
  bool visualize;
  MEM *mem;
  GenericVisualization *vis;
  int nBins;

public:
          ARTemporalFilter();
  virtual ~ARTemporalFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif




