//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A convenience header for numeric constants.
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
#ifndef TINY_NUMERIC_CONSTANTS_H
#define TINY_NUMERIC_CONSTANTS_H

#include <limits>
#include <cmath>
#include <cstring>
#include <iostream>

namespace Tiny
{

template<typename T>
const T& NaN( const T& = 0 )
{
  static const T nan = std::numeric_limits<T>::quiet_NaN();
  return nan;
}

template<typename T>
bool IsNaN( T t )
{
  return ::memcmp( &t, &NaN<T>(), sizeof( T ) ) == 0;
}

template<typename T>
const T& Inf( const T& = 0 )
{
  static const T inf = std::numeric_limits<T>::infinity();
  return inf;
}

template<typename T>
const T& Eps( const T& = 0 )
{
  static const T eps = std::numeric_limits<T>::epsilon();
  return eps;
}

#undef Pi
template<typename T>
const T& Pi( const T& = 0 )
{
  static const T pi = ::atan( T( 1 ) ) * 4;
  return pi;
}

inline const double& Pi()
{
  static const double pi = ::atan( double( 1 ) ) * 4;
  return pi;
}

template<typename T>
int Floor( T t )
{
  return static_cast<int>( ::floor( t ) );
}

template<typename T>
int Ceil( T t )
{
  return static_cast<int>( ::ceil( t ) );
}

template<typename T>
struct pretty_ { T t; };

template<typename T>
pretty_<T> Pretty( T t )
{
  pretty_<T> p = { t };
  return p;
};

} // namespace

template<typename T>
std::ostream&
operator<<( std::ostream& os, Tiny::pretty_<T> p )
{
  if( Tiny::IsNaN( p.t ) )
    os << "nan";
  else if( p.t == Tiny::Inf<T>() )
    os << "+inf";
  else if( p.t == -Tiny::Inf<T>() )
    os << "-inf";
  else
    os << p.t;
  return os;
}

using Tiny::NaN;
using Tiny::IsNaN;
using Tiny::Eps;
using Tiny::Inf;
using Tiny::Pi;
using Tiny::Floor;
using Tiny::Ceil;
using Tiny::Pretty;

#endif // TINY_NUMERIC_CONSTANTS_H
