/////////////////////////////////////////////////////////////////////////////
//
// File: StateAccessor.h
//
// Date: Oct 25, 2001
//
// Author: Juergen Mellinger
//
// Description: A class that caches state information to avoid parsing
//              of state names during time critical tasks.
//
// Changes: Feb 02, 2004, jm: Introduced "optional state" concept to
//              to improve BCI2000 compatibility.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef STATEACCESSORH
#define STATEACCESSORH

#include <cassert>

class STATEVECTOR;

class TStateAccessor
{
  public:
                    // Default constructor
                    TStateAccessor();
                    // Copy constructor
                    TStateAccessor( const TStateAccessor    &inStateAccessor );
                    TStateAccessor( const char              *inStateName );
                    ~TStateAccessor();

    void            AttachState( const char *inStateName );
    void            AttachOptionalState( const char*    inStateName,
                                         unsigned short inDefaultValue );

    // Value accessors
    unsigned short  GetStateValue() const;
    void            SetStateValue( unsigned short inValue );

    static void     Initialize( STATEVECTOR *inStateVector );
    static STATEVECTOR* GetStateVector();

  private:
            int            bitLocation;
            int            byteLocation;
            int            length;
            unsigned short mDefaultValue;
            bool           mStateAvailable;

    static  STATEVECTOR *stateVector;
};

inline
TStateAccessor::TStateAccessor()
: bitLocation( 0 ),
  byteLocation( 0 ),
  length( 0 ),
  mStateAvailable( false ),
  mDefaultValue( 0 )
{
}

inline
TStateAccessor::TStateAccessor( const TStateAccessor& inStateAccessor )
: bitLocation( inStateAccessor.bitLocation ),
  byteLocation( inStateAccessor.byteLocation ),
  length( inStateAccessor.length ),
  mStateAvailable( inStateAccessor.mStateAvailable ),
  mDefaultValue( inStateAccessor.mDefaultValue )
{
}

inline
TStateAccessor::TStateAccessor( const char  *inStateName )
{
    AttachState( inStateName );
}

inline
TStateAccessor::~TStateAccessor()
{
}

inline
STATEVECTOR*
TStateAccessor::GetStateVector()
{
    assert( stateVector != NULL );
    return stateVector;
}

#endif // STATE_ACCESSOR_H
