/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef FIRFilterH
#define FIRFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#include "getfir.h"

class FIRFilter : public GenericFilter
{
private:
       enum
       {
#undef MAX_N
         MAX_N = 256,
#undef MAX_M
         MAX_M = 64,
       };
       int samples;               // dimension of data matrix
       int channels;              // dimension of data matrix
       int detrend;
       int integrate;             // integration of FIR result
       int hz;
       float coeff[MAX_M][MAX_N];   // matrix of coefficient weights
       int m_coef;                  // number of channels to filter
       int n_coef;                  // order of FIR filter
       float datwin[MAX_M][MAX_N];
       int winlgth;
       int datawindows;
       bool visualize;
       FIR *fir;
       GenericVisualization *vis;
       int nPoints;

public:
          FIRFilter();
  virtual ~FIRFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif




