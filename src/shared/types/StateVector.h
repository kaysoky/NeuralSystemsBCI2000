////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for the the binary representation of states
//  (event markers) for an entire data block.
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
#ifndef STATE_VECTOR_H
#define STATE_VECTOR_H

#include <iostream>
#include <vector>
#include "StateVectorSample.h"

class StateVector
{
 public:
  StateVector();
  explicit StateVector( class StateList& list, size_t numSamples = 1 );

 public:
  const StateVector& CopyFromMasked( const StateVector&, const StateVectorSample& mask );

  int            Samples() const
                 { return static_cast<int>( mSamples.size() ); }
  int            Length() const
                 { return mSamples.empty() ? 0 : mSamples[0].Length(); }
  StateVectorSample& operator()( size_t inIdx )
                 { return mSamples[ inIdx ]; }
  const StateVectorSample& operator()( size_t inIdx ) const
                 { return mSamples[ inIdx ]; }
  const class StateList& StateList() const
                 { return *mpStateList; }

  State::ValueType StateValue( const std::string& name, size_t sample = 0 ) const;
  State::ValueType StateValue( size_t location, size_t length, size_t sample = 0 ) const;
  void             SetStateValue( const std::string& name, State::ValueType value )
                   { SetStateValue( name, 0, value ); }
  void             SetStateValue( const std::string& name, size_t sample, State::ValueType value );
  void             SetStateValue( size_t location, size_t length, State::ValueType value )
                   { SetStateValue( location, length, 0, value ); }
  void             SetStateValue( size_t location, size_t length, size_t sample, State::ValueType value );
  void             PostStateChange( const std::string& name, State::ValueType value );
  void             CommitStateChanges();

  std::ostream&  WriteToStream( std::ostream& ) const;
  std::istream&  ReadFromStream( std::istream& );
  std::ostream&  WriteBinary( std::ostream& ) const;
  std::istream&  ReadBinary( std::istream& );

 private:
  class StateList*               mpStateList;
  std::vector<StateVectorSample> mSamples;
};


inline
std::ostream& operator<<( std::ostream& os, const StateVector& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, StateVector& s )
{ return s.ReadFromStream( is ); }

#endif // STATE_VECTOR_BLOCK_H

