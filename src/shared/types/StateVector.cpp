////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for the the binary representation of states
//  (event markers) for an entire data block.
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
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "StateVector.h"

#include "StateList.h"
#include "BCIException.h"
#include "BCIAssert.h"

#include <sstream>
#include <iomanip>

using namespace std;

StateVector::StateVector()
: mpStateList( NULL )
{
}

StateVector::StateVector( class StateList& inList, size_t inSamples )
: mpStateList( &inList )
{
  mSamples.resize( inSamples, StateVectorSample( mpStateList->ByteLength() ) );
  // initialize the content in the state vector, according to the content
  // of the current states in the state list
  for( int i = 0; i < mpStateList->Size(); ++i )
    SetStateValue( ( *mpStateList )[ i ].Name(), ( *mpStateList )[ i ].Value() );
}

const StateVector&
StateVector::CopyFromMasked( const StateVector& inVector, const StateVectorSample& inMask )
{
  if( &inVector == this )
    return *this;
  bciassert( this->Samples() == inVector.Samples() );
  for( int i = 0; i < this->Samples(); ++i )
    ( *this )( i ).CopyFromMasked( inVector( i ), inMask );
  return *this;
}

// **************************************************************************
// Function:   StateValue
// Purpose:    returns a state's value from the state vector block
// Parameters: statename - the name of a state
// Returns:    the value of the state
//             0 on error (e.g., state not found)
// **************************************************************************
State::ValueType
StateVector::StateValue( const string& inName, size_t inSample ) const
{
  State::ValueType result = 0;
  if( mpStateList && mpStateList->Exists( inName ) )
  {
    const State& s = ( *mpStateList )[ inName ];
    result = mSamples[ inSample ].StateValue( s.Location(), s.Length() );
  };
  return result;
}

// **************************************************************************
// Function:   StateValue
// Purpose:    returns a state's value, based upon the state's location and
//             size
// Parameters: location ... bit location of the state
//             length   ... bit length of the state
//             sample   ... sample position for which to return the state's
//                          value
// Returns:    the value of the state
// **************************************************************************
State::ValueType
StateVector::StateValue( size_t inLocation, size_t inLength, size_t inSample ) const
{
  return mSamples[ inSample ].StateValue( inLocation, inLength );
}

// **************************************************************************
// Function:   SetStateValue
// Purpose:    sets a state's value in the state vector block
// Parameters: statename - name of the state
// Returns:    N/A
// **************************************************************************
void
StateVector::SetStateValue( const string& inName, size_t inSample, State::ValueType inValue )
{
  if( mpStateList && mpStateList->Exists( inName ) )
  {
    const State& s = ( *mpStateList )[ inName ];
    SetStateValue( s.Location(), s.Length(), inSample, inValue );
  }
}

// **************************************************************************
// Function:   SetStateValue
// Purpose:    sets a state's value in the state vector
// Parameters: location ... bit location of the state
//             length   ... bit length of the state
//             sample   ... sample position from which to set the state value
//             value    ... value of the state
// Returns:    N/A
// **************************************************************************
void
StateVector::SetStateValue( size_t inLocation, size_t inLength, size_t inSample, State::ValueType inValue )
{
  for( int i = static_cast<int>( inSample ); i < Samples(); ++i )
    mSamples[ i ].SetStateValue( inLocation, inLength, inValue );
}

// **************************************************************************
// Function:   PostStateChange
// Purpose:    Have a state changed upon the next call to CommitStateChanges().
// Parameters: Name of the state to be changed; requested value
// Returns:    N/A
// **************************************************************************
void
StateVector::PostStateChange( const string& inName, State::ValueType inValue )
{
  if( !mpStateList || !mpStateList->Exists( inName ) )
    throw bciexception( "Undeclared state " << inName );

  ( *mpStateList )[ inName ].SetValue( inValue ); // We use State::value as a buffer.
}

// **************************************************************************
// Function:   CommitStateChanges
// Purpose:    Have all states commit their changes, if any.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
StateVector::CommitStateChanges()
{
  if( mpStateList )
    for( int i = 0; i < mpStateList->Size(); ++i )
      ( *mpStateList )[ i ].Commit( this );
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted output of a state vector block
//             into a stream.
// Parameters: Output stream to write into.
// Returns:    Output stream.
// **************************************************************************
ostream&
StateVector::WriteToStream( ostream& os ) const
{
  streamsize indent = os.width();
  if( mpStateList == NULL )
    for( int i = 0; i < Length(); ++i )
    {
      os << '\n' << setw( indent ) << ""
         << i << ":";
      for( int j = 0; j < Samples(); ++j )
        os << " " << mSamples[ j ].Data()[ i ];
    }
  else
    for( int i = 0; i < mpStateList->Size(); ++i )
    {
      const State& state = ( *mpStateList )[ i ];
      os << '\n' << setw( indent ) << ""
         << state.Name() << ":";
      for( int j = 0; j < Samples(); ++j )
        os << " " << mSamples[ j ].StateValue( state.Location(), state.Length() );
    }
  return os;
}

// **************************************************************************
// Function:   ReadBinary
// Purpose:    Member function for input of a state vector block
//             from a binary stream, as in a state vector message.
// Parameters: Input stream to read from.
// Returns:    Input stream.
// **************************************************************************
istream&
StateVector::ReadBinary( istream& is )
{
  int length, samples;
  ( is >> length ).get();
  ( is >> samples ).get();
  if( length != Length() )
    mSamples.clear();
  if( samples != Samples() )
    mSamples.resize( samples, StateVectorSample( length ) );
  for( size_t i = 0; i < mSamples.size(); ++i )
    mSamples[ i ].ReadBinary( is );
  return is;
}

// **************************************************************************
// Function:   WriteBinary
// Purpose:    Member function for output of a state vector block
//             into a binary stream, as in a state vector message.
// Parameters: Output stream to write into.
// Returns:    Output stream.
// **************************************************************************
ostream&
StateVector::WriteBinary( ostream& os ) const
{
  ( os << Length() ).put( '\0' );
  ( os << Samples() ).put( '\0' );
  for( size_t i = 0; i < mSamples.size(); ++i )
    mSamples[ i ].WriteBinary( os );
  return os;
}



