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

#include "LengthField.h"
#include "defines.h" // for DATATYPE::FLOAT / DATATYPE::INTEGER
#include <iostream>
#include <iomanip>
#include <math.h>

#if( sizeof( signed short ) != 2 || sizeof( signed char ) != 1 )
# error This file depends on 2-byte shorts and 1-byte chars.
#endif

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
  LengthField<2>( Channels() ).WriteBinary( os );
  LengthField<2>( MaxElements() ).WriteBinary( os );
  LengthField<1>( GetDepth() ).WriteBinary( os );
  return os;
}

std::istream&
SignalProperties::ReadBinary( std::istream& is )
{
  LengthField<2> channels,
                 maxElem;
  LengthField<1> depth;
  channels.ReadBinary( is );
  maxElem.ReadBinary( is );
  depth.ReadBinary( is );
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
  int datatype = ( depth >= sizeof( float ) ? DATATYPE::FLOAT : DATATYPE::INTEGER );
  os.put( datatype );
  LengthField<1>( Channels() ).WriteBinary( os );
  LengthField<2>( MaxElements() ).WriteBinary( os );
  switch( datatype )
  {
    case DATATYPE::INTEGER:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < MaxElements(); ++j )
        {
          int value = 0;
          if( j < GetNumElements( i ) )
            value = GetValue( i, j );
          os.put( value & 0xff ).put( value >> 8 );
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
          os.put( mantissa & 0xff ).put( mantissa >> 8 );
          os.put( exponent & 0xff );
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
  int datatype = is.get();
  LengthField<1> channels;
  channels.ReadBinary( is );
  LengthField<2> maxElem;
  maxElem.ReadBinary( is );
  switch( datatype )
  {
    case DATATYPE::INTEGER:
      SetProperties( SignalProperties( channels, maxElem, 2 ) );
      for( size_t i = 0; i < channels; ++i )
        for( size_t j = 0; j < maxElem; ++j )
        {
          signed short value = is.get();
          value |= is.get() << 8;
          SetValue( i, j, value );
        }
      break;

    case DATATYPE::FLOAT:
      SetProperties( SignalProperties( channels, maxElem, sizeof( float ) ) );
      for( size_t i = 0; i < channels; ++i )
        for( size_t j = 0; j < maxElem; ++j )
        {
          signed short mantissa = is.get();
          mantissa |= is.get() << 8;
          signed char exponent = is.get();
          SetValue( i, j, mantissa * ::pow10( exponent ) );
        }
      break;
      
    default:
      is.setstate( is.failbit );
  }
  return is;
}


