//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that holds references to parameters or
//         parameter values, and allows for convenient automatic type
//         conversions.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#ifndef PARAM_REF_H
#define PARAM_REF_H

#include <sstream>
#include <string>
#include <cmath>
#include "Param.h"


class ParamRef
{
 public:
  enum { none = -1 };

 private:
  ParamRef& operator=( const ParamRef& );

 public:
  ParamRef();
  ParamRef( Param* param, size_t row = none, size_t column = none );
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
    { return double( *this ); }
  operator long double() const
    { return double( *this ); }
  // We omit a conversion to char to avoid ambiguities with std::string assignment operators.
  operator short() const
    { return double( *this ); }
  operator unsigned short() const
    { return double( *this ); }
  operator int() const
    { return double( *this ); }
  operator unsigned int() const
    { return double( *this ); }
  operator long() const
    { return double( *this ); }
  operator unsigned long() const
    { return double( *this ); }
  operator long long() const
    { return double( *this ); }
  operator unsigned long long() const
    { return double( *this ); }

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

  bool operator==( const std::string& s ) const
    { return (const std::string&)( *this ) == s; }
  bool operator!=( const std::string& s ) const
    { return (const std::string&)( *this ) != s; }

  // Dereferencing operator for access to Param members.
  Param* operator->() const;
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
ParamRef::ParamRef()
: mpParam( NULL ), mIdx1( none ), mIdx2( none )
{
}

inline
ParamRef::ParamRef( Param* param, size_t row, size_t column )
: mpParam( param ), mIdx1( row ), mIdx2( column )
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
    result = &mpParam->Value( index( mIdx1 ), index( mIdx2 ) ).ToString();
  return *result;
}


inline Param*
ParamRef::operator->() const
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


inline ParamRef
ParamRef::operator()( size_t row, size_t col ) const
{
  return ParamRef( operator->(), row, col );
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
