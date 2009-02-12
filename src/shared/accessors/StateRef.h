//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that holds references to state values, and
//         allows for convenient automatic type
//         conversions when accessing state values.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#ifndef STATE_REF_H
#define STATE_REF_H

#include "State.h"
#include "StateVector.h"

class StateRef
{
 public:
  StateRef();
  StateRef( const State*,
            StateVector*,
            int sample,
            long defaultValue = 0 );
  StateRef& operator=( const StateRef& );
  const StateRef& operator=( long );
  operator long() const;
  StateRef operator()( size_t offset ) const;
  const State* operator->() const;

 private:
  const State* mpState;
  StateVector* mpStateVector;
  int          mSample;
  long         mDefaultValue;
};


inline
StateRef::StateRef()
: mpState( NULL ),
  mpStateVector( NULL ),
  mSample( 0 ),
  mDefaultValue( 0 )
{
}

inline
StateRef::StateRef( const State* inState,
                    StateVector* inStateVector,
                    int inSample,
                    long inDefaultValue )
: mpState( inState ),
  mpStateVector( inStateVector ),
  mSample( inSample ),
  mDefaultValue( inDefaultValue )
{
}

inline
StateRef&
StateRef::operator=( const StateRef& s )
{
  *this = long( s );
  return *this;
}

inline
const StateRef&
StateRef::operator=( long inValue )
{
  if( mpState != NULL && mpStateVector != NULL )
    mpStateVector->SetStateValue( mpState->Location(), mpState->Length(), mSample, inValue );
  return *this;
}

inline
StateRef::operator long() const
{
  long value = mDefaultValue;
  if( mpStateVector != NULL )
    value = mpStateVector->StateValue( mpState->Location(), mpState->Length(), mSample );
  return value;
}

inline
StateRef
StateRef::operator()( size_t inOffset ) const
{
  return StateRef( mpState, mpStateVector, mSample + inOffset, mDefaultValue );
}

inline
const State*
StateRef::operator->() const
{
  return mpState;
}

#endif // STATE_REF_H
