////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: A notch filter for removing power line noise, and a high pass
//   collected into a single filter.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SOURCE_FILTER_H
#define SOURCE_FILTER_H

#include "IIRFilter.h"

class SourceFilter : public IIRFilter
{
 public:
  SourceFilter();
  ~SourceFilter() {}
  virtual bool AllowsVisualization() const { return false; }

 private:
  // Translate user settings into a filter definition given by
  // - overall gain,
  // - complex roots of the numerator polynomial ("zeros"),
  // - complex roots of the denominator polynomial ("poles").
  virtual void DesignFilter( Real& gain,
                             ComplexVector& zeros,
                             ComplexVector& poles ) const;
};

#endif // SOURCE_FILTER_H
