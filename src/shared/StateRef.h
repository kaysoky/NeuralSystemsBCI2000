//////////////////////////////////////////////////////////////////////
// $Id$
//
// File: StateRef.h
//
// Date: Jan 10, 2006
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// $Log$
// Revision 1.2  2006/01/12 20:19:18  mellinger
// Various fixes.
//
// Revision 1.1  2006/01/11 19:07:28  mellinger
// Revision of interface style to match corresponding parameter classes.
//
//
// Description: A class that holds references to state values, and
//         allows for convenient automatic type
//         conversions when accessing state values.
//
///////////////////////////////////////////////////////////////////////
#ifndef StateRefH
#define StateRefH

#include "UState.h"

class StateRef
{
 private:
  StateRef& operator=( const StateRef& );
 public:
  StateRef();
  StateRef( STATEVECTOR*,
            size_t location, size_t length,
            long defaultValue = 0 );
  const StateRef& operator=( long );
  operator long() const;

 private:
  STATEVECTOR* mpStatevector;
  size_t       mLocation,
               mLength;
  long         mDefaultValue;
};


inline
StateRef::StateRef()
: mpStatevector( NULL ),
  mLocation( 0 ),
  mLength( 0 ),
  mDefaultValue( 0 )
{
}

inline
StateRef::StateRef( STATEVECTOR* inStatevector,
                    size_t inLocation,
                    size_t inLength,
                    long inDefaultValue )
: mpStatevector( inStatevector ),
  mLocation( inLocation ),
  mLength( inLength ),
  mDefaultValue( inDefaultValue )
{
}

inline
const StateRef&
StateRef::operator=( long inValue )
{
  if( mpStatevector != NULL )
    mpStatevector->SetStateValue( mLocation, mLength, inValue );
  return *this;
}

inline
StateRef::operator long() const
{
  long value = mDefaultValue;
  if( mpStatevector != NULL )
    value = mpStatevector->GetStateValue( mLocation, mLength );
  return value;
}

#endif // StateRefH
