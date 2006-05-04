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
// Revision 1.1  2006/05/04 17:06:43  mellinger
// Initial revision.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef IIRFilterH
#define IIRFilterH

#include "UGenericFilter.h"
#include <deque>

class IIRFilter : public GenericFilter
{
 typedef double                num_type;
 typedef std::deque<num_type>  num_seq_type;

 protected:
  IIRFilter();
  ~IIRFilter();

 public:
  void Preflight( const SignalProperties&, SignalProperties& ) const;
  void Initialize2( const SignalProperties&, const SignalProperties& );
  void StartRun();
  void Process( const GenericSignal*, GenericSignal* );

 private:
  // Translate user settings into filter coefficients.
  // Filter coefficient semantics are identical to the Matlab filter() function:
  // outputCoeff[0] will be normalized to 1, and represents the coefficient
  // associated with the non-delayed output signal.
  virtual void DesignFilter( num_seq_type& inputCoeff,
                             num_seq_type& outputCoeff ) const = 0;

 private:
  num_seq_type              mInputCoeffs,
                            mOutputCoeffs;
  std::vector<num_seq_type> mDelayedInput,
                            mDelayedOutput;
};

#endif // IIRFilterH