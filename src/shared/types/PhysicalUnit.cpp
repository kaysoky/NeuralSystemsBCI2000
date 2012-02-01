////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: PhysicalUnit is a class that represents a linear mapping from
//   physical units to raw numbers.
//   Apart from the obvious use in conjunction with measured values, this
//   class is also used for signal element or channel indexing when appropriate,
//   e.g. to map a frequency value to a spectral bin (element index).
//   Consistently with the definition of the SourceChOffset and SourceChGain
//   parameters, the relation between raw and physical value is
//     PhysicalValue = ( RawValue - offset ) * gain * symbol
//
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
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "PhysicalUnit.h"
#include "ArithmeticExpression.h"
#include "BCIError.h"
#include "BCIException.h"
#include <limits>
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;

static const string sSexagesimalSeparators = ":";

PhysicalUnit&
PhysicalUnit::SetSymbol( const std::string& inSymbol, double inPower )
{
  mSymbolPowers.clear();
  mSymbolPowers[inSymbol] = inPower;
  mSymbol = mSymbolPowers.SingleSymbol();
  return *this;
}

int
PhysicalUnit::Size() const
{
  return static_cast<int>( ::fabs( mRawMax - mRawMin ) + 1 );
}

bool
PhysicalUnit::IsPhysical( const string& inValue ) const
{ // IsPhysical will always return false when the Symbol property is empty.
  bool result = false;
  size_t pos = string::npos;
  if( !mSymbol.empty() )
  {
    pos = inValue.rfind( mSymbol );
    if( pos != string::npos )
      result = ( pos == inValue.length() - mSymbol.length() );
    else
      result = ( SexagesimalAllowed() && inValue.find_first_of( sSexagesimalSeparators ) != string::npos );
  }
  return result;
}

bool
PhysicalUnit::SexagesimalAllowed() const
{
  const char* allowSexagesimal[] = { "", "s", "deg" };
  size_t count = sizeof( allowSexagesimal ) / sizeof( *allowSexagesimal );
  size_t i = 0;
  while( i < count && allowSexagesimal[i] != mSymbol )
    ++i;
  return i < count;
}

PhysicalUnit::ValueType
PhysicalUnit::ExtractUnit( string& ioValue ) const
{
  ValueType unit = 1.0;
  size_t pos = ioValue.find_first_not_of( "0123456789.Ee+-*/^() " + sSexagesimalSeparators );
  if( pos != string::npos )
  {
    string unitFromValue = ioValue.substr( pos );
    if( unitFromValue.length() >= mSymbol.length()
        && unitFromValue.substr( unitFromValue.length() - mSymbol.length() ) == mSymbol )
    {
      unit *= mGain;
      unitFromValue.erase( unitFromValue.length() - mSymbol.length(), mSymbol.length() );
    }
    const struct
    {
      const char* name;
      ValueType value;
    } prefixes[] =
    {
      { "p", 1e-12 },
      { "n", 1e-9 },
      { "u", 1e-6 }, { "mu", 1e-6 },
      { "m", 1e-3 },
      { "",  1.0 },
      { "k", 1e3 },
      { "M", 1e6 },
      { "G", 1e9 },
      { "T", 1e12 },
    };
    const int numPrefixes = sizeof( prefixes ) / sizeof( *prefixes );
    int i = 0;
    while( i < numPrefixes && unitFromValue != prefixes[ i ].name )
      ++i;
    if( i < numPrefixes )
    {
      unit /= prefixes[ i ].value;
      ioValue.erase( pos );
    }
    else
    {
      unit = 0.0;
    }
  }
  if( ioValue.find_first_of( sSexagesimalSeparators ) != string::npos )
  {
    if( !SexagesimalAllowed() )
    {
      bcierr << "Sexagesimal format not allowed for values of this type" << endl;
      ioValue = "1";
    }
    else if( unit == 1.0 )
    {
      unit = Gain();
    }
  }
  return unit;
#ifdef TODO
# error Issue an error when a unit is present but the string returned in ioValue is empty.
#endif
}

PhysicalUnit::ValueType
PhysicalUnit::PhysicalToRaw( const string& inPhysicalValue ) const
{
  string physValue( inPhysicalValue );
  ValueType unit = ExtractUnit( physValue ),
            result = 0;

  if( ::fabs( unit ) > numeric_limits<ValueType>::epsilon() )
  {
    if( !physValue.empty() )
    {
      bool valid = true;
      int count = 0;
      size_t beginPos = 0;
      while( beginPos <= physValue.length() )
      {
        ++count;
        size_t endPos = physValue.find_first_of( sSexagesimalSeparators, beginPos );
        if( endPos == string::npos )
          endPos = physValue.length();
        size_t length = endPos - beginPos;
        valid &= ( length > 0 );
        ValueType value = ArithmeticExpression( physValue.substr( beginPos, length ) ).Evaluate();
        if( beginPos != 0 )
        {
          valid &= ( value >= 0 && value < 60 );
          result *= 60;
        }
        result += value;
        beginPos = endPos + 1;
      }
      valid &= ( count <= 3 );
      if( !valid )
        bcierr << "Invalid sexagesimal number format: " << inPhysicalValue << endl;
      result = result / unit + mOffset;
    }
  }
  else
  {
    bcierr << "Unexpected measurement unit in expression "
           << "\"" << inPhysicalValue << "\""
           << (
                mSymbol.empty()
                ? string( mSymbol )
                : string( " , expected \"" ) + mSymbol.c_str() + "\""
              )
           << endl;
  }
  return result;
}

string
PhysicalUnit::RawToPhysical( ValueType inRawValue ) const
{
  ostringstream oss;
  ValueType value = ( inRawValue - mOffset ) * mGain;
  if( ::fabs( value ) > 0 )
  {
    int orderOfMagnitude = static_cast<int>( ::floor( ::log10( ::fabs( value ) ) ) );
    const struct
    {
      const char* name;
      int         order;
    } prefixes[] =
    {
      { "p",  -12 },
      { "n",  -9 },
      { "mu", -6 },
      { "m",  -3 },
      { "",   0 },
      { "k",  3 },
      { "M",  6 },
      { "G",  9 },
      { "T",  12 },
    };
    const int numPrefixes = sizeof( prefixes ) / sizeof( *prefixes );
    int i = 0;
    while( i < numPrefixes && prefixes[ i ].order < orderOfMagnitude - 2 )
      ++i;
    value /= ::pow( 10.0f, prefixes[ i ].order );

    oss << value << prefixes[ i ].name << mSymbol.c_str();
  }
  else
  {
    oss << '0';
  }
  return oss.str();
}

bool
PhysicalUnit::operator==( const PhysicalUnit& inUnit ) const
{
  return mSymbol == inUnit.mSymbol
      && mOffset == inUnit.mOffset
      && mGain == inUnit.mGain;
}

PhysicalUnit&
PhysicalUnit::operator*=( const PhysicalUnit& inUnit )
{
  if( mOffset != 0 || inUnit.mOffset != 0 )
    throw bciexception( "Cannot multiply physical units with offsets" );
  mSymbolPowers *= inUnit.mSymbolPowers;
  mSymbol = mSymbolPowers.SingleSymbol();
  mGain *= inUnit.mGain;
  ValueType max1max2 = mRawMax * inUnit.mRawMax,
            max1min2 = mRawMax * inUnit.mRawMin,
            min1max2 = mRawMin * inUnit.mRawMax,
            min1min2 = mRawMin * inUnit.mRawMin;
  mRawMax = max( max( max1max2, max1min2 ), max( min1max2, min1min2 ) );
  mRawMin = min( min( max1max2, max1min2 ), min( min1max2, min1min2 ) );
  return *this;
}

PhysicalUnit&
PhysicalUnit::Combine( const PhysicalUnit& inUnit )
{
  if( *this != inUnit )
  {
    mGain = 1;
    mOffset = 0;
    mSymbol = "";
  }
  mRawMax = max( mRawMax, inUnit.mRawMax );
  mRawMin = min( mRawMin, inUnit.mRawMin );
  return *this;
}

ostream&
PhysicalUnit::WriteToStream( ostream& os ) const
{
  return os << setprecision( 10 )
            << mOffset << ' '
            << mGain << ' '
            << mSymbol << ' '
            << mRawMin << ' '
            << mRawMax;
}

istream&
PhysicalUnit::ReadFromStream( istream& is )
{
  return is >> mOffset
            >> mGain
            >> mSymbol
            >> mRawMin
            >> mRawMax;
}

PhysicalUnit::SymbolPowers&
PhysicalUnit::SymbolPowers::operator*=( const SymbolPowers& inP )
{
  for( const_iterator i = inP.begin(); i != inP.end(); ++i )
    ( *this )[i->first] += i->second;
  return *this;
}

string
PhysicalUnit::SymbolPowers::SingleSymbol() const
{
  ostringstream oss;
  for( const_iterator i = begin(); i != end(); ++i )
    if( i->second != 0 )
    {
      oss << i->first;
      if( i->second != 1 )
        oss << '^' << i->second;
    }
  return oss.str();
}

