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
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "StateAccessor.h"
#include "PresErrors.h"
#include "UState.h"

STATEVECTOR*    TStateAccessor::stateVector = NULL;

void
TStateAccessor::Initialize( STATEVECTOR *inStateVector )
{
    stateVector = inStateVector;
}

void
TStateAccessor::AttachState( const char *inStateName )
{
    assert( inStateName != NULL
            && *inStateName != '\0' );
    assert( stateVector != NULL );
    STATELIST   *stateList = stateVector->GetStateListPtr();
    assert( stateList != NULL );
    // STATELIST::GetStatePtr does not declare its argument as const.
    STATE   *state = stateList->GetStatePtr( const_cast<char*>( inStateName ) );
    if( state == NULL )
      gPresErrors << "State '" << inStateName << "' inaccessible."
                  << std::endl;
    else
    {
      bitLocation = state->GetBitLocation();
      byteLocation = state->GetByteLocation();
      length = state->GetLength();
      mStateAvailable = true;
    }
}

void
TStateAccessor::AttachOptionalState( const char *inStateName, unsigned short inDefaultValue )
{
    assert( inStateName != NULL
            && *inStateName != '\0' );
    assert( stateVector != NULL );
    STATELIST   *stateList = stateVector->GetStateListPtr();
    assert( stateList != NULL );
    // STATELIST::GetStatePtr does not declare its argument as const.
    STATE   *state = stateList->GetStatePtr( const_cast<char*>( inStateName ) );
    if( state == NULL )
    {
      mDefaultValue = inDefaultValue;
      mStateAvailable = false;
    }
    else
    {
      bitLocation = state->GetBitLocation();
      byteLocation = state->GetByteLocation();
      length = state->GetLength();
      mStateAvailable = true;
    }
}

unsigned short
TStateAccessor::GetStateValue() const
{
    assert( stateVector != NULL );
    return mStateAvailable ?
            stateVector->GetStateValue( byteLocation, bitLocation, length ) :
            mDefaultValue;
}

void
TStateAccessor::SetStateValue( unsigned short inValue )
{
    assert( stateVector != NULL );
    assert( mStateAvailable );
    stateVector->SetStateValue( byteLocation, bitLocation, length, inValue );
}

