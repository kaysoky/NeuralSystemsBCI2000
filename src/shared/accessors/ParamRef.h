//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that holds references to parameters or
//         parameter values, and allows for convenient automatic type
//         conversions.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
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
///////////////////////////////////////////////////////////////////////
#ifndef PARAM_REF_H
#define PARAM_REF_H

#include <sstream>
#include <string>
#include <cmath>
#include <limits.h>
#include "Param.h"
#include "defines.h"
#include "MeasurementUnits.h"

class ParamRef
{
 public:
  enum { none = UINT_MAX / 2 };

 private:
  ParamRef();
  ParamRef& operator=( const ParamRef& );

 public:
  explicit ParamRef( Param* param, size_t row = none, size_t column = none );
  bool IsNull() const
    { return mpParam == NULL; }
  // Assignment operators for write access.
  ParamRef& operator=( const std::string& );
  ParamRef& operator=( double );
  ParamRef& operator=( const Param& );
  // Conversion operators for read access.
  operator const std::string&() const;
  const char* c_str() const;

  // To-number conversion operators.
  operator double() const;
  operator float() const
    { return static_cast<float>( double( *this ) ); }
  operator long double() const
    { return double( *this ); }
  // We omit a conversion to char to avoid ambiguities with std::string assignment operators.
  operator short() const
    { return static_cast<short>( double( *this ) ); }
  operator unsigned short() const
    { return static_cast<unsigned short>( double( *this ) ); }
  operator int() const
    { return static_cast<int>( double( *this ) ); }
  operator unsigned int() const
    { return static_cast<unsigned int>( double( *this ) ); }
  operator long() const
    { return static_cast<long>( double( *this ) ); }
  operator unsigned long() const
    { return static_cast<unsigned long>( double( *this ) ); }
  operator long long() const
    { return static_cast<long long>( double( *this ) ); }
  operator unsigned long long() const
    { return static_cast<unsigned long long>( double( *this ) ); }

  // We also need to override operators to avoid ambiguities
  // when the compiler resolves expressions.
  double operator-( const ParamRef& p ) const
    { return double( *this ) - double( p ); }
  double operator+( const ParamRef& p ) const
    { return double( *this ) + double( p ); }
  double operator*( const ParamRef& p ) const
    { return double( *this ) * double( p ); }
  double operator/( const ParamRef& p ) const
    { return double( *this ) / double( p ); }

  bool operator==( const ParamRef& p ) const
    { return (const std::string&)( *this ) == (const std::string&)( p ); }
  bool operator!=( const ParamRef& p ) const
    { return (const std::string&)( *this ) != (const std::string&)( p ); }
  bool operator<( const ParamRef& p ) const
    { return double( *this ) < double( p ); }
  bool operator>( const ParamRef& p ) const
    { return double( *this ) > double( p ); }
  bool operator<=( const ParamRef& p ) const
    { return double( *this ) <= double( p ); }
  bool operator>=( const ParamRef& p ) const
    { return double( *this ) >= double( p ); }

  // double
  double operator-( double d ) const
    { return double( *this ) - d; }
  double operator+( double d ) const
    { return double( *this ) + d; }
  double operator*( double d ) const
    { return double( *this ) * d; }
  double operator/( double d ) const
    { return double( *this ) / d; }

  bool operator==( double d ) const
    { return double( *this ) == d; }
  bool operator!=( double d ) const
    { return double( *this ) != d; }
  bool operator<( double d ) const
    { return double( *this ) < d; }
  bool operator>( double d ) const
    { return double( *this ) > d; }
  bool operator<=( double d ) const
    { return double( *this ) <= d; }
  bool operator>=( double d ) const
    { return double( *this ) >= d; }

#ifndef __BORLANDC__
  // float
  double operator-( float f ) const
    { return double( *this ) - f; }
  double operator+( float f ) const
    { return double( *this ) + f; }
  double operator*( float f ) const
    { return double( *this ) * f; }
  double operator/( float f ) const
    { return double( *this ) / f; }

  bool operator==( float f ) const
    { return double( *this ) == f; }
  bool operator!=( float f ) const
    { return double( *this ) != f; }
  bool operator<( float f ) const
    { return double( *this ) < f; }
  bool operator>( float f ) const
    { return double( *this ) > f; }
  bool operator<=( float f ) const
    { return double( *this ) <= f; }
  bool operator>=( float f ) const
    { return double( *this ) >= f; }

  // int
  double operator-( int i ) const
    { return double( *this ) - i; }
  double operator+( int i ) const
    { return double( *this ) + i; }
  double operator*( int i ) const
    { return double( *this ) * i; }
  double operator/( int i ) const
    { return double( *this ) / i; }

  bool operator==( int i ) const
    { return double( *this ) == i; }
  bool operator!=( int i ) const
    { return double( *this ) != i; }
  bool operator<( int i ) const
    { return double( *this ) < i; }
  bool operator>( int i ) const
    { return double( *this ) > i; }
  bool operator<=( int i ) const
    { return double( *this ) <= i; }
  bool operator>=( int i ) const
    { return double( *this ) >= i; }

  // unsigned int
  double operator-( unsigned int i ) const
    { return double( *this ) - i; }
  double operator+( unsigned int i ) const
    { return double( *this ) + i; }
  double operator*( unsigned int i ) const
    { return double( *this ) * i; }
  double operator/( unsigned int i ) const
    { return double( *this ) / i; }

  bool operator==( unsigned int i ) const
    { return double( *this ) == i; }
  bool operator!=( unsigned int i ) const
    { return double( *this ) != i; }
  bool operator<( unsigned int i ) const
    { return double( *this ) < i; }
  bool operator>( unsigned int i ) const
    { return double( *this ) > i; }
  bool operator<=( unsigned int i ) const
    { return double( *this ) <= i; }
  bool operator>=( unsigned int i ) const
    { return double( *this ) >= i; }
#endif // !__BORLANDC__

  bool operator==( const std::string& s ) const
    { return (const std::string&)( *this ) == s; }
  bool operator!=( const std::string& s ) const
    { return (const std::string&)( *this ) != s; }

  // Conversions involving units.
  // The following functions convert the parameter's value into the unit specified,
  // honoring physical units when present.
  double InSampleBlocks() const
    { return MeasurementUnits::TimeInSampleBlocks( (const std::string&)( *this ) ); }
  double InSeconds() const
    { return MeasurementUnits::TimeInSeconds( (const std::string&)( *this ) ); }
  double InMilliseconds() const
    { return MeasurementUnits::TimeInMilliseconds( (const std::string&)( *this ) ); }

  double InHertz() const
    { return MeasurementUnits::FreqInHertz( (const std::string&)( *this ) ); }

  double InVolts() const
    { return MeasurementUnits::VoltageInVolts( (const std::string&)( *this ) ); }
  double InMicrovolts() const
    { return MeasurementUnits::VoltageInMicrovolts( (const std::string&)( *this ) ); }


  // Dereferencing operators for access to Param members.
  Param* operator->();
  const Param* operator->() const;
  // Indexing operators for sub-parameters.
  ParamRef operator()( size_t row, size_t col = none ) const;
  ParamRef operator()( size_t row, const std::string&  ) const;
  ParamRef operator()( const std::string&, size_t = none ) const;
  ParamRef operator()( const std::string&, const std::string& ) const;
  // Stream i/o.
  std::ostream& WriteToStream( std::ostream& os ) const;
  std::istream& ReadFromStream( std::istream& is );

 private:
  static size_t index( int idx )
    { return idx == none ? 0 : idx; }

 private:
  Param* mpParam;
  int    mIdx1, mIdx2;

  static Param       sNullParam;
  static std::string sNullString;
};

// double
inline double operator-( double d, const ParamRef& p )
  { return d - double( p ); }
inline double operator+( double d, const ParamRef& p )
  { return d + double( p ); }
inline double operator*( double d, const ParamRef& p )
  { return d * double( p ); }
inline double operator/( double d, const ParamRef& p )
  { return d / double( p ); }

inline bool operator==( double d, const ParamRef& p )
  { return d == double( p ); }
inline bool operator!=( double d, const ParamRef& p )
  { return d != double( p ); }
inline bool operator<( double d, const ParamRef& p )
  { return d < double( p ); }
inline bool operator>( double d, const ParamRef& p )
  { return d > double( p ); }
inline bool operator<=( double d, const ParamRef& p )
  { return d <= double( p ); }
inline bool operator>=( double d, const ParamRef& p )
  { return d >= double( p ); }

#ifndef __BORLANDC__
// float
inline double operator-( float f, const ParamRef& p )
  { return f - double( p ); }
inline double operator+( float f, const ParamRef& p )
  { return f + double( p ); }
inline double operator*( float f, const ParamRef& p )
  { return f * double( p ); }
inline double operator/( float f, const ParamRef& p )
  { return f / double( p ); }

inline bool operator==( float f, const ParamRef& p )
  { return f == double( p ); }
inline bool operator!=( float f, const ParamRef& p )
  { return f != double( p ); }
inline bool operator<( float f, const ParamRef& p )
  { return f < double( p ); }
inline bool operator>( float f, const ParamRef& p )
  { return f > double( p ); }
inline bool operator<=( float f, const ParamRef& p )
  { return f <= double( p ); }
inline bool operator>=( float f, const ParamRef& p )
  { return f >= double( p ); }

// int
inline double operator-( int i, const ParamRef& p )
  { return i - double( p ); }
inline double operator+( int i, const ParamRef& p )
  { return i + double( p ); }
inline double operator*( int i, const ParamRef& p )
  { return i * double( p ); }
inline double operator/( int i, const ParamRef& p )
  { return i / double( p ); }

inline bool operator==( int i, const ParamRef& p )
  { return i == double( p ); }
inline bool operator!=( int i, const ParamRef& p )
  { return i != double( p ); }
inline bool operator<( int i, const ParamRef& p )
  { return i < double( p ); }
inline bool operator>( int i, const ParamRef& p )
  { return i > double( p ); }
inline bool operator<=( int i, const ParamRef& p )
  { return i <= double( p ); }
inline bool operator>=( int i, const ParamRef& p )
  { return i >= double( p ); }

// unsigned int
inline double operator-( unsigned int i, const ParamRef& p )
  { return i - double( p ); }
inline double operator+( unsigned int i, const ParamRef& p )
  { return i + double( p ); }
inline double operator*( unsigned int i, const ParamRef& p )
  { return i * double( p ); }
inline double operator/( unsigned int i, const ParamRef& p )
  { return i / double( p ); }

inline bool operator==( unsigned int i, const ParamRef& p )
  { return i == double( p ); }
inline bool operator!=( unsigned int i, const ParamRef& p )
  { return i != double( p ); }
inline bool operator<( unsigned int i, const ParamRef& p )
  { return i < double( p ); }
inline bool operator>( unsigned int i, const ParamRef& p )
  { return i > double( p ); }
inline bool operator<=( unsigned int i, const ParamRef& p )
  { return i <= double( p ); }
inline bool operator>=( unsigned int i, const ParamRef& p )
  { return i >= double( p ); }
#endif // !__BORLANDC__

inline bool operator==( const std::string& s, const ParamRef& p )
  { return p == s; }
inline bool operator!=( const std::string& s, const ParamRef& p )
  { return p != s; }

template<class T>
T& operator-=( T& d, const ParamRef& p )
  { return d -= T( p ); }
template<class T>
T& operator+=( T& d, const ParamRef& p )
  { return d += T( p ); }
template<class T>
T& operator*=( T& d, const ParamRef& p )
  { return d *= T( p ); }
template<class T>
T& operator/=( T& d, const ParamRef& p )
  { return d /= T( p ); }

inline
std::ostream& operator<<( std::ostream& s, const ParamRef& t )
  { return t.WriteToStream( s ); }

inline
std::istream& operator>>( std::istream& s, ParamRef& t )
  { return t.ReadFromStream( s ); }

////////////////////////////////////////////////////////////////////////////////
// Definitions of inline functions
////////////////////////////////////////////////////////////////////////////////
inline
ParamRef::ParamRef( Param* param, size_t row, size_t column )
: mpParam( param ), mIdx1( static_cast<int>( row ) ), mIdx2( static_cast<int>( column ) )
{
}

inline
ParamRef&
ParamRef::operator=( const std::string& s )
{
  if( mpParam )
    mpParam->Value( index( mIdx1 ), index( mIdx2 ) ) = s;
  return *this;
}


inline
ParamRef&
ParamRef::operator=( double d )
{
  std::ostringstream os;
  os << d;
  if( mpParam )
    mpParam->Value( index( mIdx1 ), index( mIdx2 ) ) = os.str();
  return *this;
}


inline
const char*
ParamRef::c_str() const
{
  return this->operator const std::string&().c_str();
}


inline
ParamRef::operator const std::string&() const
{
  const std::string* result = &sNullString;
  if( mpParam )
  {
    const Param* p = mpParam;
    result = &p->Value( index( mIdx1 ), index( mIdx2 ) ).ToString();
  }
  return *result;
}


inline Param*
ParamRef::operator->()
{
  Param* result = mpParam;
  if( mpParam && ( mIdx1 != none || mIdx2 != none ) )
    result = mpParam->Value( index( mIdx1 ), index( mIdx2 ) ).ToParam();
  if( result == NULL )
  {
    sNullParam = Param();
    result = &sNullParam;
  }
  return result;
}


inline const Param*
ParamRef::operator->() const
{
  return const_cast<ParamRef*>( this )->operator->();
}


inline ParamRef
ParamRef::operator()( size_t row, size_t col ) const
{
  return ParamRef( const_cast<Param*>( operator->() ), row, col );
}


inline
std::ostream&
ParamRef::WriteToStream( std::ostream& os ) const
{
  os << operator const std::string&();
  return os;
}


inline
std::istream&
ParamRef::ReadFromStream( std::istream& is )
{
  std::string s;
  is >> s;
  *this = s;
  return is;
}

#endif // PARAM_REF_H

