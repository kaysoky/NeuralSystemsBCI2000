////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: Linear detrending functions.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef DETREND_H
#define DETREND_H

#include <valarray>
#include <algorithm>
#include <numeric>

namespace Detrend
{
  template<typename T>
   static const std::valarray<T>& MeanDetrend( const std::valarray<T>& );
  template<typename T>
   static const std::valarray<T>& LinearDetrend( const std::valarray<T>& );
}

// Implementation
template<typename T>
const std::valarray<T>&
Detrend::MeanDetrend( const std::valarray<T>& inData )
{
  static std::valarray<T> result;
  result.resize( inData.size(), 0.0 );
  if( inData.size() > 0 )
    result = inData - inData.sum() / inData.size();
  return result;
}

template<typename T>
const std::valarray<T>&
Detrend::LinearDetrend( const std::valarray<T>& inData )
{
  static std::valarray<T> result;
  size_t n = inData.size();
  result.resize( n );
  result = inData;
  if( n > 0 )
  {
    static std::valarray<T> linbuf; // a buffer that holds [0:length(inData)-1] (as spelled in Matlab notation)
    if( linbuf.size() != n )
    {
      linbuf.resize( n );
      for( size_t i = 0; i < n; ++i )
        linbuf[ i ] = i;
    }
    T x2 = ( ( 2 * n - 1 ) * ( n - 1 ) * n ) / 6,
      xy = std::inner_product( &linbuf[0], &linbuf[n], &result[0], 0.0 ),
       x = n * ( n - 1 ) / 2,
       y = inData.sum(),
       b = ( xy - x * y / n ) / ( x2 - ( x * x  / n ) ),
       a = ( y - b * x ) / n;
    result -= b * linbuf + a;
  }
  return result;
}

#endif // DETREND_H
