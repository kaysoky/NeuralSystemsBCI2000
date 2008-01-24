//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that holds references to state values, and
//         allows for convenient automatic type
//         conversions when accessing state values.
//
// (C) 2000-2008, BCI2000 Project
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
  StateRef( StateVector*,
            size_t location, size_t length,
            long defaultValue = 0 );
  StateRef& operator=( const StateRef& );
  const StateRef& operator=( long );
  operator long() const;

 private:
  StateVector* mpStateVector;
  size_t       mLocation,
               mLength;
  long         mDefaultValue;
};


inline
StateRef::StateRef()
: mpStateVector( NULL ),
  mLocation( 0 ),
  mLength( 0 ),
  mDefaultValue( 0 )
{
}

inline
StateRef::StateRef( StateVector* inStateVector,
                    size_t inLocation,
                    size_t inLength,
                    long inDefaultValue )
: mpStateVector( inStateVector ),
  mLocation( inLocation ),
  mLength( inLength ),
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
  if( mpStateVector != NULL )
    mpStateVector->SetStateValue( mLocation, mLength, inValue );
  return *this;
}

inline
StateRef::operator long() const
{
  long value = mDefaultValue;
  if( mpStateVector != NULL )
    value = mpStateVector->StateValue( mLocation, mLength );
  return value;
}

#endif // STATE_REF_H
