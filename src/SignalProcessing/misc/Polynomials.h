/////////////////////////////////////////////////////////////////////////////////
// $Id$
// File: Polynomials.h
// Description: Class templates for univariate polynomials and rational
//              expressions.
// $Log$
// Revision 1.3  2006/10/26 17:05:00  mellinger
// Rewrote IIR filter as a sequence of complex-valued first-order filters to improve numerical stability.
//
// Revision 1.2  2006/05/05 16:07:40  mellinger
// Added multiplication operators for Ratpoly class.
//
// Revision 1.1  2006/05/04 17:06:43  mellinger
// Initial revision.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////////
#ifndef POLYNOMIALS_H
#define POLYNOMIALS_H

#include <vector>
#include <algorithm>
#include "UBCIError.h"

template<class T>
class Polynomial
{
 private:
  Polynomial();
  Polynomial( const std::vector<T>& roots, const T& factor = 1 );

 public:
  static Polynomial FromRoots( const std::vector<T>& roots, const T& factor = 1 );

  Polynomial& operator*=( const T& );
  Polynomial& operator*=( const Polynomial& );
  template<class U> Polynomial operator*( const U& ) const;

  // Return a list of polynomial coefficients.
  const std::vector<T>& Coefficients() const;
  // Return a vector of roots.
  const std::vector<T>& Roots() const;
  const T& ConstantFactor() const;
  // Compute the polynomial's value for a given argument.
  T Evaluate( const T& ) const;

 private:
  T mConstantFactor;
  std::vector<T> mRoots;
  mutable std::vector<T> mCoefficients;
};

template<class T>
class Ratpoly // A rational expression with a polynomial numerator and denominator.
{
 public:
  Ratpoly( const T& = 1 );
  Ratpoly( const Polynomial<T>& numerator, const Polynomial<T>& denominator );

  Ratpoly& operator*=( const T& );
  Ratpoly& operator*=( const Polynomial<T>& );
  Ratpoly& operator*=( const Ratpoly& );
  template<class U> Ratpoly operator*( const U& ) const;

  const Polynomial<T>& Numerator() const;
  const Polynomial<T>& Denominator() const;
  T Evaluate( const T& ) const;

 private:
  void Simplify(); // remove cancelling factors

 private:
  Polynomial<T> mNumerator, mDenominator;
};


// Polynomial definitions


template<class T>
Polynomial<T>::Polynomial()
: mConstantFactor( 1 )
{
}

template<class T>
Polynomial<T>::Polynomial( const std::vector<T>& roots, const T& constantFactor )
: mConstantFactor( constantFactor ),
  mRoots( roots )
{
}

template<class T>
Polynomial<T>
Polynomial<T>::FromRoots( const std::vector<T>& roots, const T& constantFactor )
{
  return Polynomial<T>( roots, constantFactor );
}

template<class T>
T
Polynomial<T>::Evaluate( const T& z ) const
{
  T result = mConstantFactor;
  for( std::vector<T>::const_iterator i = mRoots.begin(); i != mRoots.end(); ++i )
    result *= ( z - *i );
  return result;
}

template<class T>
const std::vector<T>&
Polynomial<T>::Coefficients() const
{ // Compute coefficients by expanding the product of roots.
  if( mCoefficients.empty() )
  {
    mCoefficients.resize( 1, mConstantFactor );
    mCoefficients.resize( mRoots.size() + 1, 0 );
    /* one after one, multiply a factor of (z-mRoots[i]) into coeffs */
    for( std::vector<T>::const_iterator i = mRoots.begin(); i != mRoots.end(); ++i )
    {
      for( size_t j = mCoefficients.size() - 1; j >= 1; --j )
      {
        mCoefficients[ j ] *= -( *i );
        mCoefficients[ j ] += mCoefficients[ j - 1 ];
      }
      mCoefficients[ 0 ] *= -( *i );
    }
  }
  return mCoefficients;
}

template<class T>
const std::vector<T>&
Polynomial<T>::Roots() const
{
  return mRoots;
}

template<class T>
const T&
Polynomial<T>::ConstantFactor() const
{
  return mConstantFactor;
}

template<class T>
Polynomial<T>&
Polynomial<T>::operator*=( const T& f )
{
  mFactor *= f;
  mCoefficients.clear();
  return *this;
}

template<class T>
Polynomial<T>&
Polynomial<T>::operator*=( const Polynomial& p )
{
  mConstantFactor *= p.mConstantFactor;
  mRoots.insert( mRoots.end(), p.mRoots.begin(), p.mRoots.end() );
  mCoefficients.clear();
  return *this;
}

template<class T> template<class U>
Polynomial<T>
Polynomial<T>::operator*( const U& u ) const
{
  return Polynomial<T>( *this ) *= u;
}


// Ratpoly definitions

template<class T>
Ratpoly<T>::Ratpoly( const T& z )
: mNumerator( Polynomial<T>::FromRoots( std::vector<T>( 0 ), z ) ),
  mDenominator( Polynomial<T>::FromRoots( std::vector<T>( 0 ), 1 ) )
{
}

template<class T>
Ratpoly<T>::Ratpoly( const Polynomial<T>& numerator,
                     const Polynomial<T>& denominator )
: mNumerator( numerator ),
  mDenominator( denominator )
{
  Simplify();
}

template<class T>
Ratpoly<T>&
Ratpoly<T>::operator*=( const T& f )
{
  mNumerator *= f;
  return *this;
}

template<class T>
Ratpoly<T>&
Ratpoly<T>::operator*=( const Polynomial<T>& p )
{
  mNumerator *= p;
  Simplify();
  return *this;
}

template<class T>
Ratpoly<T>&
Ratpoly<T>::operator*=( const Ratpoly& r )
{
  mNumerator *= r.mNumerator;
  mDenominator *= r.mDenominator;
  Simplify();
  return *this;
}

template<class T> template<class U>
Ratpoly<T>
Ratpoly<T>::operator*( const U& u ) const
{
  return Ratpoly<T>( *this ) *= u;
}

template<class T>
const Polynomial<T>&
Ratpoly<T>::Numerator() const
{
  return mNumerator;
}

template<class T>
const Polynomial<T>&
Ratpoly<T>::Denominator() const
{
  return mDenominator;
}

template<class T>
T
Ratpoly<T>::Evaluate( const T& z ) const
{
  return mNumerator.Evaluate( z ) / mDenominator.Evaluate( z );
}

template<class T>
void
Ratpoly<T>::Simplify()
{
  std::vector<T> numerRoots = mNumerator.Roots(),
                 denomRoots = mDenominator.Roots(),
                 commonRoots;
  for( std::vector<T>::const_iterator i = numerRoots.begin(); i != numerRoots.end(); ++i )
  {
    std::vector<T>::iterator j = find( denomRoots.begin(), denomRoots.end(), *i );
    if( j != denomRoots.end() )
    {
      commonRoots.push_back( *i );
      denomRoots.erase( j );
    }
  }
  for( std::vector<T>::const_iterator i = commonRoots.begin(); i != commonRoots.end(); ++i )
    numerRoots.erase( find( numerRoots.begin(), numerRoots.end(), *i ) );

  if( !commonRoots.empty() )
  {
    mNumerator = Polynomial<T>::FromRoots( numerRoots, mNumerator.ConstantFactor() );
    mDenominator = Polynomial<T>::FromRoots( denomRoots, mDenominator.ConstantFactor() );
  }
}

#endif // POLYNOMIALS_H

