////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        NotchFilter.h
// Description: A notch filter for removing power line noise.
// $Log$
// Revision 1.2  2006/10/26 17:05:00  mellinger
// Rewrote IIR filter as a sequence of complex-valued first-order filters to improve numerical stability.
//
// Revision 1.1  2006/05/04 17:06:43  mellinger
// Initial revision.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
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
  // Translate user settings into a filter definition given by
  // - overall gain,
  // - complex roots of the numerator polynomial ("zeros"),
  // - complex roots of the denominator polynomial ("poles").
  virtual void DesignFilter( real_type& gain,
                             complex_vector& zeros,
                             complex_vector& poles ) const;
};

#endif // NotchFilterH
