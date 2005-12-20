////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: UGenericSignal.cpp
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// $Log$
// Revision 1.17  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
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

static struct
{
  SignalType::Type type;
  const char*      name;
  size_t           size;
} SignalTypeProperties[] =
{
  { SignalType::int16,   "int16",   2 },
  { SignalType::float24, "float24", 3 },
  { SignalType::float32, "float32", 4 },
  { SignalType::int32,   "int32",   4 },
};

static const numSignalTypes = sizeof( SignalTypeProperties ) / sizeof( *SignalTypeProperties );

const char*
SignalType::Name() const
{
  for( size_t i = 0; i < numSignalTypes; ++i )
    if( SignalTypeProperties[ i ].type == mType )
      return SignalTypeProperties[ i ].name;
  return "n/a";
};

size_t
SignalType::Size() const
{
  for( size_t i = 0; i < numSignalTypes; ++i )
    if( SignalTypeProperties[ i ].type == mType )
      return SignalTypeProperties[ i ].size;
  return sizeof( double );
}

// Determine whether a given signal type can be converted into another one without
// loss of information.
bool
SignalType::ConversionIsSafe( SignalType from, SignalType to )
{
  static const bool conversionTable[ numTypes ][ numTypes ] =
  {
    /*              int16    float24   float32    int32 */
    /* int16   */ {  true,     true,     true,    true, },
    /* float24 */ { false,     true,     true,   false, },
    /* float32 */ { false,    false,     true,   false, },
    /* int32   */ { false,    false,    false,    true, },
  };
  return conversionTable[ from.mType ][ to.mType ];
}

void
SignalType::WriteToStream( ostream& os ) const
{
  os << Name();
}

void
SignalType::ReadFromStream( istream& is )
{
  mType = none;
  string s;
  if( is >> s )
    for( size_t i = 0; mType == none && i < numSignalTypes; ++i )
      if( s == SignalTypeProperties[ i ].name )
        mType = SignalTypeProperties[ i ].type;
  if( mType == none )
    is.setstate( ios::failbit );
}

void
SignalType::WriteBinary( ostream& os ) const
{
  os.put( mType );
}

void
SignalType::ReadBinary( istream& is )
{
  mType = Type( is.get() );
}

void
SignalProperties::WriteToStream( ostream& os ) const
{
  os << Channels() << " " << Elements() << " " << Type();
}

ostream&
SignalProperties::WriteBinary( ostream& os ) const
{
  Type().WriteBinary( os );
  LengthField<2>( Channels() ).WriteBinary( os );
  LengthField<2>( Elements() ).WriteBinary( os );
  return os;
}

istream&
SignalProperties::ReadBinary( istream& is )
{
  SignalType     type;
  LengthField<2> channels,
                 elements;
  type.ReadBinary( is );
  channels.ReadBinary( is );
  elements.ReadBinary( is );
  *this = SignalProperties( channels, elements, type );
  return is;
}

bool
SignalProperties::operator<=( const SignalProperties& sp ) const
{
  if( IsEmpty() )
    return true;
  if( sp.IsEmpty() )
    return false;
  if( !SignalType::ConversionIsSafe( Type(), sp.Type() ) )
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

GenericSignal::GenericSignal( size_t inChannels, size_t inElements, SignalType inType )
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
  return operator()( inChannel, inElement );
}

void
GenericSignal::SetValue( size_t inChannel, size_t inElement, value_type inValue )
{
#ifdef SIGNAL_BACK_COMPAT
  if( ( inChannel >= mValues.size() ) || ( inElement >= mValues[ inChannel ].size() ) )
    return;
#endif // SIGNAL_BACK_COMPAT
  operator()( inChannel, inElement ) = inValue;
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
          PutValueBinary<SignalType::int16>( os, i, j );
      break;

    case SignalType::float24:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
          PutValueBinary<SignalType::float24>( os, i, j );
      break;

    case SignalType::float32:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
          PutValueBinary<SignalType::float32>( os, i, j );
      break;

    case SignalType::int32:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
          PutValueBinary<SignalType::int32>( os, i, j );
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
          GetValueBinary<SignalType::int16>( is, i, j );
      break;

    case SignalType::float24:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
          GetValueBinary<SignalType::float24>( is, i, j );
      break;

    case SignalType::float32:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
          GetValueBinary<SignalType::float32>( is, i, j );
      break;

    case SignalType::int32:
      for( size_t i = 0; i < Channels(); ++i )
        for( size_t j = 0; j < Elements(); ++j )
          GetValueBinary<SignalType::int32>( is, i, j );
      break;

    default:
      is.setstate( is.failbit );
  }
  return is;
}

void
GenericSignal::WriteValueBinary( ostream& os, size_t inChannel, size_t inElement ) const
{
  switch( Type() )
  {
    case SignalType::int16:
      PutValueBinary<SignalType::int16>( os, inChannel, inElement );
      break;

    case SignalType::float24:
      PutValueBinary<SignalType::float24>( os, inChannel, inElement );
      break;

    case SignalType::float32:
      PutValueBinary<SignalType::float32>( os, inChannel, inElement );
      break;

    case SignalType::int32:
      PutValueBinary<SignalType::int32>( os, inChannel, inElement );
      break;

    default:
      os.setstate( os.failbit );
  }
}

void
GenericSignal::ReadValueBinary( istream& is, size_t inChannel, size_t inElement )
{
  switch( Type() )
  {
    case SignalType::int16:
      GetValueBinary<SignalType::int16>( is, inChannel, inElement );
      break;

    case SignalType::float24:
      GetValueBinary<SignalType::float24>( is, inChannel, inElement );
      break;

    case SignalType::float32:
      GetValueBinary<SignalType::float32>( is, inChannel, inElement );
      break;

    case SignalType::int32:
      GetValueBinary<SignalType::int32>( is, inChannel, inElement );
      break;

    default:
      is.setstate( is.failbit );
  }
}

template<>
void
GenericSignal::PutValueBinary<SignalType::int16>( std::ostream& os, size_t inChannel, size_t inElement ) const
{
  int value = GetValue( inChannel, inElement );
  os.put( value & 0xff ).put( value >> 8 );
}

template<>
void
GenericSignal::GetValueBinary<SignalType::int16>( std::istream& is, size_t inChannel, size_t inElement )
{
  signed short value = is.get();
  value |= is.get() << 8;
  SetValue( inChannel, inElement, value );
}

template<>
void
GenericSignal::PutValueBinary<SignalType::int32>( std::ostream& os, size_t inChannel, size_t inElement ) const
{
  signed int value = GetValue( inChannel, inElement );
  PutLittleEndian( os, value );
}

template<>
void
GenericSignal::GetValueBinary<SignalType::int32>( std::istream& is, size_t inChannel, size_t inElement )
{
  signed int value = 0;
  GetLittleEndian( is, value );
  SetValue( inChannel, inElement, value );
}

template<>
void
GenericSignal::PutValueBinary<SignalType::float24>( std::ostream& os, size_t inChannel, size_t inElement ) const
{
  float value = GetValue( inChannel, inElement );
  int mantissa,
      exponent;
  if( value == 0.0 )
  {
    mantissa = 0;
    exponent = 1;
  }
  else
  {
    exponent = ::ceil( ::log10( ::fabs( value ) ) );
    mantissa = ( value / ::pow10( exponent ) ) * 10000;
    exponent -= 4;
  }
  os.put( mantissa & 0xff ).put( mantissa >> 8 );
  os.put( exponent & 0xff );
}

template<>
void
GenericSignal::GetValueBinary<SignalType::float24>( std::istream& is, size_t inChannel, size_t inElement )
{
  signed short mantissa = is.get();
  mantissa |= is.get() << 8;
  signed char exponent = is.get();
  SetValue( inChannel, inElement, mantissa * ::pow10( exponent ) );
}

template<>
void
GenericSignal::PutValueBinary<SignalType::float32>( std::ostream& os, size_t inChannel, size_t inElement ) const
{
  assert( numeric_limits<float>::is_iec559 && sizeof( unsigned int ) == sizeof( float ) );
  float floatvalue = GetValue( inChannel, inElement );
  unsigned int value = *reinterpret_cast<const unsigned int*>( &floatvalue );
  PutLittleEndian( os, value );
}

template<>
void
GenericSignal::GetValueBinary<SignalType::float32>( std::istream& is, size_t inChannel, size_t inElement )
{
  assert( numeric_limits<float>::is_iec559 && sizeof( unsigned int ) == sizeof( float ) );
  unsigned int value = 0;
  GetLittleEndian( is, value );
  try
  {
    SetValue( inChannel, inElement, *reinterpret_cast<float*>( &value ) );
  }
  catch( ... )
  {
    is.setstate( is.failbit );
  }
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

