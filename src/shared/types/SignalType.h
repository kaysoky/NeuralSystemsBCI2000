////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents properties of numeric types present
//   in GenericSignals.
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
#ifndef SIGNAL_TYPE_H
#define SIGNAL_TYPE_H

#include <iostream>

class SignalType
{
 public:
  typedef enum Type
  {
    none = -1,
    int16 = 0,
    float24,
    float32,
    int32,

    numTypes,
    defaultType = float32

  } Type;

  SignalType() : mType( none )      {}
  SignalType( Type t ) : mType( t ) {}
  operator SignalType::Type() const { return mType; }

  const char* Name() const;
  int         Size() const;
  double      Min()  const;
  double      Max()  const;
  static bool ConversionIsSafe( SignalType from, SignalType to );

  std::istream& ReadFromStream( std::istream& );
  std::ostream& WriteToStream( std::ostream& ) const;
  std::istream& ReadBinary( std::istream& );
  std::ostream& WriteBinary( std::ostream& ) const;

 private:
  Type mType;
};

inline
std::ostream& operator<<( std::ostream& os, const SignalType& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, SignalType& s )
{ return s.ReadFromStream( is ); }

#endif // SIGNAL_TYPE_H

