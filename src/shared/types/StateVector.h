////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for the the binary representation of states
//  (event markers) for an entire data block.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef STATE_VECTOR_H
#define STATE_VECTOR_H

#include <iostream>
#include <vector>
#include "StateVectorSample.h"

class StateVector
{
 public:
  explicit StateVector( class StateList& list, size_t numSamples = 1 );
  ~StateVector();

 public:
  int            Samples() const
                 { return mSamples.size(); }
  int            Length() const
                 { return mByteLength; }
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
  size_t                         mByteLength;
  std::vector<StateVectorSample> mSamples;
};


inline
std::ostream& operator<<( std::ostream& os, const StateVector& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, StateVector& s )
{ return s.ReadFromStream( is ); }

#endif // STATE_VECTOR_BLOCK_H

