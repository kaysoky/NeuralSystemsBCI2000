////////////////////////////////////////////////////////////////////////////////
//
// File: UGenericSignal.cpp
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: This file defines a SignalProperties base class and a
//   GenericSignal class deriving from it.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UGenericSignal.h"

#include "LengthField.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <float.h>
#include <math.h>

#include <assert>

#if( USHRT_MAX != ( 1UL << 16 ) - 1 || UCHAR_MAX != ( 1UL << 8 ) - 1 )
# error This file depends on 2-byte shorts and 1-byte chars.
#endif

#if( UINT_MAX / 2 + 1 != ( 1UL << 31 ) || FLT_MANT_DIG != 24 || FLT_MAX_EXP != 128 )
# error This file assumes a size of 4 bytes for unsigned int and float.
#endif

using namespace std;

const char*
SignalType::Name( SignalType::Type t )
{
  static const char* nameTable[] =
  {
    "int16",
    "float24",
    "float32",
    "int32",
    "float64",
  };
  return nameTable[ t ];
};

// Determine whether a given signal type can be converted into another one without
// loss of information.
bool
SignalType::ConversionSafe( SignalType::Type from, SignalType::Type to )
{
  static const bool conversionTable[ numTypes ][ numTypes ] =
  {
    /*              int16    float24   float32    int32 */
    /* int16   */ {  true,     true,     true,    true, },
    /* float24 */ { false,     true,     true,   false, },
    /* float32 */ { false,    false,     true,   false, },
    /* int32   */ { false,    false,    false,    true, },
  };
  return conversionTable[ from ][ to ];
}

void
SignalProperties::WriteToStream( ostream& os ) const
{
  os << Channels() << " " << Elements() << " " << SignalType::Name( Type() );
}

ostream&
SignalProperties::WriteBinary( ostream& os ) const
{
  LengthField<1>( Type() ).WriteBinary( os );
  LengthField<2>( Channels() ).WriteBinary( os );
  LengthField<2>( Elements() ).WriteBinary( os );
  return os;
}

istream&
SignalProperties::ReadBinary( istream& is )
{
  LengthField<1> type;
  LengthField<2> channels,
                 elements;
  type.ReadBinary( is );
  channels.ReadBinary( is );
  elements.ReadBinary( is );
  *this = SignalProperties( channels, elements, SignalType::Type( int( type ) ) );
  return is;
}

bool
SignalProperties::operator<=( const SignalProperties& sp ) const
{
  if( IsEmpty() )
    return true;
  if( sp.IsEmpty() )
    return false;
  if( !ConversionSafe( Type(), sp.Type() ) )
    return false;
  if( sp.Elements() < Elements() )
    return false;
  if( sp.Elements() < Elements() )
    return false;
  return true;
}

GenericSignal::GenericSignal()
{
  SetProperties( mProperties );
}

GenericSignal::GenericSignal( size_t inChannels, size_t inElements, SignalType::Type inType )
{
  SetProperties( SignalProperties( inChannels, inElements, inType ) );
}

GenericSignal::GenericSignal( const SignalProperties& inProperties )
{
  SetProperties( inProperties );
}

const GenericSignal::value_type&
GenericSignal::GetValue( size_t inChannel, size_t inElement ) const
{
#ifdef SIGNAL_BACK_COMPAT
  static value_type nullvalue = ( value_type )0;
  if( ( inChannel >= mValues.size() ) || ( inElement >= mValues[ inChannel ].size() ) )
    return nullvalue;
#endif // SIGNAL_BACK_COMPAT
  return mValues.at( inChannel ).at( inElement );
}

void
GenericSignal::SetValue( size_t inChannel, size_t inElement, value_type inValue )
{
#ifdef SIGNAL_BACK_COMPAT
  if( ( inChannel >= mValues.size() ) || ( inElement >= mValues[ inChannel ].size() ) )
    return;
#endif // SIGNAL_BACK_COMPAT
  mValues.at( inChannel ).at( inElement ) = inValue;
}

const GenericSignal::value_type&
GenericSignal::operator() ( size_t inChannel, size_t inElement ) const
{
#ifdef _DEBUG
  return mValues.at( inChannel ).at( inElement );
#else
  return mValues[ inChannel ][ inElement ];
#endif
}

GenericSignal::value_type&
GenericSignal::operator() ( size_t inChannel, size_t inElement )
{
#ifdef _DEBUG
  return mValues.at( inChannel ).at( inElement );
#else
  return mValues[ inChannel ][ inElement ];
#endif
}

void
GenericSignal::SetProperties( const SignalProperties& inSp )
{
  mValues.resize( inSp.Channels() );
  for( size_t i = 0; i != mValues.size(); ++i )
    mValues[ i ].resize( inSp.Elements(), value_type( 0 ) );
  mProperties = inSp;
}

void
GenericSignal::WriteToStream( ostream& os ) const
{
  int indent = os.width();
  os << '\n' << setw( indent ) << ""
     << "SignalProperties { ";
  mProperties.WriteToStream( os );
  os << '\n' << setw( indent ) << ""
     << "}";
  os << setprecision( 7 );
  for( size_t j = 0; j < Elements(); ++j )
  {
    os << '\n' << setw( indent ) << "";
    for( size_t i = 0; i < mValues.size(); ++i )
    {
      os << setw( 14 )
         << GetValue( i, j )
         << ' ';
    }
  }
}

ostream&
GenericSignal::WriteBinary( ostream& os ) const
{
  mProperties.WriteBinary( os );
  switch( Type() )
  {
    case SignalType::int16:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
        {
          int value = GetValue( i, j );
          os.put( value & 0xff ).put( value >> 8 );
        }
      break;

    case SignalType::float24:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
        {
          float value = GetValue( i, j );
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

    case SignalType::float32:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
        {
          assert( numeric_limits<float>::is_iec559 && sizeof( unsigned int ) == sizeof( float ) );
          float floatvalue = GetValue( i, j );
          unsigned int value = *reinterpret_cast<const unsigned int*>( &floatvalue );
          PutLittleEndian( os, value );
        }
      break;

    case SignalType::int32:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
        {
          signed int value = GetValue( i, j );
          PutLittleEndian( os, value );
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
  SignalProperties p;
  p.ReadBinary( is );
  SetProperties( p );
  switch( Type() )
  {
    case SignalType::int16:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
        {
          signed short value = is.get();
          value |= is.get() << 8;
          SetValue( i, j, value );
        }
      break;

    case SignalType::float24:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
        {
          signed short mantissa = is.get();
          mantissa |= is.get() << 8;
          signed char exponent = is.get();
          SetValue( i, j, mantissa * ::pow10( exponent ) );
        }
      break;

    case SignalType::float32:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
        {
          assert( numeric_limits<float>::is_iec559 && sizeof( unsigned int ) == sizeof( float ) );
          unsigned int value = 0;
          GetLittleEndian( is, value );
          SetValue( i, j, *reinterpret_cast<float*>( &value ) );
        }
      break;

    case SignalType::int32:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
        {
          signed int value = 0;
          GetLittleEndian( is, value );
          SetValue( i, j, value );
        }
      break;

    default:
      is.setstate( is.failbit );
  }
  return is;
}

template<typename T>
void
GenericSignal::PutLittleEndian( std::ostream& os, const T& inValue )
{
  T value = inValue;
  for( int i = 0; i < sizeof( T ); ++i )
  {
    os.put( value & 0xff );
    value >>= 8;
  }
}
template<typename T>
void
GenericSignal::GetLittleEndian( std::istream& is, T& outValue )
{
  outValue = 0;
  for( int i = 0; i < sizeof( T ); ++i )
    outValue |= is.get() << ( i * 8 );
}

