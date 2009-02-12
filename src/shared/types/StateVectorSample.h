////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for the binary representation of a list of
//   state variables corresponding to a single sample.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef STATE_VECTOR_SAMPLE_H
#define STATE_VECTOR_SAMPLE_H

#include <iostream>
#include "State.h"
#include "StateList.h"

class StateVectorSample
{
 public:
  StateVectorSample( const StateVectorSample& );
  explicit StateVectorSample( size_t inByteLength );
  ~StateVectorSample();
  const StateVectorSample& operator=( const StateVectorSample& );

 public:
  int            Length() const
                 { return mByteLength; }
  unsigned char* Data()
                 { return mpData; }
  const unsigned char* Data() const
                 { return mpData; }

  State::ValueType StateValue( size_t location, size_t length ) const;
  void             SetStateValue( size_t location, size_t length, State::ValueType value );

  std::ostream&  WriteBinary( std::ostream& ) const;
  std::istream&  ReadBinary( std::istream& );

 private:
  unsigned char* mpData;      // binary state data
  size_t         mByteLength; // the length of the binary representation
};

#endif // STATE_VECTOR_SAMPLE_H

