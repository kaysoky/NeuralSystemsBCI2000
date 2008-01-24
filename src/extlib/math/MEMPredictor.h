////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: This LinearPredictor implements the Maximum Entropy Method for
//     autoregressive spectral analysis adapted from Press et. al.
//     Numerical Recipes in C (chapter 13).
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef MEM_PREDICTOR_H
#define MEM_PREDICTOR_H

#include "LinearPredictor.h"
#include <numeric>
#include <limits>

template<typename T>
class MEMPredictor : public LinearPredictor<T>
{
 public:
  typedef std::complex<T>  Complex;
  typedef std::valarray<T> DataVector;

 public:
  MEMPredictor();
  virtual ~MEMPredictor() {}

  virtual const Ratpoly<Complex>& TransferFunction( const DataVector& ) const;
};


// Implementation
template<typename T>
MEMPredictor<T>::MEMPredictor()
{
}

template<typename T>
const Ratpoly<MEMPredictor::Complex>&
MEMPredictor<T>::TransferFunction( const DataVector& inData ) const
{
  const T eps = std::numeric_limits<T>::epsilon();
  static DataVector  coeff, wkm,
                     wk1, wk2;
  int n = inData.size();
  coeff.resize( mModelOrder + 1 );
  wkm.resize( coeff.size() );
  wk1.resize( n );
  wk1 = inData;
  wk2.resize( n );
  wk2 = inData.shift( 1 );

  T meanPower = std::inner_product( &wk1[0], &wk1[n], &wk1[0], 0.0 ) / n;
  coeff[0] = 1.0;
  for( int k = 1; k <= mModelOrder; ++k )
  {
    T num   = ( n > k ) ?
                2.0 * std::inner_product( &wk1[0], &wk1[n-k], &wk2[0], 0.0 ) :
                0.0,
      denom = ( n > k ) ?
                std::inner_product( &wk1[0], &wk1[n-k], &wk1[0], 0.0 )
                + std::inner_product( &wk2[0], &wk2[n-k], &wk2[0], 0.0 ) :
                0.0;
    if( denom < eps )
    { // limit for zero data
      num = 1.0;
      denom = 1.0;
    }
    coeff[k] = - num / denom;
    meanPower *= 1.0 - coeff[k] * coeff[k];
    for( int i = 1; i < k; ++i )
      coeff[i] = wkm[i] + coeff[k] * wkm[k-i];

    if( k < mModelOrder )
    {
      for( int i = 1; i <= k; ++i )
        wkm[i] = coeff[i];

      for( int j = 0; j < n-k-1; ++j )
      {
        wk1[j] += wkm[k] * wk2[j];
        wk2[j] = wk2[j+1] + wkm[k] * wk1[j+1];
      }
    }
  }
  if( meanPower < 0.0 )
    meanPower = 0.0;
  static Ratpoly<Complex> result;
  result = Ratpoly<Complex>(
             Polynomial<Complex>( std::sqrt( meanPower ) ),
             Polynomial<Complex>::FromCoefficients( coeff )
           );
  return result;
}

#endif // MEM_PREDICTOR_H

