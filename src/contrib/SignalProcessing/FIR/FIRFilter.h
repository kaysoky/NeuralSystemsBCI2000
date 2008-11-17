/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef FIRFilterH
#define FIRFilterH
//---------------------------------------------------------------------------

#include "GenericFilter.h"
#include "GenericVisualization.h"

#include "getfir.h"

class FIRFilter : public GenericFilter
{
private:
       enum
       {
         MAX_N = 256,
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
       int nPoints;

public:
          FIRFilter();
  virtual ~FIRFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process(const GenericSignal& Input, GenericSignal& Output);
};
#endif


