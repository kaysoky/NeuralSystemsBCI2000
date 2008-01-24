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
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef PHYSICAL_UNIT_H
#define PHYSICAL_UNIT_H

#include <iostream>
#include "EncodedString.h"

class PhysicalUnit
{
 public:
  typedef double ValueType;

  PhysicalUnit() : mOffset( 0 ), mGain( 1 ), mRawMin( -1 ), mRawMax( 1 ) {}
  ~PhysicalUnit() {}

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
  PhysicalUnit& SetSymbol( const std::string& s )
                { mSymbol = s; return *this; }

  ValueType     RawMin() const
                { return mRawMin; }
  PhysicalUnit& SetRawMin( ValueType v )
                { mRawMin = v; return *this; }

  ValueType     RawMax() const
                { return mRawMax; }
  PhysicalUnit& SetRawMax( ValueType v )
                { mRawMax = v; return *this; }

  bool          IsPhysical( const std::string& ) const;
  ValueType     PhysicalToRaw( const std::string& ) const;
  std::string   RawToPhysical( ValueType ) const;

  std::ostream& WriteToStream( std::ostream& os ) const;
  std::istream& ReadFromStream( std::istream& is );

 private:
  double        ExtractUnit( std::string& ) const;

  EncodedString mSymbol;
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

