////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents properties of numeric types present
//   in GenericSignals.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SignalType.h"
#include <string>
#include <limits>
#include <cfloat>

#if( USHRT_MAX != ( 1UL << 16 ) - 1 || UCHAR_MAX != ( 1UL << 8 ) - 1 )
# error This file depends on 2-byte shorts and 1-byte chars.
#endif

#if( UINT_MAX / 2 + 1 != ( 1UL << 31 ) )
# error This file assumes a size of 4 bytes for unsigned int.
#endif

#if( FLT_MANT_DIG != 24 || FLT_MAX_EXP != 128 )
# error This file assumes IEEE754 floats.
#endif

using namespace std;

static struct
{
  SignalType::Type type;
  const char*      name;
  size_t           size;
  double           min,
                   max;
} SignalTypeProperties[] =
{
  { SignalType::int16,   "int16",   2, - ( 1 << 15 ), ( 1 << 15 ) - 1 },
  { SignalType::float24, "float24", 3, - numeric_limits<float>::max(), numeric_limits<float>::max() },
  { SignalType::float32, "float32", 4, - numeric_limits<float>::max(), numeric_limits<float>::max() },
  { SignalType::int32,   "int32",   4, - ( 1LL << 31 ), ( 1LL << 31 ) - 1 },
};

static const int numSignalTypes = sizeof( SignalTypeProperties ) / sizeof( *SignalTypeProperties );

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

double
SignalType::Min() const
{
  for( size_t i = 0; i < numSignalTypes; ++i )
    if( SignalTypeProperties[ i ].type == mType )
      return SignalTypeProperties[ i ].min;
  return -numeric_limits<double>::max();
}

double
SignalType::Max() const
{
  for( size_t i = 0; i < numSignalTypes; ++i )
    if( SignalTypeProperties[ i ].type == mType )
      return SignalTypeProperties[ i ].max;
  return numeric_limits<double>::max();
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

ostream&
SignalType::WriteToStream( ostream& os ) const
{
  return os << Name();
}

istream&
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
  return is;
}

ostream&
SignalType::WriteBinary( ostream& os ) const
{
  return os.put( mType );
}

istream&
SignalType::ReadBinary( istream& is )
{
  mType = Type( is.get() );
  return is;
}
