//---------------------------------------------------------------------------

#ifndef FIRClassFilterH
#define FIRClassFilterH
//---------------------------------------------------------------------------

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

class FIRClassFilter : public GenericFilter
{
private:
       static const int cNumControlSignals = 2;

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
       float mat_ud[MAX_M][MAX_N];     // filter kernal matrix
       float mat_lr[MAX_M][MAX_N];     // filter kernal matrix
       bool visualize;
       GenericVisualization *vis;

public:
          FIRClassFilter();
  virtual ~FIRClassFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif // FIRClassFilterH


