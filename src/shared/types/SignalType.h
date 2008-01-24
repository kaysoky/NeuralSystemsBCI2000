////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents properties of numeric types present
//   in GenericSignals.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
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
  size_t      Size() const;
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

