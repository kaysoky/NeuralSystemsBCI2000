//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Classes that allow for platform-independent reading
//   and writing of binary data.
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
#ifndef BINARY_DATA_H
#define BINARY_DATA_H

#include <iostream>
#include <limits>

#if _WIN32
# define LITTLE_ENDIAN 1
# define BIG_ENDIAN 2
# define BYTE_ORDER LITTLE_ENDIAN
#else
# include <sys/param.h>
# ifndef BYTE_ORDER
#  define BYTE_ORDER __BYTE_ORDER
#  define LITTLE_ENDIAN __LITTLE_ENDIAN
#  define BIG_ENDIAN __BIG_ENDIAN
# endif
#endif

namespace bci {

enum
{
  LittleEndian = LITTLE_ENDIAN,
  BigEndian = BIG_ENDIAN,
  HostOrder = BYTE_ORDER,
};

template<bool HostMatchesData> struct BinaryIO;
template<> struct BinaryIO<true>
{
  template<typename T> static
  std::istream& Get( std::istream& is, T& t )
  {
    union { T* t; char* c; } p = { &t };
    return is.read( p.c, sizeof( T ) );
  }
  template<typename T> static
  std::ostream& Put( std::ostream& os, const T& t )
  {
    union { const T* t; const char* c; } p = { &t };
    return os.write( p.c, sizeof( T ) );
  }
};
template<> struct BinaryIO<false>
{
  template<typename T> static
  std::istream& Get( std::istream& is, T& t )
  {
    union { T* t; char* c; } p = { &t };
    char* q = p.c + sizeof( T );
    while( q > p.c )
      is.get( *--q );
    return is;
  }
  template<typename T> static
  std::ostream& Put( std::ostream& os, const T& t )
  {
    union { const T* t; const char* c; } p = { &t };
    char* q = p.c + sizeof( T );
    while( q > p.c )
      os.put( *--q );
    return os;
  }
};

template<bool b> struct ErrorIf;
template<> struct ErrorIf<false> {};
template<> struct ErrorIf<true> { private: ErrorIf(); /* Intentional compiler error, results from unsuited type argument */ };

template<typename T, int DataByteOrder> class BinaryData
{
 public:
  typedef T Type;

  BinaryData() {}
  BinaryData( T t ) : mData( t ) {}
  BinaryData( std::istream& is ) { Get( is ); }
  operator T() const { return mData; }
  static size_t Size() { return sizeof( T ); }
  std::istream& Get( std::istream& is ) { return BinaryIO<DataByteOrder == HostOrder>::Get( is, mData ); }
  std::ostream& Put( std::ostream& os ) const { return BinaryIO<DataByteOrder == HostOrder>::Put( os, mData ); }

 private:
  T mData;

  // Avoid instantiation for unsuited types T:
  enum
  {
    IsElementaryNumericType = std::numeric_limits<T>::is_specialized,
    IsFloat = std::numeric_limits<T>::is_specialized && !std::numeric_limits<T>::is_integer,
    IsStandardFloat = IsFloat && std::numeric_limits<T>::is_iec559,
  };
  ErrorIf<!IsElementaryNumericType> mError1;
  ErrorIf<IsFloat && !IsStandardFloat> mError2;
};

} // namespace

using bci::BinaryData;

#endif // BINARY_DATA_H
