////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        IIRFilter.h
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: An abstract base class that implements a direct form II
//              transposed IIR filter.
//              Subclasses will provide individual implementations for the
//              DesignFilter() member which translates parameter settings into
//              filter coefficients.
//
// $Log$
// Revision 1.2  2006/10/26 17:05:00  mellinger
// Rewrote IIR filter as a sequence of complex-valued first-order filters to improve numerical stability.
//
// Revision 1.1  2006/05/04 17:06:43  mellinger
// Initial revision.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef IIRFilterH
#define IIRFilterH

#include "UGenericFilter.h"
#include <deque>
#include <complex>

class IIRFilter : public GenericFilter
{
 typedef double                    real_type;
 typedef std::complex<real_type>   complex_type;
 typedef std::vector<complex_type> complex_vector;

 protected:
  IIRFilter();
  ~IIRFilter();

 public:
  void Preflight( const SignalProperties&, SignalProperties& ) const;
  void Initialize2( const SignalProperties&, const SignalProperties& );
  void StartRun();
  void Process( const GenericSignal*, GenericSignal* );

 private:
  // Translate user settings into a filter definition given by
  // - overall gain,
  // - complex roots of the numerator polynomial ("zeros"),
  // - complex roots of the denominator polynomial ("poles").
  virtual void DesignFilter( real_type& gain,
                             complex_vector& zeros,
                             complex_vector& poles ) const = 0;

 private:
  real_type                   mGain;
  complex_vector              mZeros,
                              mPoles;
  std::vector<complex_vector> mDelays;
};

#endif // IIRFilterH