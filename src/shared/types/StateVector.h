////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for the binary representation of a list of
//   states (event markers).
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef STATE_VECTOR_H
#define STATE_VECTOR_H

#include <iostream>
#include "State.h"
#include "StateList.h"

class StateVector
{
 public:
  StateVector( const StateVector& );
  explicit StateVector( class StateList& list, bool usePositions = false );
  ~StateVector();
  const StateVector& operator=( const StateVector& );

 private:
  void           Initialize( bool useAssignedPositions = false );

 public:
  int            Length() const
                 { return mByteLength; }
  unsigned char* Data()
                 { return mpData; }
  const unsigned char* Data() const
                 { return mpData; }
  class StateList& StateList()
                 { return *mpStateList; }
  const class StateList& StateList() const
                 { return *mpStateList; }

  State::ValueType StateValue( const std::string& name ) const;
  State::ValueType StateValue( size_t location, size_t length ) const;
  void             SetStateValue( const std::string& name, State::ValueType value );
  void             SetStateValue( size_t location, size_t length, State::ValueType value );
  void             PostStateChange( const std::string& name, State::ValueType value );
  void             CommitStateChanges();

  std::ostream&  WriteToStream( std::ostream& ) const;
  std::istream&  ReadFromStream( std::istream& );
  std::ostream&  WriteBinary( std::ostream& ) const;
  std::istream&  ReadBinary( std::istream& );

 private:
  unsigned char*    mpData;      // the actual state vector
  size_t            mByteLength; // the length of the actual state vector
  class StateList*  mpStateList; // a pointer to the list responsible for this vector
};


inline
std::ostream& operator<<( std::ostream& os, const StateVector& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, StateVector& s )
{ return s.ReadFromStream( is ); }

#endif // STATE_VECTOR_H

