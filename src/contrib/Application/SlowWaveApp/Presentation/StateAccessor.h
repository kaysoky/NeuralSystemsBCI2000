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
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef STATEACCESSORH
#define STATEACCESSORH

#include <cassert>

class StateVector;

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

    static void     Initialize( StateVector *inStateVector );
    static StateVector* GetStateVector();

  private:
            std::string    mName;
            unsigned short mDefaultValue;
            bool           mStateAvailable;

    static  StateVector *stateVector;
};

inline
TStateAccessor::TStateAccessor()
: mStateAvailable( false ),
  mDefaultValue( 0 )
{
}

inline
TStateAccessor::TStateAccessor( const TStateAccessor& inStateAccessor )
: mName( inStateAccessor.mName ),
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
StateVector*
TStateAccessor::GetStateVector()
{
    assert( stateVector != NULL );
    return stateVector;
}

#endif // STATE_ACCESSOR_H
