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

#include "defines.h" // for DATATYPE_FLOAT / DATATYPE_INTEGER
#include <iostream>
#include <iomanip>
#include <math.h>

// This will go into a numtypes header some day.
#if ( sizeof( unsigned char ) == 1 )
typedef unsigned char  uint8;
#endif
#if ( sizeof( signed char ) == 1 )
typedef signed char    sint8;
#endif
#if ( sizeof( unsigned short ) == 2 )
typedef unsigned short uint16;
#endif
#if ( sizeof( signed short ) == 2 )
typedef signed short   sint16;
#endif
#if ( sizeof( unsigned int ) == 4 )
typedef unsigned int   uint32;
#endif
#if ( sizeof( signed char ) == 4 )
typedef signed int     sint32;
#endif

template<typename T> std::ostream& put( std::ostream& os, const T& x )
{ os.write( ( const char* )&x, sizeof( T ) ); return os; }
template<typename T> std::istream& get( std::istream& is, T& x )
{ is.read( ( char* )&x, sizeof( T ) ); return is; }

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
  put( os, uint8( DATATYPE_FLOAT ) );
  put( os, uint8( Channels() ) );
  put( os, uint16( MaxElements() ) );
  for( size_t j = 0; j < MaxElements(); ++j )
    for( size_t i = 0; i < Value.size(); ++i )
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
  SetProperties( SignalProperties( channels, maxElem, sizeof( value_type ) ) );
  switch( datatype )
  {
    case DATATYPE_INTEGER:
      for( int i = 0; i < channels; ++i )
        for( int j = 0; j < maxElem; ++j )
        {
          sint16 value;
          get( is, value );
          SetValue( i, j, value );
        }
      break;
    case DATATYPE_FLOAT:
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


