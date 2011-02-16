////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: This LinearPredictor implements the Maximum Entropy Method for
//     autoregressive spectral analysis adapted from Press et. al.
//     Numerical Recipes in C (chapter 13).
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
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

  virtual void TransferFunction(DataVector&, Ratpoly<Complex>& ) const;
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
void
MEMPredictor<T>::TransferFunction(DataVector& inData, Ratpoly<Complex>& outResult) const
{
  typedef double D;
  const T eps = std::numeric_limits<T>::epsilon();
  DataVector  coeff, wkm, wk1, wk2;
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

    if (den < eps){
      num = 0.5;
      den = 1.0;
    }
    else{
      if (coeff[k] >= 1 || coeff[k] <= -1){
        den = 0;
        for (int t = 0; t < n-k; t++)
          den += wk1[t+1]*wk1[t+1] + wk2[t]*wk2[t];
      }
    }
    coeff[k] = 2*num / den;

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
 
  outResult = Ratpoly<Complex>(
               Polynomial<Complex>( std::sqrt( meanPower ) ),
               Polynomial<Complex>::FromCoefficients( coeff )
              );
}

#undef TYPENAME

#endif // MEM_PREDICTOR_H

