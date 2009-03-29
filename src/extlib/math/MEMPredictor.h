////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Update: 3/26/09 jawilson@cae.wisc.edu - A more efficient method for finding
//      the AR coefficients was implemented from:
//      N. Andersen. Comments on the performance of maximum entropy algorithms. 
//          Proceedings of the IEEE, 66(11):1581?1582, Nov. 1978.
// Description: This LinearPredictor implements the Maximum Entropy Method for
//     autoregressive spectral analysis adapted from Press et. al.
//     Numerical Recipes in C (chapter 13).
//
// (C) 2000-2009, BCI2000 Project
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

#ifdef __GNUC__
# define TYPENAME class
#else
# define TYPENAME typename
#endif // __GNUC__

template<typename T>
const TYPENAME Ratpoly<typename MEMPredictor<T>::Complex>&
MEMPredictor<T>::TransferFunction( const DataVector& inData ) const
{
  typedef double D;
  const T eps = std::numeric_limits<T>::epsilon();
  DataVector  coeff, wkm,
                     wk1, wk2;
  int n = inData.size();
  coeff.resize( LinearPredictor<T>::mModelOrder + 1 );
  wkm.resize( coeff.size() );
  wk1.resize( n );
  wk1 = inData;
  wk2.resize( n );
  wk2 = inData;

  D meanPower = 0;
  D den = 0;
  for (int t = 0; t < n; t++)
    meanPower += (wk1[t]*wk1[t]);
  
  den = meanPower*2;
  meanPower /= n;
  D num=0.0;
  D q = 1.0;
  coeff[0] = 1.0;
  for( int k = 1; k <= LinearPredictor<T>::mModelOrder; ++k )
  {
    num = 0;
    for (int t = 0; t < n-k; t++)
        num += wk1[t+1]*wk2[t];
    
    den = den*q - wk1[0]*wk1[0] - wk2[n-k]*wk2[n-k];
   
    coeff[k] = 2*num / den;
    if (fabs(coeff[k]) >= 1){
        den = 0;
        for (int t = 0; t < n-k; t++)
            den += wk1[t+1]*wk1[t+1] + wk2[t]*wk2[t];
        
        coeff[k] = 2*num/den;
    }
    q = 1.0 - coeff[k] * coeff[k];
    meanPower *= q;
    for( int i = 1; i < k; ++i )
      coeff[i] = wkm[i] - coeff[k] * wkm[k-i];

    if( k < LinearPredictor<T>::mModelOrder )
    {
      for( int i = 1; i <= k; ++i )
        wkm[i] = coeff[i];

      for( int j = 0; j < n-k; ++j )
      {
        wk1[j] = wk1[j+1] - wkm[k] * wk2[j];
        wk2[j] = wk2[j] - wkm[k] * wk1[j+1];
      }
    }
  }
  if( meanPower < 0.0 )
    meanPower = 0.0;
  
  for (int k = 1; k <= LinearPredictor<T>::mModelOrder; k++)
      coeff[k] *= -1;
 
  static Ratpoly<Complex> result;
  result = Ratpoly<Complex>(
             Polynomial<Complex>( std::sqrt( meanPower ) ),
             Polynomial<Complex>::FromCoefficients( coeff )
           );
  return result;
}

#undef TYPENAME

#endif // MEM_PREDICTOR_H

