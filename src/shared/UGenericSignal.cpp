////////////////////////////////////////////////////////////////////////////////
//
// File: UGenericSignal.cpp
//
// Description: This file declares a SignalProperties base class and a BasicSignal
//   class template deriving from it with the signal's numerical type as the
//   template argument.
//   Two classes, GenericSignal and GenericIntSignal, are derived from a float
//   and int instantiation of this template. With a compatibility flag set
//   (SIGNAL_BACK_COMPAT) existing code should compile with minimal changes.
//
//   For the future, the following name transitions might be considered:
//     BasicSignal --> GenericSignal
//     GenericSignal --> FloatSignal
//     GenericIntSignal --> IntSignal
//   as the latter two don't have anything generic about them any more.
//
// Changes: June 28, 2002, juergen.mellinger@uni-tuebingen.de
//          - Rewrote classes from scratch but kept old class interface.
//          Mar 28, 2003, juergen.mellinger@uni-tuebingen.de
//          - Added depth member to SignalProperties to unify GenericSignal
//            and GenericIntSignal into one signal type.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UGenericSignal.h"

#include "defines.h" // for DATATYPE::FLOAT / DATATYPE::INTEGER
#include <iostream>
#include <iomanip>
#include <math.h>

// This will go into a numtypes header some day.
typedef unsigned char  uint8;
typedef signed char    sint8;
typedef unsigned short uint16;
typedef signed short   sint16;
typedef unsigned int   uint32;
typedef signed int     sint32;

template<typename T>
inline
std::ostream& _put_int( std::ostream& os, const T& x )
{
  // This will result in little endian encoding regardless of the target
  // system's byte ordering.
  T t = x;
  for( int i = 0; i < sizeof( T ); ++i )
  {
    os.put( t & 0xff );
    t >>= 8;
  }
  return os;
}

// This will result in an error for types not considered.
template<typename T>
std::ostream& _put_is_not_implemented_for_type();

template<typename T>
inline
std::ostream& put( std::ostream& os, const T& x )
{
  return std::numeric_limits<T>::is_integer ?
           _put_int( os, x ) :
           _put_is_not_implemented_for_type<T>();
}

template<typename T>
inline
std::istream& _get_int( std::istream& is, T& x )
{
  x = 0;
  for( int i = 0; i < sizeof( T ); ++i )
  {
    x |= ( T( is.get() ) << ( i * 8 ) );
  }
  return is;
}

// This will result in an error for types not considered.
template<typename T>
std::istream& _get_is_not_implemented_for_type();

template<typename T>
inline
std::istream& get( std::istream& is, T& x )
{
  return std::numeric_limits<T>::is_integer ?
           _get_int( is, x ) :
           _get_is_not_implemented_for_type<T>();
}

bool
SignalProperties::SetNumElements( size_t inChannel, size_t inElements )
{
  bool elementsTooBig = ( inElements > maxElements );
  if( elementsTooBig )
    elements.at( inChannel ) = maxElements;
  else
    elements.at( inChannel ) = inElements;
  return elementsTooBig;
}

void
SignalProperties::WriteToStream( std::ostream& os ) const
{
  os << Channels() << " " << MaxElements() << " " << GetDepth();
}

std::ostream&
SignalProperties::WriteBinary( std::ostream& os ) const
{
  uint32 channels = Channels(),
         maxElem = MaxElements(),
         depth = GetDepth();
  put( os, channels );
  put( os, maxElem );
  put( os, depth );
  return os;
}

std::istream&
SignalProperties::ReadBinary( std::istream& is )
{
  uint32 channels, maxElem, depth;
  get( is, channels );
  get( is, maxElem );
  get( is, depth );
  *this = SignalProperties( channels, maxElem, depth );
  return is;
}

bool
SignalProperties::operator>=( const SignalProperties& sp ) const
{
  if( elements.size() < sp.elements.size() )
    return false;
  for( size_t i = 0; i < sp.elements.size(); ++i )
    if( elements[ i ] < sp.elements[ i ] )
      return false;
  return true;
}

bool
SignalProperties::operator<=( const SignalProperties& sp ) const
{
  if( sp.elements.size() < elements.size() )
    return false;
  for( size_t i = 0; i < elements.size(); ++i )
    if( sp.elements[ i ] < elements[ i ] )
      return false;
  return true;
}

void
GenericSignal::SetChannel( const short *inSource, size_t inChannel )
{
  for( size_t i = 0; i < elements.at( inChannel ); ++i )
    Value[ inChannel ][ i ] = ( float )inSource[ i ];
}

const GenericSignal&
GenericSignal::operator=( const GenericIntSignal& inRHS )
{
  SetProperties( inRHS );
  for( size_t i = 0; i < inRHS.Channels(); ++i )
    for( size_t j = 0; j < inRHS.GetNumElements( i ); ++j )
      SetValue( i, j, ( float )inRHS.GetValue( i, j ) );
  return *this;
}

void
GenericSignal::WriteToStream( std::ostream& os ) const
{
  int indent = os.width();
  os << '\n' << std::setw( indent ) << ""
     << "SignalProperties { ";
  SignalProperties::WriteToStream( os );
  os << '\n' << std::setw( indent ) << ""
     << "}";
  os << std::setprecision( 7 );
  for( size_t j = 0; j < MaxElements(); ++j )
  {
    os << '\n' << std::setw( indent ) << "";
    for( size_t i = 0; i < Value.size(); ++i )
    {
      os << std::setw( 14 );
      if( j < GetNumElements( i ) )
        os << GetValue( i, j );
      else
        os << "n/a";
      os << ' ';
    }
  }
}

std::ostream&
GenericSignal::WriteBinary( std::ostream& os ) const
{
  uint8 datatype = ( depth >= sizeof( float ) ? DATATYPE::FLOAT : DATATYPE::INTEGER );
  put( os, uint8( datatype ) );
  put( os, uint8( Channels() ) );
  put( os, uint16( MaxElements() ) );
  switch( datatype )
  {
    case DATATYPE::INTEGER:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < MaxElements(); ++j )
        {
          sint16 value = 0;
          if( j < GetNumElements( i ) )
            value = GetValue( i, j );
          put( os, value );
        }
      break;

    case DATATYPE::FLOAT:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < MaxElements(); ++j )
        {
          float value = 0.0;
          if( j < GetNumElements( i ) )
            value = GetValue( i, j );
          int mantissa,
              exponent;
          if( value == 0.0 )
          {
            mantissa = 0;
            exponent = 1;
          }
          else
          {
            exponent = ceil( log10( fabs( value ) ) );
            mantissa = ( value / pow10( exponent ) ) * 10000;
            exponent -= 4;
          }
          put( os, sint16( mantissa ) );
          put( os, sint8( exponent ) );
        }
      break;
      
    default:
      os.setstate( os.failbit );
  }
  return os;
}

std::istream&
GenericSignal::ReadBinary( std::istream& is )
{
  uint8  datatype,
         channels;
  uint16 maxElem;
  get( is, datatype );
  get( is, channels );
  get( is, maxElem );
  switch( datatype )
  {
    case DATATYPE::INTEGER:
      SetProperties( SignalProperties( channels, maxElem, 2 ) );
      for( int i = 0; i < channels; ++i )
        for( int j = 0; j < maxElem; ++j )
        {
          sint16 value;
          get( is, value );
          SetValue( i, j, value );
        }
      break;

    case DATATYPE::FLOAT:
      SetProperties( SignalProperties( channels, maxElem, sizeof( float ) ) );
      for( int i = 0; i < channels; ++i )
        for( int j = 0; j < maxElem; ++j )
        {
          sint16 mantissa;
          sint8  exponent;
          get( is, mantissa );
          get( is, exponent );
          SetValue( i, j, mantissa * pow10( exponent ) );
        }
      break;
      
    default:
      is.setstate( is.failbit );
  }
  return is;
}


