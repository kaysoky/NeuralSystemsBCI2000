////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: A finite impulse response (FIR) filter for temporal filtering.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef FIR_FILTER_H
#define FIR_FILTER_H

#include "GenericFilter.h"
#include <valarray>
#include <vector>

class FIRFilter : public GenericFilter
{
 public:
          FIRFilter();
  virtual ~FIRFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal& Input, GenericSignal& Output );

 private:
  enum
  {
    none = 0,
    mean,
    rms,
    max,
  };
  int mFIRIntegration;

  typedef std::valarray<float> DataVector;
  std::vector<DataVector>      mFilter,
                               mBuffer;
};
#endif // FIR_FILTER_H


