////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that evaluates rational transfer functions on the unit
//   circle in small intervals, and determines spectral amplitude by collecting
//   power into bins.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TRANSFER_SPECTRUM_H
#define TRANSFER_SPECTRUM_H

#include <valarray>
#include <complex>
#include "Polynomials.h"

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
  const std::valarray<T>& Evaluate( const Ratpoly< std::complex<T> >& ) const;

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
const std::valarray<T>&
TransferSpectrum<T>::Evaluate( const Ratpoly< std::complex<T> >& inFunction ) const
{
  static std::valarray<T> result( 0 );
  result.resize( mNumBins );
  result = 0.0;
  for( int bin = 0; bin < mNumBins; ++bin )
  {
    for( int sample = 0; sample < mEvaluationsPerBin; ++sample )
    {
      std::complex<double> value
        = inFunction.Evaluate( mLookupTable[ mEvaluationsPerBin * bin + sample ] );
      result[ bin ] += value.real() * value.real() + value.imag() * value.imag();
    }
  }
  result /= mEvaluationsPerBin;
  return result;
}

#endif // TRANSFER_SPECTRUM_H

