/////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Class templates for univariate polynomials and rational
//              expressions.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////////
#ifndef POLYNOMIALS_H
#define POLYNOMIALS_H

#include <vector>
#include <valarray>
#include <algorithm>

template<class T>
class Polynomial
{
 public:
  typedef std::vector<T> Vector;

 private:
  Polynomial( const Vector& roots, const T& factor );
  Polynomial( const Vector& coefficients );

 public:
  Polynomial( const T& );

  static Polynomial FromRoots( const Vector& roots, const T& factor = 1 );
  static Polynomial FromCoefficients( const Vector& coefficients );
  template<typename U> static Polynomial FromCoefficients( const std::valarray<U>& );

  Polynomial& operator*=( const T& );
  Polynomial& operator*=( const Polynomial& );
  template<class U> Polynomial operator*( const U& ) const;

  int Order() const;
  // Return a list of polynomial coefficients.
  const Vector& Coefficients() const;
  // Do we know the polynomial's roots?
  bool RootsKnown() const;
  // Return a list of roots.
  const Vector& Roots() const;
  const T& ConstantFactor() const;
  // Compute the polynomial's value for a given argument.
  T Evaluate( const T&, int derivative = 0 ) const;

 private:
  void FindRoots() const { throw __FUNC__ ": Root finding not implemented."; }

  mutable bool   mRootsKnown;     //
  mutable T      mConstantFactor; // These may change during a call to Roots().
  mutable Vector mRoots;          //

  mutable Vector mCoefficients;   // This may change during a call to Coefficients().
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
Polynomial<T>::Polynomial( const T& factor )
: mRootsKnown( true ),
  mConstantFactor( factor )
{
}

template<class T>
Polynomial<T>::Polynomial( const Vector& roots, const T& constantFactor )
: mRootsKnown( true ),
  mConstantFactor( constantFactor ),
  mRoots( roots )
{
}

template<class T>
Polynomial<T>::Polynomial( const Vector& coefficients )
: mRootsKnown( false ),
  mConstantFactor( 1 ),
  mCoefficients( coefficients )
{
  if( mCoefficients.size() == 1 )
  {
    mConstantFactor = mCoefficients[ 0 ];
    mRootsKnown = true;
  }
}

template<class T>
Polynomial<T>
Polynomial<T>::FromRoots( const Vector& roots, const T& constantFactor )
{
  return Polynomial<T>( roots, constantFactor );
}

template<class T>
Polynomial<T>
Polynomial<T>::FromCoefficients( const Vector& coefficients )
{
  return Polynomial<T>( coefficients );
}

template<class T> template<class U>
Polynomial<T>
Polynomial<T>::FromCoefficients( const std::valarray<U>& inCoefficients )
{
  Vector coefficients( inCoefficients.size() );
  for( size_t i = 0; i < inCoefficients.size(); ++i )
    coefficients[ i ] = inCoefficients[ i ];
  return Polynomial<T>( coefficients );
}

template<class T>
T
Polynomial<T>::Evaluate( const T& z, int d ) const
{
  typedef double D;
  D result = 0;
  if( mRootsKnown && d == 0 )
  {
    result = mConstantFactor;
    for( Vector::const_iterator i = mRoots.begin(); i != mRoots.end(); ++i )
      result *= ( z - *i );
  }
  else
  {
    Coefficients();
    D powerOfZ = 1;
    for( size_t i = 0; i < mCoefficients.size() - d; ++i, powerOfZ *= z )
      result += mCoefficients[ i + d ] * powerOfZ;
  }
  return result;
}

template<class T>
int
Polynomial<T>::Order() const
{
  return mRootsKnown ? mRoots.size() : mCoefficients.size() - 1;
}

template<class T>
const Polynomial<T>::Vector&
Polynomial<T>::Coefficients() const
{ // Compute coefficients by expanding the product of roots.
  if( mRootsKnown && mCoefficients.empty() )
  {
    mCoefficients = Vector( mRoots.size() + 1, 0 );
    mCoefficients[ 0 ] = 1;
    /* one after one, multiply a factor of (z-mRoots[i]) into coeffs */
    for( Vector::const_iterator i = mRoots.begin(); i != mRoots.end(); ++i )
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
bool Polynomial<T>::RootsKnown() const
{
  return mRootsKnown;
}

template<class T>
const Polynomial<T>::Vector&
Polynomial<T>::Roots() const
{
  if( !mRootsKnown )
    FindRoots();
  return mRoots;
}

template<class T>
const T&
Polynomial<T>::ConstantFactor() const
{
  if( !mRootsKnown )
    FindRoots();
  return mConstantFactor;
}

template<class T>
Polynomial<T>&
Polynomial<T>::operator*=( const T& f )
{
  if( mRootsKnown )
    mConstantFactor *= f;
  mCoefficients *= f;
  return *this;
}

template<class T>
Polynomial<T>&
Polynomial<T>::operator*=( const Polynomial& p )
{
  if( mRootsKnown && p.mRootsKnown )
  {
    mConstantFactor *= p.mConstantFactor;
    mRoots.insert( mRoots.end(), p.mRoots.begin(), p.mRoots.end() );
    mCoefficients = Vector( 0 );
  }
  else
  {
    Vector        coeff1 = this->Coefficients();
    const Vector& coeff2 = p.Coefficients();
    mRootsKnown = false;
    mRoots.clear();
    mCoefficients = Vector( coeff1.size() + coeff2.size() - 1, 0 );
    for( size_t i = 0; i < coeff1.size(); ++i )
      for( size_t j = 0; j < coeff2.size(); ++j )
        mCoefficients[ i + j ] += coeff1[ i ] * coeff2[ j ];
  }
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
: mNumerator( Polynomial<T>::FromRoots( Polynomial<T>::Vector(), z ) ),
  mDenominator( Polynomial<T>::FromRoots( Polynomial<T>::Vector(), 1 ) )
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
  const float eps = 1e-20;
  T num = mNumerator.Evaluate( z ),
    denom = mDenominator.Evaluate( z );
  int derivative = 0;
  while( abs( denom ) < eps && derivative <= mDenominator.Order() && abs( num ) < eps )
  {
    num = mNumerator.Evaluate( z, derivative );
    denom = mDenominator.Evaluate( z, derivative );
  }
  if( abs( denom ) < eps )
  {
    denom = 1;
    if( abs( num ) < eps )
      num = 1;
    else
      num = 1 / eps;
  }
  return num / denom;
}

template<class T>
void
Ratpoly<T>::Simplify()
{
  if( mNumerator.RootsKnown() && mDenominator.RootsKnown() )
  {
    Polynomial<T>::Vector numerRoots = mNumerator.Roots(),
                          denomRoots = mDenominator.Roots(),
                          commonRoots;
    for( Polynomial<T>::Vector::const_iterator i = numerRoots.begin(); i != numerRoots.end(); ++i )
    {
      Polynomial<T>::Vector::iterator j = find( denomRoots.begin(), denomRoots.end(), *i );
      if( j != denomRoots.end() )
      {
        commonRoots.push_back( *i );
        denomRoots.erase( j );
      }
    }
    for( Polynomial<T>::Vector::const_iterator i = commonRoots.begin(); i != commonRoots.end(); ++i )
      numerRoots.erase( find( numerRoots.begin(), numerRoots.end(), *i ) );

    if( !commonRoots.empty() )
    {
      mNumerator = Polynomial<T>::FromRoots( numerRoots, mNumerator.ConstantFactor() );
      mDenominator = Polynomial<T>::FromRoots( denomRoots, mDenominator.ConstantFactor() );
    }
  }
}

#endif // POLYNOMIALS_H

