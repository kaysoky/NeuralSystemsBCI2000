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
#include <iostream>
#include <iomanip>
#include <limits>
#include <math.h>

#include <assert>

#if( sizeof( signed short ) != 2 || sizeof( signed char ) != 1 )
# error This file depends on 2-byte shorts and 1-byte chars.
#endif

#if( sizeof( unsigned int ) != 4 && sizeof( float ) != 4 )
# error This file assumes a size of 4 bytes for unsigned int and float.
#endif

namespace DATATYPE
{
  enum
  {
    INTEGER = 0,
    FLOAT = 1,
    FLOAT32 = 2,
  };
};

using namespace std;

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
SignalProperties::WriteToStream( ostream& os ) const
{
  os << Channels() << " " << MaxElements() << " " << GetDepth();
}

ostream&
SignalProperties::WriteBinary( ostream& os ) const
{
  LengthField<2>( Channels() ).WriteBinary( os );
  LengthField<2>( MaxElements() ).WriteBinary( os );
  LengthField<1>( GetDepth() ).WriteBinary( os );
  return os;
}

istream&
SignalProperties::ReadBinary( istream& is )
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
  if( IsEmpty() )
    return sp.IsEmpty();
  if( sp.IsEmpty() )
    return true;
  if( depth < sp.depth )
    return false;
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
  if( IsEmpty() )
    return true;
  if( sp.IsEmpty() )
    return false;
  if( sp.depth < depth )
    return false;
  if( sp.elements.size() < elements.size() )
    return false;
  for( size_t i = 0; i < elements.size(); ++i )
    if( sp.elements[ i ] < elements[ i ] )
      return false;
  return true;
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
GenericSignal::WriteToStream( ostream& os ) const
{
  int indent = os.width();
  os << '\n' << setw( indent ) << ""
     << "SignalProperties { ";
  SignalProperties::WriteToStream( os );
  os << '\n' << setw( indent ) << ""
     << "}";
  os << setprecision( 7 );
  for( size_t j = 0; j < MaxElements(); ++j )
  {
    os << '\n' << setw( indent ) << "";
    for( size_t i = 0; i < Value.size(); ++i )
    {
      os << setw( 14 );
      if( j < GetNumElements( i ) )
        os << GetValue( i, j );
      else
        os << "n/a";
      os << ' ';
    }
  }
}

ostream&
GenericSignal::WriteBinary( ostream& os ) const
{
  int datatype = DATATYPE::FLOAT;
  switch( depth )
  {
    case 0:
    case 1:
    case 2:
      datatype = DATATYPE::INTEGER;
      break;
    case 3:
      datatype = DATATYPE::FLOAT;
      break;
#ifdef BCI_TOOL // Remove the #ifdef once the protocol allows for it.
    case 4:
      datatype = DATATYPE::FLOAT32;
      break;
#endif // BCI_TOOL
    default:
      datatype = DATATYPE::FLOAT;
  }
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

    case DATATYPE::FLOAT32:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < MaxElements(); ++j )
        {
          assert( numeric_limits<float>::is_iec559 && sizeof( unsigned int ) == sizeof( float ) );
          unsigned int value = 0;
          if( j < GetNumElements( i ) )
            value = *reinterpret_cast<const unsigned int*>( &GetValue( i, j ) );
          for( int i = 0; i < sizeof( value ); ++i )
          {
            os.put( value & 0xff );
            value >>= 8;
          }
        }
      break;

    default:
      os.setstate( os.failbit );
  }
  return os;
}

istream&
GenericSignal::ReadBinary( istream& is )
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
      SetProperties( SignalProperties( channels, maxElem, 3 ) );
      for( size_t i = 0; i < channels; ++i )
        for( size_t j = 0; j < maxElem; ++j )
        {
          signed short mantissa = is.get();
          mantissa |= is.get() << 8;
          signed char exponent = is.get();
          SetValue( i, j, mantissa * ::pow10( exponent ) );
        }
      break;

    case DATATYPE::FLOAT32:
      SetProperties( SignalProperties( channels, maxElem, 4 ) );
      for( size_t i = 0; i < channels; ++i )
        for( size_t j = 0; j < maxElem; ++j )
        {
          assert( numeric_limits<float>::is_iec559 && sizeof( unsigned int ) == sizeof( float ) );
          unsigned int value = 0;
          for( int i = 0; i < sizeof( value ); ++i )
            value |= is.get() << ( i * 8 );
          SetValue( i, j, *reinterpret_cast<float*>( &value ) );
        }
      break;

    default:
      is.setstate( is.failbit );
  }
  return is;
}


