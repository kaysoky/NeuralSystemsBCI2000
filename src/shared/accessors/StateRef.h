//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that holds references to state values, and
//         allows for convenient automatic type
//         conversions when accessing state values.
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

  class StateRefFloat  AsFloat();
  class StateRefSigned AsSigned();
  StateRef&            AsUnsigned();

 private:
  const State* mpState;
  StateVector* mpStateVector;
  int          mSample;
  long         mDefaultValue;
};

class StateRefFloat
{
 public:
  StateRefFloat( StateRef& );
  const StateRefFloat& operator=( float );
  operator float() const;

 private:
  StateRef& mrStateRef;
};

class StateRefSigned
{
 public:
  StateRefSigned( StateRef& );
  const StateRefSigned& operator=( long );
  operator long() const;

 private:
  StateRef& mrStateRef;
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
  return StateRef( mpState, mpStateVector, static_cast<int>( mSample + inOffset ), mDefaultValue );
}

inline
const State*
StateRef::operator->() const
{
  return mpState;
}

inline
StateRefFloat
StateRef::AsFloat()
{
  return StateRefFloat( *this );
}

inline
StateRefSigned
StateRef::AsSigned()
{
  return StateRefSigned( *this );
}

inline
StateRef&
StateRef::AsUnsigned()
{
  return *this;
}

inline
StateRefFloat::StateRefFloat( StateRef& inStateRef )
: mrStateRef( inStateRef )
{}

inline
const StateRefFloat&
StateRefFloat::operator=( float inF )
{
  union { float f; long l; } value;
  value.f = inF;
  mrStateRef = value.l;
  return *this;
}

inline
StateRefFloat::operator float() const
{
  union { float f; long l; } value;
  value.l = mrStateRef;
  return value.f;
}

inline
StateRefSigned::StateRefSigned( StateRef& inStateRef )
: mrStateRef( inStateRef )
{}

inline
const StateRefSigned&
StateRefSigned::operator=( long inL )
{
  const int bitsPerByte = 8;
  if( mrStateRef->Length() == static_cast<int>( sizeof( 1L ) * bitsPerByte ) )
    mrStateRef = inL;
  else
    mrStateRef = ( inL & ( ( 1L << mrStateRef->Length() ) - 1L ) );
  return *this;
}

inline
StateRefSigned::operator long() const
{
  long result = mrStateRef;
  // Interpret the most significant bit as a sign bit, and
  // extend it to all leading bits in the result.
  if( mrStateRef & 1L << ( mrStateRef->Length() - 1 ) )
    result |= -1L << mrStateRef->Length();
  return result;
}

#endif // STATE_REF_H
