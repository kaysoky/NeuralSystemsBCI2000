////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An abstract base class for linear predictors.
//   The predictor's TransferFunction() member returns a rational transfer
//   function with the estimated coefficients in the denominator, and the
//   explained signal variance in the numerator.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef LINEAR_PREDICTOR_H
#define LINEAR_PREDICTOR_H

#include <valarray>
#include <complex>
#include "Polynomials.h"

template<typename T>
class LinearPredictor
{
 public:
  LinearPredictor()
    : mModelOrder( 1 )
    {}
  virtual ~LinearPredictor() {}

  LinearPredictor& SetModelOrder( int inOrder )
    { mModelOrder = inOrder; return *this; }
  int ModelOrder() const
    { return mModelOrder; }

  virtual const Ratpoly< std::complex<T> >& TransferFunction( const std::valarray<T>& ) const = 0;

 protected:
  int mModelOrder;
};

#endif // LINEAR_PREDICTOR_H

