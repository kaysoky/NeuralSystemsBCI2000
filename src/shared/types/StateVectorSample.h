////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for the binary representation of a list of
//   state variables corresponding to a single sample.
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
#ifndef STATE_VECTOR_SAMPLE_H
#define STATE_VECTOR_SAMPLE_H

#include <iostream>
#include "State.h"

class StateVectorSample
{
 public:
  StateVectorSample( const StateVectorSample& );
  explicit StateVectorSample( size_t byteLength );
  ~StateVectorSample();
  const StateVectorSample& operator=( const StateVectorSample& );
  // CopyFromMasked() copies bits that are set to 1 in the mask.
  const StateVectorSample& CopyFromMasked( const StateVectorSample&, const StateVectorSample& mask );

 public:
  int            Length() const
                 { return static_cast<int>( mByteLength ); }
  unsigned char* Data()
                 { return mpData; }
  const unsigned char* Data() const
                 { return mpData; }

  State::ValueType StateValue( size_t location, size_t length ) const;
  void             SetStateValue( size_t location, size_t length, State::ValueType value );

  std::ostream&  WriteBinary( std::ostream& ) const;
  std::istream&  ReadBinary( std::istream& );

 private:
  void           SetStateValue_( size_t, size_t, State::ValueType );
  size_t         mByteLength; // the length of the binary representation
  unsigned char* mpData;      // binary state data
};

#endif // STATE_VECTOR_SAMPLE_H

