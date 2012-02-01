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
//     PhysicalValue = ( RawValue - offset ) * gain * unit
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
#ifndef PHYSICAL_UNIT_H
#define PHYSICAL_UNIT_H

#include <iostream>
#include <map>
#include <string>
#include "EncodedString.h"

class PhysicalUnit
{
 public:
  typedef double ValueType;

  PhysicalUnit() : mOffset( 0 ), mGain( 1 ), mRawMin( -1 ), mRawMax( 1 ) {}
  ~PhysicalUnit() {}

  PhysicalUnit& Clear()
                { return ( *this = PhysicalUnit() ); }

  ValueType     Offset() const
                { return mOffset; }
  PhysicalUnit& SetOffset( ValueType v )
                { mOffset = v; return *this; }

  ValueType     Gain() const
                { return mGain; }
  PhysicalUnit& SetGain( ValueType g )
                { mGain = g; return *this; }

  const std::string& Symbol() const
                { return mSymbol; }
  PhysicalUnit& SetSymbol( const std::string&, double power = 1 );

  ValueType     RawMin() const
                { return mRawMin; }
  PhysicalUnit& SetRawMin( ValueType v )
                { mRawMin = v; return *this; }

  ValueType     RawMax() const
                { return mRawMax; }
  PhysicalUnit& SetRawMax( ValueType v )
                { mRawMax = v; return *this; }

  int           Size() const;

  bool          IsPhysical( const std::string& ) const;
  ValueType     PhysicalToRaw( const std::string& ) const;
  std::string   RawToPhysical( ValueType ) const;

  bool          operator==( const PhysicalUnit& ) const;
  bool          operator!=( const PhysicalUnit& u ) const
                { return !( *this == u ); }
  PhysicalUnit& operator*=( const PhysicalUnit& );

  PhysicalUnit& Combine( const PhysicalUnit& );

  std::ostream& WriteToStream( std::ostream& os ) const;
  std::istream& ReadFromStream( std::istream& is );

 private:
  bool          SexagesimalAllowed() const;
  double        ExtractUnit( std::string& ) const;

  struct SymbolPowers : std::map<std::string, double>
  {
    SymbolPowers& operator*=( const SymbolPowers& );
    std::string SingleSymbol() const;
  };

  EncodedString mSymbol;
  SymbolPowers  mSymbolPowers;
  ValueType     mOffset,
                mGain,
                mRawMin,
                mRawMax;
};

inline
std::ostream& operator<<( std::ostream& os, const PhysicalUnit& u )
{ return u.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, PhysicalUnit& u )
{ return u.ReadFromStream( is ); }

#endif // PHYSICAL_UNIT_H

