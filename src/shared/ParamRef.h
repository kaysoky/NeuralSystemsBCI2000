//////////////////////////////////////////////////////////////////////
// $Id$
//
// File: ParamRef.h
//
// Date: Oct 31, 2005
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// $Log$
// Revision 1.3  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
//
// Description: A class that holds references to parameters or
//         parameter values, and allows for convenient automatic type
//         conversions.
//
///////////////////////////////////////////////////////////////////////
#ifndef ParamRefH
#define ParamRefH

#include <sstream>
#include <math.h>
#include "UParameter.h"

class ParamRef
{
 public:
  enum { none = -1 };
  
 private:
  ParamRef& operator=( const ParamRef& );
 public:
  ParamRef();
  ParamRef( PARAM* param, size_t row, size_t column );
  // Assignment operators for write access.
  ParamRef& operator=( const char* s );
  ParamRef& operator=( double d );
  ParamRef& operator=( const PARAM& );
  // Conversion operators for read access.
  operator const char*() const;
  operator double() const;

  // We need to override operators to avoid ambiguities
  // when the compiler resolves expressions.
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

  // Dereferencing operator for access to PARAM members.
  PARAM* operator->() const;
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
  PARAM* mpParam;
  int    mIdx1, mIdx2;
  static PARAM  sNullParam;
};

inline
ParamRef::ParamRef()
: mpParam( NULL ), mIdx1( none ), mIdx2( none )
{
}

inline
ParamRef::ParamRef( PARAM* param, size_t row, size_t column )
: mpParam( param ), mIdx1( row ), mIdx2( column )
{
}

inline ParamRef&
ParamRef::operator=( const char* s )
{
  if( mpParam )
    mpParam->SetValue( s, index( mIdx1 ), index( mIdx2 ) );
  return *this;
}

inline
ParamRef&
ParamRef::operator=( double d )
{
  std::ostringstream os;
  os << d;
  if( mpParam )
    mpParam->SetValue( os.str().c_str(), index( mIdx1 ), index( mIdx2 ) );
  return *this;
}

inline
ParamRef::operator const char*() const
{
  const char* result = "";
  if( mpParam )
    result = mpParam->GetValue( index( mIdx1 ), index( mIdx2 ) );
  return result;
}

inline
ParamRef::operator double() const
{
  double result = 0.0;
  if( mpParam )
    result = ::atof( mpParam->GetValue( index( mIdx1 ), index( mIdx2 ) ) );
  return result;
}

inline PARAM*
ParamRef::operator->() const
{
  PARAM* result = mpParam;
  if( mpParam && ( mIdx1 != none || mIdx2 != none ) )
    result = mpParam->Value( index( mIdx1 ), index( mIdx2 ) ).ToParam();
  if( result == NULL )
  {
    sNullParam = PARAM();
    result = &sNullParam;
  }
  return result;
}

inline ParamRef
ParamRef::operator()( size_t row, size_t col ) const
{
  return ParamRef( operator->(), row, col );
}

inline std::ostream&
ParamRef::WriteToStream( std::ostream& os ) const
{
  os << operator const char*();
  return os;
}

inline std::istream&
ParamRef::ReadFromStream( std::istream& is )
{
  std::string s;
  is >> s;
  *this = s.c_str();
  return is;
}

inline std::ostream& operator<<( std::ostream& s, const ParamRef& t )
{
  return t.WriteToStream( s );
}

inline std::istream& operator>>( std::istream& s, ParamRef& t )
{
  return t.ReadFromStream( s );
}

#endif // ParamRefH