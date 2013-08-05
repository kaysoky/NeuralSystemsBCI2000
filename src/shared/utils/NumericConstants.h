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
#ifndef NUMERIC_CONSTANTS_H
#define NUMERIC_CONSTANTS_H

#include <limits>
#include <cmath>
#include <cstring>

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
  static const T Eps = std::numeric_limits<T>::epsilon();
  return Eps;
}

template<typename T>
const T& Pi( const T& = 0 )
{
  static const T Pi = ::atan( T( 1 ) ) * 4;
  return Pi;
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

#endif // NUMERIC_CONSTANTS_H
