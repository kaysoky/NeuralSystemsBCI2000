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
#include "PCHIncludes.h"
#pragma hdrstop

#include "PhysicalUnit.h"
#include "ArithmeticExpression.h"
#include "BCIError.h"
#include <limits>
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;

bool
PhysicalUnit::IsPhysical( const string& inValue ) const
{ // IsPhysical will always return false when the Symbol property is empty.
  size_t pos = string::npos;
  if( !mSymbol.empty() )
    pos = inValue.rfind( mSymbol );
  return pos != string::npos && pos == inValue.length() - mSymbol.length();
}

PhysicalUnit::ValueType
PhysicalUnit::ExtractUnit( string& ioValue ) const
{
  ValueType unit = 1.0;
  size_t pos = ioValue.find_first_not_of( "0123456789.Ee+-*/^() " );
  if( pos != ioValue.npos )
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
      ValueType      value;
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
  return unit;
}

PhysicalUnit::ValueType
PhysicalUnit::PhysicalToRaw( const string& inPhysicalValue ) const
{
  string physValue( inPhysicalValue );
  ValueType unit = ExtractUnit( physValue ),
            result = 0;
            
  if( ::fabs( unit ) > numeric_limits<ValueType>::epsilon() )
  {
    result = ArithmeticExpression( physValue ).Evaluate() / unit + mOffset;
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
    while( i < numPrefixes && prefixes[ i ].order < orderOfMagnitude - 1 )
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

