/////////////////////////////////////////////////////////////////////////////
//
// File: StateAccessor.cpp
//
// Date: Oct 25, 2001
//
// Author: Juergen Mellinger
//
// Description: A class that caches state information to avoid parsing
//              of state names during time critical tasks.
//              Most methods are defined inline in StateAccessor.h.
//
// Changes: Feb 02, 2004, jm: Introduced "optional state" concept to
//              to improve BCI2000 compatibility. Note that optional states
//              cannot be written to.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "StateAccessor.h"
#include "PresErrors.h"
#include "State.h"
#include "StateVector.h"

StateVector*    TStateAccessor::stateVector = NULL;

void
TStateAccessor::Initialize( StateVector *inStateVector )
{
    stateVector = inStateVector;
}

void
TStateAccessor::AttachState( const char *inStateName )
{
    mName = inStateName;
    mStateAvailable = stateVector->StateList().Exists( mName );
    if( !mStateAvailable )
      gPresErrors << "State '" << inStateName << "' inaccessible."
                  << std::endl;
}

void
TStateAccessor::AttachOptionalState( const char *inStateName, unsigned short inDefaultValue )
{
    mName = inStateName;
    mDefaultValue = inDefaultValue;
    mStateAvailable = stateVector->StateList().Exists( mName );
}

unsigned short
TStateAccessor::GetStateValue() const
{
    assert( stateVector != NULL );
    return mStateAvailable ?
            stateVector->StateValue( mName.c_str() ) :
            mDefaultValue;
}

void
TStateAccessor::SetStateValue( unsigned short inValue )
{
    assert( stateVector != NULL );
    assert( mStateAvailable );
    stateVector->SetStateValue( mName.c_str(), inValue );
}

