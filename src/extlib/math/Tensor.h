////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A template for tensors of arbitrary rank, templated for a
//   numeric type.
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
#ifndef TENSOR_H
#define TENSOR_H

#include <valarray>
#include <iostream>

namespace Tensor
{

template<int rank, typename D = double>
class Tensor : public std::valarray< Tensor<rank-1, D> >
{
 public:
  typedef D Number;
  typedef std::valarray< Tensor<rank-1, D> > ArrayType;

  enum { Rank = rank };

  template<typename T>
  Tensor( const T& inT )
  : ArrayType( inT )
  {
  }

  Tensor( size_t s1 = 0, size_t s2 = 0, size_t s3 = 0, size_t s4 = 0 )
  : ArrayType( Tensor<rank-1, D>( s2, s3, s4 ), s1 )
  {
  }

  Tensor& operator=( const Tensor& inT )
  {
    return operator=<Tensor>( inT );
  }

  template<typename T>
  Tensor& operator=( const T& inT )
  {
    resize( inT.size() ); // valarray assignment does not resize
    ArrayType::operator=( inT );
    return *this;
  }

#if 0
  Tensor& operator*=( Number inN )
  {
    for( size_t i = 0; i < this->size(); ++i )
      ( *this )[i] *= inN;
    return *this;
  }

  Tensor& operator/=( Number inN )
  {
    return operator*=( 1.0 / inN );
  }
#endif

  template<int inRank, typename inD>
  Tensor<rank+inRank, D> OuterProduct( const Tensor<inRank, inD>& inT ) const
  {
    Tensor<rank+inRank, D> result( this->size() );
    for( size_t i = 0; i < this->size(); ++i )
      result[i] = ( *this )[i].OuterProduct( inT );
    return result;
  }

  Tensor<2*rank, D> OuterProduct( const ArrayType& inT ) const
  {
    return OuterProduct( Tensor( inT ) );
  }
};

template<typename D>
class Tensor<1, D> : public std::valarray<D>
{
 public:
  typedef std::valarray<D> ArrayType;
  typedef D Number;

  enum { Rank = 1 };

  template<typename T>
  Tensor( const T& inT )
  : ArrayType( inT )
  {
  }

  Tensor( Number inN, size_t inSize )
  : ArrayType( inN, inSize )
  {
  }

  Tensor( size_t inSize = 0, size_t = 0, size_t = 0, size_t = 0 )
  : ArrayType( inSize )
  {
  }

  Tensor& operator=( const Tensor& inT )
  {
    return operator=<Tensor>( inT );
  }

  template<typename T>
  Tensor& operator=( const T& inT )
  {
    resize( inT.size() ); // valarray assignment does not resize
    ArrayType::operator=( inT );
    return *this;
  }

  operator ArrayType() const
  {
    return *this;
  }

  template<int inRank, typename inD>
  Tensor<inRank+1, D> OuterProduct( const Tensor<inRank, inD>& inT ) const
  {
    Tensor<inRank+1, D> result( this->size() );
    for( size_t i = 0; i < result.size(); ++i )
      result[i] = inT * ( *this )[i];
    return result;
  }

  Tensor<2, D> OuterProduct( const ArrayType& inT ) const
  {
    return OuterProduct( Tensor( inT ) );
  }
};

template<typename D>
class Tensor<0, D>
{
 public:
  typedef D Number;

  enum { Rank = 0 };

  Tensor( Number inN )
  : mNumber( inN )
  {
  }

  operator Number() const
  {
    return mNumber;
  }

  template<class T>
  T OuterProduct( const T& inT ) const
  {
    return inT * mNumber;
  }

 private:
  Number mNumber;
};

template<int rank, typename D>
std::ostream&
operator<<( std::ostream& os, const Tensor<rank, D>& inT )
{
  os << "{ ";
  for( size_t i = 0; i < inT.size(); ++i )
    os << inT[i] << " ";
  os << "}";
  return os;
}

template<typename D>
std::ostream&
operator<<( std::ostream& os, const Tensor<0, D>& inT )
{
  return os << inT;
}



} // namespace Tensor

#endif // TENSOR_H
