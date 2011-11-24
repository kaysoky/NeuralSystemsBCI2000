////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: See the associated header file for documentation.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "ObserverBase.h"
#include "BCIException.h"
#include "BCIAssert.h"
#include "ClassName.h"

using namespace std;
using namespace bci;
using namespace StatisticalObserver;

// Independent function definitions
Matrix
StatisticalObserver::RSquared( const ObserverBase& inObs1, const ObserverBase& inObs2 )
{
  Number n1 = inObs1.Count(),
         n2 = inObs2.Count();
  if( n1 < eps || n2 < eps )
    throw bciexception( "Trying to compute r squared without observations" );

  Vector sum1 = inObs1.PowerSum1(),
         sum2 = inObs2.PowerSum1();
  Vector sqSum1 = inObs1.PowerSum2Diag(),
         sqSum2 = inObs2.PowerSum2Diag();

  Matrix result( sum1.size(), sum2.size() );
  for( size_t i = 0; i < sum1.size(); ++i )
    for( size_t j = 0; j < sum2.size(); ++j )
    {
      Number G = ( sum1[i] + sum2[j] ) * ( sum1[i] + sum2[j] ) / ( n1 + n2 ),
             numerator = sum1[i] * sum1[i] / n1 + sum2[j] * sum2[j] / n2 - G,
             denominator = sqSum1[i] + sqSum2[j] - G,
             rsq = 0;
      if( ::fabs( denominator ) > eps )
        rsq = ::max<Number>( numerator / denominator, 0 );
      result[i][j] = rsq;
    }
  return result;
}

Matrix
StatisticalObserver::ZScore( const ObserverBase& inDist, const ObserverBase& inRef )
{
  Vector mean( inDist.Mean() ),
         refMean( inRef.Mean() ),
         refStd( sqrt( inRef.Variance() ) );

  Matrix result( mean.size(), refMean.size() );
  for( size_t i = 0; i < mean.size(); ++i )
    for( size_t j = 0; j < refMean.size(); ++j )
      if( ::fabs( refStd[j] ) > eps )
        result[i][j] = ::fabs( mean[i] - refMean[j] ) / refStd[j];
  return result;
}

// ObserverBase definitions
ObserverBase::ObserverBase( int inConfig, int inSupported )
: mConfig( ImpliedConfig( inConfig ) ),
  mAge( 0 ),
  mWindowLength( Unlimited ),
  mDecayFactor( 1 ),
  mQuantileAccuracy( Auto ),
  mActualAccuracy( 0 ),
  mSampleSize( 0 )
{
  if( mConfig & ~ImpliedConfig( inSupported ) )
    throw bciexception( "Unsupported function requested" );
}

// Observations
ObserverBase&
ObserverBase::Observe( Number inN )
{
  mBuffer.resize( 1 );
  mBuffer[0] = inN;
  return Observe( mBuffer );
}

ObserverBase&
ObserverBase::Observe( const Vector& inV )
{
  if( mSampleSize == 0 )
  {
    mSampleSize = inV.size();
    DoChange();
  }
  if( inV.size() != mSampleSize )
    throw bciexception(
      "Observe(Vector) called with inconsistent sample sizes, expected: " << mSampleSize
      << ", got: " << inV.size()
    );
  AgeBy( 1 );
  DoObserve( inV, 1 );
  return *this;
}

ObserverBase&
ObserverBase::Observe( const Distribution& inD )
{
  if( mSampleSize == 0 && !inD.empty() )
  {
    mSampleSize = inD.front().first.size();
    DoChange();
  }
  for( Distribution::const_iterator i = inD.begin(); i != inD.end(); ++i )
    if( i->first.size() != mSampleSize )
      throw bciexception(
        "Observe(Distribution) called with inconsistent sample sizes, expected: " << mSampleSize
        << ", got: " << i->first.size()
      );
  AgeBy( 1 );
  for( Distribution::const_iterator i = inD.begin(); i != inD.end(); ++i )
    DoObserve( i->first, i->second );
  return *this;
}

ObserverBase&
ObserverBase::ObserveData( const Matrix& inM )
{
  for( size_t i = 0; i < inM.size(); ++i )
    Observe( inM[i] );
  return *this;
}

ObserverBase&
ObserverBase::ObserveHistograms( const Matrix& inWeights, const Vector& inValues )
{
  size_t numValues = inValues.size();
  Matrix values( inWeights.size() );
  for( size_t i = 0; i < inWeights.size(); ++i )
  {
    if( inWeights[i].size() != inValues.size() )
      throw bciexception(
        "Histogram length in Weights matrix differs from length of Values vector"
       );
    values[i] = inValues;
  }
  return ObserveHistograms( inWeights, values );
}

ObserverBase&
ObserverBase::ObserveHistograms( const Matrix& inWeights, const Matrix& inValues )
{
  if( inWeights.size() != inValues.size() )
    throw bciexception(
      "Weights matrix size (" << inWeights.size() << ")"
      " and Values matrix size (" << inValues.size() << ") disagree"
    );
  size_t numEntries = 0;
  for( size_t i = 0; i < inWeights.size(); ++i )
  {
    if( inWeights[i].size() != inValues[i].size() )
      throw bciexception(
        "Histogram length differs between Weights matrix, and Values matrix, in index " << i
      );
     numEntries += inWeights[i].size();
  }

  Distribution dist( numEntries );
  Distribution::iterator d = dist.begin();
  for( size_t i = 0; i < inWeights.size(); ++i )
  {
    for( size_t j = 0; j < inWeights[i].size(); ++j, ++d )
    {
      d->first.resize( inWeights.size(), Number( 0 ) );
      d->first[i] = inValues[i][j];
      d->second = inWeights[i][j];
    }
  }
  return Observe( dist );
}


ObserverBase&
ObserverBase::AgeBy( unsigned int inCount )
{
  DoAgeBy( inCount );
  mAge += inCount;
  return *this;
}

ObserverBase&
ObserverBase::Clear()
{
  mSampleSize = 0;
  mAge = 0;
  DoClear();
  return *this;
}

// Properties
ObserverBase&
ObserverBase::SetWindowLength( Number inLength )
{
  if( inLength != Unlimited && inLength < eps )
    throw bciexception( "Argument is " << inLength << ", must be > 0, or \"Unlimited\"" );
  mWindowLength = inLength;
  return Change();
}

ObserverBase&
ObserverBase::SetQuantileAccuracy( Number inAccuracy )
{
  if( inAccuracy != Auto && ( inAccuracy < 0 || inAccuracy > 1 ) )
    throw bciexception( "QuantileAccuracy is " << mQuantileAccuracy << ", must be in [0,1], or \"Auto\"" );
  mQuantileAccuracy = inAccuracy;
  Change();
  return *this;
}

ObserverBase&
ObserverBase::Change()
{
  if( WindowLength() > 0.5 + eps )
    mDecayFactor = ::exp( - 1 / ( WindowLength() - 0.5 ) );
  else
    mDecayFactor = 1;

  if( mQuantileAccuracy == Auto )
  {
    Number autoAccuracy = 0;
    if( WindowLength() > 0 )
      autoAccuracy = 1 / WindowLength();
    mActualAccuracy = ::max( autoAccuracy, QuantileAccuracyAutoLimit() );
  }
  else
  {
    mActualAccuracy = mQuantileAccuracy;
  }
  DoChange();
  return *this;
}

// Statistical results
#define REQUIRE( x )  \
  if( !( mConfig & StatisticalObserver::x ) ) \
    throw bciexception( ClassName( typeid( *this ) ) << " was not configured to compute function " << #x << "()" )

Number
ObserverBase::Count() const
{
  REQUIRE( Count );
  return PowerSum0();
}

Vector
ObserverBase::Mean() const
{
  REQUIRE( Mean );
  Number count = Count();
  Vector sum = PowerSum1();
  if( count < eps )
    throw bciexception( "Trying to compute mean without observation" );
  return Vector( sum / count );
}

Vector
ObserverBase::Variance() const
{
  REQUIRE( Variance );
  Vector mean = Mean(),
         sqMean = PowerSum2Diag();
  sqMean /= Count();
  return Vector( abs( sqMean - mean * mean ) );
}

Matrix
ObserverBase::Covariance() const
{
  REQUIRE( Covariance );
  Vector mean = Mean();
  Matrix result = PowerSum2Full();
  result /= Count();
  result -= mean.OuterProduct( mean );
  return result;
}

Matrix
ObserverBase::Correlation() const
{
  REQUIRE( Correlation );
  Matrix result = Covariance();
  for( size_t i = 0; i < result.size(); ++i )
  {
    Number varI = result[i][i];
    result[i][i] = 1;
    for( size_t j = i + 1; j < result.size(); ++j )
    {
      Number varJ = result[j][j];
      if( varI < eps && varJ < eps )
      { // For a pair of constant values, we return 0
        // (midway between perfect correlation, and perfect anticorrelation).
        result[i][j] = 0;
      }
      else
      { // If only one of the values is constant,
        // covariance will be zero, and we return zero correlation.
        if( varI >= eps )
          result[i][j] /= sqrt( varI );
        if( varJ >= eps )
          result[i][j] /= sqrt( varJ );
      }
      result[j][i] = result[i][j];
    }
  }
  return result;
}

Vector
ObserverBase::CentralMoment( unsigned int inN ) const
{
  REQUIRE( CentralMoment );
  switch( inN )
  {
    case 0:
      return Vector( Number( 1 ), SampleSize() );
    case 1:
      return Vector( Number( 0 ), SampleSize() );
    case 2:
      return Variance();
  }
  Vector result( 0, SampleSize() ),
         negativeMean( -Mean() ),
         meanPower( 1, SampleSize() );
  for( signed int j = inN; j >= 0; --j, meanPower *= negativeMean )
  {
    Number binomialCoeff = 1;
    for( signed int i = 1; i <= j; ++i )
      binomialCoeff *= ( inN - ( j - i ) ) / i;
    result += binomialCoeff * meanPower * PowerSumDiag( j ) / PowerSum0();
  }
  return result;
}

Vector
ObserverBase::Skewness() const
{
  REQUIRE( Skewness );
  Vector variance( Variance() ),
         denominator( sqrt( variance * variance * variance ) ),
         result( CentralMoment( 3 ) );
  for( size_t i = 0; i < result.size(); ++i )
  {
    if( denominator[i] < eps )
      result[i] = 0; // For zero variance, skewness is 0.
    else
      result[i] /= denominator[i];
  }
  return result;
}

Vector
ObserverBase::Kurtosis() const
{
  REQUIRE( Kurtosis );
  Vector variance( Variance() ),
         denominator( variance * variance ),
         result( CentralMoment( 4 ) );
  for( size_t i = 0; i < result.size(); ++i )
  {
    if( denominator[i] < eps )
      result[i] = 1; // The zero variance limit of the standardized 4th central moment is 1.
    else
      result[i] /= denominator[i];
    result[i] -= 3;
  }
  return result;
}

Vector
ObserverBase::Quantile( Number inP ) const
{
  REQUIRE( Quantile );
  if( inP < 0 || inP > 1 )
    throw bciexception( "Argument is " << inP << ", must be in [0,1]" );
  return InverseCDF( inP * PowerSum0() );
}

Matrix
ObserverBase::QQuantiles( unsigned int inQ ) const
{
  REQUIRE( QQuantiles );
  Vector quantile = Quantile( 0 );
  Matrix result( quantile.size(), inQ + 1 );
  for( size_t j = 0; j < result.size(); ++j )
    result[j][0] = quantile[j];
  for( size_t i = 1; i <= inQ; ++i )
  {
    quantile = Quantile( 1.0 * i / inQ );
    for( size_t j = 0; j < result.size(); ++j )
      result[j][i] = quantile[j];
  }
  return result;
}

Matrix
ObserverBase::Histogram( Number inCenter, Number inResolution, unsigned int inNumBins, Vector* outpEdges ) const
{
  REQUIRE( Histogram );
  Vector binEdges;
  if( inNumBins == 0 )
  {
    if( outpEdges )
      *outpEdges = binEdges;
    return Matrix( SampleSize() );
  }
  if( ::fabs( inResolution ) < eps )
    throw bciexception( "Resolution argument is " << inResolution << ", may not be 0" );
  binEdges.resize( inNumBins - 1 );
  for( size_t i = 0; i < inNumBins - 1; ++i )
    binEdges[i] = inResolution * ( i + 1 ) + inCenter - inResolution * inNumBins / 2;
  if( outpEdges )
    *outpEdges = binEdges;
  return Histogram( binEdges );
}

Matrix
ObserverBase::Histogram( const Vector& inBinEdges ) const
{
  REQUIRE( Histogram );
  Matrix result( SampleSize(), inBinEdges.size() + 1 );
  Vector leftCDF( SampleSize() );
  size_t bin = 0;
  while( bin < inBinEdges.size() )
  {
    Vector rightCDF( CDF( inBinEdges[bin] ) ),
           binValues( rightCDF - leftCDF );
    leftCDF = rightCDF;
    for( size_t i = 0; i < result.size(); ++i )
      result[i][bin] = binValues[i];
    ++bin;
  }
  for( size_t i = 0; i < result.size(); ++i )
    result[i][bin] = PowerSum0() - leftCDF[i];
  return result;
}

// Default implementations of computation functions
Vector
ObserverBase::PowerSum2Diag() const
{
  Matrix p = PowerSum2Full();
  for( size_t i = 1; i < p.size(); ++i )
  {
    bciassert( p[i].size() > i );
    p[0][i] = p[i][i];
  }
  return p[0];
}

#define UNSUPPORTED throw bciexception( ClassName( typeid( *this ) ) << " does not support this computation" )

Matrix
ObserverBase::PowerSum2Full() const
{
  UNSUPPORTED;
  return Matrix();
}

Vector
ObserverBase::PowerSumDiag( unsigned int i ) const
{
  switch( i )
  {
    case 0:
      return Vector( PowerSum0(), SampleSize() );
    case 1:
      return PowerSum1();
    case 2:
      return PowerSum2Diag();
  }
  UNSUPPORTED;
  return Vector();
}

Vector
ObserverBase::CDF( Number ) const
{
  UNSUPPORTED;
  return Vector();
}


Vector
ObserverBase::InverseCDF( Number ) const
{
  UNSUPPORTED;
  return Vector();
}

int
ObserverBase::ImpliedConfig( int inConfig )
{ // This function describes dependencies of ObserverBase
  // functions.
  const int groups[] =
  {
    StatisticalObserver::CentralMoment |
    StatisticalObserver::Skewness |
    StatisticalObserver::Kurtosis,

    StatisticalObserver::Correlation |
    StatisticalObserver::Covariance,

    StatisticalObserver::Quantile |
    StatisticalObserver::QQuantiles |
    StatisticalObserver::Histogram,
  };

  int config = inConfig | MinConfig;
  for( size_t i = 0; i < sizeof( groups ) / sizeof( *groups ); ++i )
    if( config & groups[i] )
      config |= groups[i];

  return config;
}


