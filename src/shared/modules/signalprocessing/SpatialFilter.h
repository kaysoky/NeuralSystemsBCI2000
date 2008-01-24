////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The SpatialFilter computes a linear transformation of its
//   input signal, given by a matrix-valued parameter.
//   In this matrix, input channels correspond to columns, and output channels
//   to rows.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SPATIAL_FILTER_H
#define SPATIAL_FILTER_H

#include "GenericFilter.h"

class SpatialFilter : public GenericFilter
{
 public:
          SpatialFilter();
  virtual ~SpatialFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal& Input, GenericSignal& Output );

 private:
  std::vector<std::vector<double> > mFilterMatrix;
};

#endif // SPATIAL_FILTER_H


