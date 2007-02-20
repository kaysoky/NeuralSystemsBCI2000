/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
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
          SpatialFilter();
  virtual ~SpatialFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};
#endif




