////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that evaluates rational transfer functions on the unit
//   circle in small intervals, and determines spectral amplitude by collecting
//   power into bins.
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
#ifndef TRANSFER_SPECTRUM_H
#define TRANSFER_SPECTRUM_H

#include <valarray>
#include <complex>
#include <cmath>
#include "Polynomials.h"

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif

template<typename T>
class TransferSpectrum
{
 public:
  TransferSpectrum()
  : mFirstBinCenter( 0 ),
    mBinWidth( 1 ),
    mNumBins( 0 ),
    mEvaluationsPerBin( 1 )
  {}
  // Configuration
  //  Center of first and last bin in units of sampling rate
  TransferSpectrum& SetFirstBinCenter( double d )
                    { mFirstBinCenter = d; return Init(); }
  double            FirstBinCenter() const
                    { return mFirstBinCenter; }
  //  Bin width in terms of sampling rate
  TransferSpectrum& SetBinWidth( double d )
                    { mBinWidth = d; return Init(); }
  double            BinWidth() const
                    { return mBinWidth; }
  //  Number of bins
  TransferSpectrum& SetNumBins( int n )
                    { mNumBins = n; return Init(); }
  int               NumBins() const
                    { return mNumBins; }
  //  Number of uniformly spaced function evaluations per bin
  TransferSpectrum& SetEvaluationsPerBin( int n )
                    { mEvaluationsPerBin = n; return Init(); }
  int               EvaluationsPerBin() const
                    { return mEvaluationsPerBin; }

  // Processing
  const TransferSpectrum& Evaluate( const Ratpoly< std::complex<T> >&, std::valarray<T>& ) const;

 private:
  TransferSpectrum& Init();

  double  mFirstBinCenter,
          mBinWidth;
  int     mNumBins,
          mEvaluationsPerBin;
  std::valarray< std::complex<T> > mLookupTable;
};


// Implementation
template<typename T>
TransferSpectrum<T>&
TransferSpectrum<T>::Init()
{
  mLookupTable.resize( mNumBins * mEvaluationsPerBin );
  for( int bin = 0; bin < mNumBins; ++bin )
  {
    double binBegin = mFirstBinCenter + mBinWidth * ( ( 1.0 / mEvaluationsPerBin - 1.0 ) / 2.0 + bin );
    for( int sample = 0; sample < mEvaluationsPerBin; ++sample )
    {
      double theta = 2.0 * M_PI * ( binBegin + ( mBinWidth * sample ) / mEvaluationsPerBin );
      mLookupTable[ mEvaluationsPerBin * bin + sample ] = std::polar<T>( 1.0, theta );
    }
  }
  return *this;
}


template<typename T>
const TransferSpectrum<T>&
TransferSpectrum<T>::Evaluate( const Ratpoly< std::complex<T> >& inFunction, std::valarray<T>& outResult ) const
{
  outResult.resize( mNumBins );
  for( int bin = 0; bin < mNumBins; ++bin )
  {
    outResult[bin] = 0.0;
    for( int sample = 0; sample < mEvaluationsPerBin; ++sample )
    {
      std::complex<double> value
        = inFunction.Evaluate( mLookupTable[ mEvaluationsPerBin * bin + sample ] );
      outResult[ bin ] += value.real() * value.real() + value.imag() * value.imag();
    }
  }
  outResult /= mEvaluationsPerBin;
  return *this;
}

#endif // TRANSFER_SPECTRUM_H

