/////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: IIR filter design classes that compute coefficents for
//       Butterworth, Chebyshev, or Resonator type digital filters.
//       Based on mkfilter.C written by
//         A.J. Fisher, University of York   <fisher@minster.york.ac.uk>
//         September 1992
//       Results have been tested against the mkfilter on-line version available at
//       http://www-users.cs.york.ac.uk/~fisher/mkfilter/
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////////
#ifndef FILTER_DESIGN_H
#define FILTER_DESIGN_H

#include "Polynomials.h"
#include <complex>
#include <vector>

namespace FilterDesign
{
  enum FilterCharacter
  {
    none,
    lowpass,
    highpass,
    bandpass,
    bandstop,
    allpass,
  };

  typedef double                 Real;
  typedef std::complex<Real>     Complex;
  typedef std::vector<Complex>   ComplexVector;

// Compute direct form II transposed structure filter coefficients from
// a transfer function.
// The list returned corresponds to the coefficient vector arguments of the
// Matlab filter() function.
// Coefficient output arguments must be of vector, deque, or list type.
  template<class T>
  void ComputeCoefficients( const Ratpoly<Complex>& TransferFunction,
                            T& outInputCoeffs, T& outOutputCoeffs );

 class Butterworth
 {
  public:
   Butterworth();
   // Filter design classes employ the "named parameter" idiom
   // to ensure readability of calling code.
   Butterworth& Order( int order );
   Butterworth& Lowpass( Real corner ); // in terms of the sampling rate
   Butterworth& Highpass( Real corner );
   Butterworth& Bandpass( Real corner1, Real corner2 );
   Butterworth& Bandstop( Real corner1, Real corner2 );
   // Filter coefficients are numerator and denominator polynomial coefficients
   // of the filter's rational transfer function.
   Ratpoly<Complex> TransferFunction() const;

  protected:
   virtual void S_Poles( ComplexVector& outPoles ) const;

  private:
   void Normalize( ComplexVector& ioPoles, ComplexVector& outZeros ) const;
   // Prewarping and bilinear transform allow for converting analog filters into
   // digital ones.
   static Real Prewarp( Real );
   static void BilinearTransform( ComplexVector& );

  protected:
   int mOrder;

  private:
   Real mCorner1, mCorner2;
   int  mCharacter;
 };

 class Chebyshev : public Butterworth
 {
  public:
   Chebyshev();
   Chebyshev& Ripple_dB( Real ripple_dB );

  private:
   virtual void S_Poles( ComplexVector& outPoles ) const;

  private:
   Real mRipple_dB;
 };

 class Resonator
 {
  public:
   Resonator();
   Resonator& QFactor( Real qFactor );
   Resonator& Bandpass( Real centerFreq ); // in terms of the sampling rate
   Resonator& Bandstop( Real centerFreq );
   Resonator& Allpass( Real centerFreq );
   // Filter coefficients are numerator and denominator polynomial coefficients
   // of the filter's rational transfer function.
   Ratpoly<Complex> TransferFunction() const;

  private:
   Real mCenterFreq,
        mQFactor;
   int  mCharacter;
 };

// Compute direct form II transposed structure filter coefficients from
// a transfer function.
// The list returned corresponds to the coefficient vector arguments of the
// Matlab filter() function.
template<class T>
void
ComputeCoefficients( const Ratpoly<Complex>& inTF,
                     T& outInputCoeffs, T& outOutputCoeffs )
{
  ComplexVector b = inTF.Numerator().Coefficients(),
                a = inTF.Denominator().Coefficients();
  outInputCoeffs.clear();
  outOutputCoeffs.clear();
  Real normFactor = a.rbegin()->real();
  for( ComplexVector::const_reverse_iterator i = b.rbegin(); i != b.rend(); ++i )
    outInputCoeffs.push_back( i->real() / normFactor );
  for( ComplexVector::const_reverse_iterator i = a.rbegin(); i != b.rend(); ++i )
    outOutputCoeffs.push_back( i->real() / normFactor );
}

}; // namespace FilterDesign



#endif // FILTER_DESIGN_H

