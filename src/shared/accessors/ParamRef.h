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
#include "MeasurementUnits.h"

class MutableParamRef;

class ParamRef
{
  struct IstreamRef
  {
    IstreamRef( const ParamRef& r )
      : p( new std::istringstream( r.ToString() ) ) {}
    IstreamRef( IstreamRef& r )
      : p( r.p ) { r.p = 0; }
    ~IstreamRef()
      { delete p; }
    template<typename T>IstreamRef& operator>>( T& t )
      { *p >> t; return *this; }
    std::istringstream* p;
  };

 public:
  enum { none = ~size_t( 0 ) };

 private:
  ParamRef();
  ParamRef& operator=( const ParamRef& );

 public:
  explicit ParamRef( const Param* param, size_t row = none, size_t column = none );
  ParamRef( const ParamRef& );
  ParamRef( const MutableParamRef& );
  
  bool IsNull() const
    { return mpParam == NULL; }

  double ToNumber() const;
  const std::string& ToString() const;
  const char* c_str() const;
  template<typename T> IstreamRef operator>>( T& t ) const
    { return IstreamRef( *this ) >> t; }

  // Conversion operators for read access.
  operator const std::string&() const
    { return ToString(); }
  operator double() const
    { return ToNumber(); }
  operator float() const
    { return static_cast<float>( ToNumber() ); }
  operator long double() const
    { return ToNumber(); }
  operator short() const
    { return static_cast<short>( ToNumber() ); }
  operator unsigned short() const
    { return static_cast<unsigned short>( ToNumber() ); }
  operator int() const
    { return static_cast<int>( ToNumber() ); }
  operator unsigned int() const
    { return static_cast<unsigned int>( ToNumber() ); }
  operator long() const
    { return static_cast<long>( ToNumber() ); }
  operator unsigned long() const
    { return static_cast<unsigned long>( ToNumber() ); }
  operator long long() const
    { return static_cast<long long>( ToNumber() ); }
  operator unsigned long long() const
    { return static_cast<unsigned long long>( ToNumber() ); }

  // We also need to override operators to avoid ambiguities
  // when the compiler resolves expressions.
  double operator-( const ParamRef& p ) const
    { return ToNumber() - p.ToNumber(); }
  double operator+( const ParamRef& p ) const
    { return ToNumber() + p.ToNumber(); }
  double operator*( const ParamRef& p ) const
    { return ToNumber() * p.ToNumber(); }
  double operator/( const ParamRef& p ) const
    { return ToNumber() / p.ToNumber(); }

  bool operator==( const ParamRef& p ) const
    { return ToString() == p.ToString(); }
  bool operator!=( const ParamRef& p ) const
    { return ToString() != p.ToString(); }
  bool operator<( const ParamRef& p ) const
    { return ToNumber() < p.ToNumber(); }
  bool operator>( const ParamRef& p ) const
    { return ToNumber() > p.ToNumber(); }
  bool operator<=( const ParamRef& p ) const
    { return ToNumber() <= p.ToNumber(); }
  bool operator>=( const ParamRef& p ) const
    { return ToNumber() >= p.ToNumber(); }

  // double
  double operator-( double d ) const
    { return ToNumber() - d; }
  double operator+( double d ) const
    { return ToNumber() + d; }
  double operator*( double d ) const
    { return ToNumber() * d; }
  double operator/( double d ) const
    { return ToNumber() / d; }

  bool operator==( double d ) const
    { return ToNumber() == d; }
  bool operator!=( double d ) const
    { return ToNumber() != d; }
  bool operator<( double d ) const
    { return ToNumber() < d; }
  bool operator>( double d ) const
    { return ToNumber() > d; }
  bool operator<=( double d ) const
    { return ToNumber() <= d; }
  bool operator>=( double d ) const
    { return ToNumber() >= d; }

#ifndef __BORLANDC__
  // float
  double operator-( float f ) const
    { return ToNumber() - f; }
  double operator+( float f ) const
    { return ToNumber() + f; }
  double operator*( float f ) const
    { return ToNumber() * f; }
  double operator/( float f ) const
    { return ToNumber() / f; }

  bool operator==( float f ) const
    { return ToNumber() == f; }
  bool operator!=( float f ) const
    { return ToNumber() != f; }
  bool operator<( float f ) const
    { return ToNumber() < f; }
  bool operator>( float f ) const
    { return ToNumber() > f; }
  bool operator<=( float f ) const
    { return ToNumber() <= f; }
  bool operator>=( float f ) const
    { return ToNumber() >= f; }

  // int
  double operator-( int i ) const
    { return ToNumber() - i; }
  double operator+( int i ) const
    { return ToNumber() + i; }
  double operator*( int i ) const
    { return ToNumber() * i; }
  double operator/( int i ) const
    { return ToNumber() / i; }

  bool operator==( int i ) const
    { return ToNumber() == i; }
  bool operator!=( int i ) const
    { return ToNumber() != i; }
  bool operator<( int i ) const
    { return ToNumber() < i; }
  bool operator>( int i ) const
    { return ToNumber() > i; }
  bool operator<=( int i ) const
    { return ToNumber() <= i; }
  bool operator>=( int i ) const
    { return ToNumber() >= i; }

  // unsigned int
  double operator-( unsigned int i ) const
    { return ToNumber() - i; }
  double operator+( unsigned int i ) const
    { return ToNumber() + i; }
  double operator*( unsigned int i ) const
    { return ToNumber() * i; }
  double operator/( unsigned int i ) const
    { return ToNumber() / i; }

  bool operator==( unsigned int i ) const
    { return ToNumber() == i; }
  bool operator!=( unsigned int i ) const
    { return ToNumber() != i; }
  bool operator<( unsigned int i ) const
    { return ToNumber() < i; }
  bool operator>( unsigned int i ) const
    { return ToNumber() > i; }
  bool operator<=( unsigned int i ) const
    { return ToNumber() <= i; }
  bool operator>=( unsigned int i ) const
    { return ToNumber() >= i; }
#endif // !__BORLANDC__

  bool operator==( const std::string& s ) const
    { return ToString() == s; }
  bool operator!=( const std::string& s ) const
    { return ToString() != s; }

  // Conversions involving units.
  // The following functions convert the parameter's value into the unit specified,
  // honoring physical units when present.
  double InSampleBlocks() const
    { return MeasurementUnits::TimeInSampleBlocks( ToString() ); }
  double InSeconds() const
    { return MeasurementUnits::TimeInSeconds( ToString() ); }
  double InMilliseconds() const
    { return MeasurementUnits::TimeInMilliseconds( ToString() ); }

  double InHertz() const
    { return MeasurementUnits::FreqInHertz( ToString() ); }

  double InVolts() const
    { return MeasurementUnits::VoltageInVolts( ToString() ); }
  double InMicrovolts() const
    { return MeasurementUnits::VoltageInMicrovolts( ToString() ); }


  // Dereferencing operators for access to Param members.
  const Param* operator->() const;
  // Indexing operators for sub-parameters.
  ParamRef operator()( size_t row, size_t col = none ) const;
  ParamRef operator()( size_t row, const std::string&  ) const;
  ParamRef operator()( const std::string&, size_t = none ) const;
  ParamRef operator()( const std::string&, const std::string& ) const;
  // Stream i/o.
  std::ostream& WriteToStream( std::ostream& os ) const;

 protected:
  int Idx1() const
    { return index( mIdx1 ); }
  int Idx2() const
    { return index( mIdx2 ); }
  const Param* Ptr() const
    { return mpParam; }

 private:
  static int index( int idx )
    { return idx == ParamRef::none ? 0 : idx; }

 private:
  const Param* mpParam;
  int mIdx1, mIdx2;

  static Param       sNullParam;
  static std::string sNullString;
};

class MutableParamRef : public ParamRef
{
  struct OstreamRef
  {
    OstreamRef( MutableParamRef& r )
      : r( r ), p( new std::ostringstream ) {}
    OstreamRef( OstreamRef& os )
      : r( os.r ), p( os.p ) { os.p = 0; }
    ~OstreamRef()
      { if( p ) { p->flush(); r = p->str(); delete p; } }
    template<typename T> OstreamRef& operator<<( const T& t )
      { *p << t; return *this; }
    std::ostringstream* p;
    MutableParamRef& r;
  };

 private:
  MutableParamRef( const ParamRef& p )
    : ParamRef( p ) {}

 public:
  explicit MutableParamRef( Param* param, size_t row = ParamRef::none, size_t column = ParamRef::none )
    : ParamRef( param, row, column ) {}

  // Assignment operators for write access.
  MutableParamRef& operator=( const std::string& );
  MutableParamRef& operator=( double );
  MutableParamRef& operator=( const Param& );
  template<typename T> OstreamRef operator<<( const T& t )
    { return OstreamRef( *this ) << t; }

  Param* operator->();
  MutableParamRef operator()( size_t row, size_t col = ParamRef::none ) const;
  MutableParamRef operator()( size_t row, const std::string&  ) const;
  MutableParamRef operator()( const std::string&, size_t = ParamRef::none ) const;
  MutableParamRef operator()( const std::string&, const std::string& ) const;

  std::istream& ReadFromStream( std::istream& is );
};

// double
inline double operator-( double d, const ParamRef& p )
  { return d - p.ToNumber(); }
inline double operator+( double d, const ParamRef& p )
  { return d + p.ToNumber(); }
inline double operator*( double d, const ParamRef& p )
  { return d * p.ToNumber(); }
inline double operator/( double d, const ParamRef& p )
  { return d / p.ToNumber(); }

inline bool operator==( double d, const ParamRef& p )
  { return d == p.ToNumber(); }
inline bool operator!=( double d, const ParamRef& p )
  { return d != p.ToNumber(); }
inline bool operator<( double d, const ParamRef& p )
  { return d < p.ToNumber(); }
inline bool operator>( double d, const ParamRef& p )
  { return d > p.ToNumber(); }
inline bool operator<=( double d, const ParamRef& p )
  { return d <= p.ToNumber(); }
inline bool operator>=( double d, const ParamRef& p )
  { return d >= p.ToNumber(); }

#ifndef __BORLANDC__
// float
inline double operator-( float f, const ParamRef& p )
  { return f - p.ToNumber(); }
inline double operator+( float f, const ParamRef& p )
  { return f + p.ToNumber(); }
inline double operator*( float f, const ParamRef& p )
  { return f * p.ToNumber(); }
inline double operator/( float f, const ParamRef& p )
  { return f / p.ToNumber(); }

inline bool operator==( float f, const ParamRef& p )
  { return f == p.ToNumber(); }
inline bool operator!=( float f, const ParamRef& p )
  { return f != p.ToNumber(); }
inline bool operator<( float f, const ParamRef& p )
  { return f < p.ToNumber(); }
inline bool operator>( float f, const ParamRef& p )
  { return f > p.ToNumber(); }
inline bool operator<=( float f, const ParamRef& p )
  { return f <= p.ToNumber(); }
inline bool operator>=( float f, const ParamRef& p )
  { return f >= p.ToNumber(); }

// int
inline double operator-( int i, const ParamRef& p )
  { return i - p.ToNumber(); }
inline double operator+( int i, const ParamRef& p )
  { return i + p.ToNumber(); }
inline double operator*( int i, const ParamRef& p )
  { return i * p.ToNumber(); }
inline double operator/( int i, const ParamRef& p )
  { return i / p.ToNumber(); }

inline bool operator==( int i, const ParamRef& p )
  { return i == p.ToNumber(); }
inline bool operator!=( int i, const ParamRef& p )
  { return i != p.ToNumber(); }
inline bool operator<( int i, const ParamRef& p )
  { return i < p.ToNumber(); }
inline bool operator>( int i, const ParamRef& p )
  { return i > p.ToNumber(); }
inline bool operator<=( int i, const ParamRef& p )
  { return i <= p.ToNumber(); }
inline bool operator>=( int i, const ParamRef& p )
  { return i >= p.ToNumber(); }

// unsigned int
inline double operator-( unsigned int i, const ParamRef& p )
  { return i - p.ToNumber(); }
inline double operator+( unsigned int i, const ParamRef& p )
  { return i + p.ToNumber(); }
inline double operator*( unsigned int i, const ParamRef& p )
  { return i * p.ToNumber(); }
inline double operator/( unsigned int i, const ParamRef& p )
  { return i / p.ToNumber(); }

inline bool operator==( unsigned int i, const ParamRef& p )
  { return i == p.ToNumber(); }
inline bool operator!=( unsigned int i, const ParamRef& p )
  { return i != p.ToNumber(); }
inline bool operator<( unsigned int i, const ParamRef& p )
  { return i < p.ToNumber(); }
inline bool operator>( unsigned int i, const ParamRef& p )
  { return i > p.ToNumber(); }
inline bool operator<=( unsigned int i, const ParamRef& p )
  { return i <= p.ToNumber(); }
inline bool operator>=( unsigned int i, const ParamRef& p )
  { return i >= p.ToNumber(); }
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
std::istream& operator>>( std::istream& s, MutableParamRef& t )
  { return t.ReadFromStream( s ); }

////////////////////////////////////////////////////////////////////////////////
// Definitions of inline functions
////////////////////////////////////////////////////////////////////////////////
inline
ParamRef::ParamRef( const Param* param, size_t row, size_t column )
: mpParam( param ), mIdx1( static_cast<int>( row ) ), mIdx2( static_cast<int>( column ) )
{
}

inline
ParamRef::ParamRef( const ParamRef& p )
: mpParam( p.mpParam ), mIdx1( p.mIdx1 ), mIdx2( p.mIdx2 )
{
}

inline
ParamRef::ParamRef( const MutableParamRef& p )
: mpParam( p.mpParam ), mIdx1( p.mIdx1 ), mIdx2( p.mIdx2 )
{
}

inline
const char*
ParamRef::c_str() const
{
  return ToString().c_str();
}


inline
const std::string&
ParamRef::ToString() const
{
  const std::string* result = &sNullString;
  if( mpParam )
  {
    const Param* p = mpParam;
    result = &p->Value( Idx1(), Idx2() ).ToString();
  }
  return *result;
}


inline const Param*
ParamRef::operator->() const
{
  const Param* result = mpParam;
  if( mpParam && ( mIdx1 != ParamRef::none || mIdx2 != ParamRef::none ) )
    result = mpParam->Value( Idx1(), Idx2() ).ToParam();
  if( result == NULL )
  {
    sNullParam = Param();
    result = &sNullParam;
  }
  return result;
}


inline ParamRef
ParamRef::operator()( size_t row, size_t col ) const
{
  return ParamRef( operator->(), row, col );
}


inline
std::ostream&
ParamRef::WriteToStream( std::ostream& os ) const
{
  os << ToString();
  return os;
}


inline Param*
MutableParamRef::operator->()
{
  return const_cast<Param*>( ParamRef::operator->() );
}


inline
MutableParamRef&
MutableParamRef::operator=( const std::string& s )
{
  if( !IsNull() )
    const_cast<Param*>( Ptr() )->Value( Idx1(), Idx2() ) = s;
  return *this;
}


inline
MutableParamRef&
MutableParamRef::operator=( double d )
{
  std::ostringstream os;
  os << d;
  return *this = os.str();
}

inline MutableParamRef
MutableParamRef::operator()( size_t row, size_t col ) const
{
  return ParamRef::operator()( row, col );
}

inline MutableParamRef
MutableParamRef::operator()( size_t row, const std::string& col ) const
{
  return ParamRef::operator()( row, col );
}

inline MutableParamRef
MutableParamRef::operator()( const std::string&  row, size_t col ) const
{
  return ParamRef::operator()( row, col );
}

inline MutableParamRef
MutableParamRef::operator()( const std::string& row, const std::string& col ) const
{
  return ParamRef::operator()( row, col );
}

inline
std::istream&
MutableParamRef::ReadFromStream( std::istream& is )
{
  std::string s;
  is >> s;
  *this = s;
  return is;
}

#endif // PARAM_REF_H

