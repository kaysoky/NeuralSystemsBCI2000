////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        NotchFilter.h
// Description: A notch filter for removing power line noise.
// $Log$
// Revision 1.1  2006/05/04 17:06:43  mellinger
// Initial revision.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NotchFilterH
#define NotchFilterH

#include "IIRFilter.h"

class NotchFilter : public IIRFilter
{
 public:
  NotchFilter();
  ~NotchFilter() {}

 private:
  // Translate user settings into filter coefficients.
  virtual void DesignFilter( num_seq_type& inputCoeff,
                             num_seq_type& outputCoeff ) const;
};

#endif // NotchFilterH