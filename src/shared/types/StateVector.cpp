////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for the binary representation of a list of
//   states (event markers).
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "StateVector.h"

#include <sstream>
#include <cassert>
#include <iomanip>

using namespace std;

// **************************************************************************
// Function:   Initialize
// Purpose:    It initializes the state vector with the values of each state in the list
// Parameters: use_assigned_positions = true  ... use the Byte/Bit of the states for the statevector
//                                      false ... calculate the byte and bit position of each state in the statevector
// Returns:    N/A
// **************************************************************************
void
StateVector::Initialize( bool inUseAssignedPositions )
{
  delete[] mpData;
  mpData = NULL;

  // calculate the state vector length
  int bitLength = 0;
  for( int i = 0; i < mpStateList->Size(); ++i )
  {
    if( !inUseAssignedPositions )
      ( *mpStateList )[ i ].SetLocation( bitLength );
    bitLength += ( *mpStateList )[ i ].Length();
  }
  mByteLength = ( bitLength / 8 ) + 1; // divide it by eight, add one to get the number of needed bytes
  mpData = new unsigned char[ mByteLength ];
  // at the very beginning, initialize the state vector to all 0 bytes
  for( size_t i = 0; i < mByteLength; ++i )
    mpData[ i ] = 0;

  // initialize the content in the state vector, according to the content
  // of the current states in the state list
  for( int i = 0; i < mpStateList->Size(); ++i )
    SetStateValue( ( *mpStateList )[ i ].Name(), ( *mpStateList )[ i ].Value() );
}

StateVector::StateVector( const StateVector& s )
: mByteLength( 0 ),
  mpData( NULL ),
  mpStateList( NULL )
{
  this->operator=( s );
}

// can specify, whether or not defined byte/bit positions should be used
// true: use already specified ones
StateVector::StateVector( class StateList& inList, bool inUsePositions )
: mByteLength( 0 ),
  mpData( NULL ),
  mpStateList( &inList )
{
  Initialize( inUsePositions );
}

StateVector::~StateVector()
{
  delete[] mpData;
}

// **************************************************************************
// Function:   operator=
// Purpose:    Make a deep copy of a StateVector object.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
const StateVector&
StateVector::operator=( const StateVector& s )
{
  if( &s != this )
  {
    mByteLength = s.mByteLength;
    mpStateList = s.mpStateList;
    delete[] mpData;
    mpData = new unsigned char[ mByteLength ];
    ::memcpy( mpData, s.mpData, mByteLength );
  }
  return *this;
}

// **************************************************************************
// Function:   GetStateValue
// Purpose:    returns a state's value, based upon the state's location and size
// Parameters: location ... bit location of the state
//             length   ... bit length of the state
// Returns:    the value of the state
// **************************************************************************
State::ValueType
StateVector::StateValue( size_t inLocation, size_t inLength ) const
{
  if( inLength > 8 * sizeof( State::ValueType ) )
    throw "Invalid state length";
  if( inLocation + inLength > 8 * mByteLength )
    throw "Accessing non-existent state vector data";

  State::ValueType result = 0;
  for( int bitIndex = inLocation + inLength - 1; bitIndex >= int( inLocation ); --bitIndex )
  {
    result <<= 1;
    if( mpData[ bitIndex / 8 ] & ( 1 << ( bitIndex % 8 ) ) )
      result |= 1;
  }
  return result;
}


// **************************************************************************
// Function:   StateValue
// Purpose:    returns a state's value from the state vector
// Parameters: statename - the name of a state
// Returns:    the value of the state
//             0 on error (e.g., state not found)
// **************************************************************************
State::ValueType
StateVector::StateValue( const string& inName ) const
{
  State::ValueType result = 0;
  if( mpStateList->Exists( inName ) )
  {
    State& s = ( *mpStateList )[ inName ];
    result = StateValue( s.Location(), s.Length() );
  };
  return result;
}


// **************************************************************************
// Function:   SetStateValue
// Purpose:    sets a state's value in the state vector
// Parameters: location ... bit location of the state
//             length   ... bit length of the state
//             value   ... value of the state
// Returns:    N/A
// **************************************************************************
void
StateVector::SetStateValue( size_t inLocation, size_t inLength, State::ValueType inValue )
{
  if( inValue < 0 || ( inValue & ~( -1 << inLength ) ) != inValue )
    throw "Value exceeds limit given by state length";
  if( inLength > 8 * sizeof( State::ValueType ) )
    throw "Invalid state length";
  if( inLocation + inLength > 8 * mByteLength )
    throw "Accessing non-existent state vector data";

  State::ValueType value = inValue;
  for( size_t bitIndex = inLocation; bitIndex < inLocation + inLength; ++bitIndex )
  {
    unsigned char mask = 1 << ( bitIndex % 8 );
    if( value & 1 )
      mpData[ bitIndex / 8 ] |= mask;
    else
      mpData[ bitIndex / 8 ] &= ~mask;
    value >>= 1;
  }
}

// **************************************************************************
// Function:   SetStateValue
// Purpose:    sets a state's value in the state vector
// Parameters: statename - name of the state
// Returns:    N/A
// **************************************************************************
void
StateVector::SetStateValue( const string& inName, State::ValueType inValue )
{
  if( mpStateList->Exists( inName ) )
  {
    State& s = ( *mpStateList )[ inName ];
    SetStateValue( s.Location(), s.Length(), inValue );
  };
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
  if( !mpStateList->Exists( inName ) )
    throw "PostStateChange() called for undeclared state";

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
  for( int i = 0; i < mpStateList->Size(); ++i )
    ( *mpStateList )[ i ].Commit( this );
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted output of a state vector
//             into a stream.
// Parameters: Output stream to write into.
// Returns:    Output stream.
// **************************************************************************
ostream&
StateVector::WriteToStream( ostream& os ) const
{
  int indent = os.width();
  if( mpStateList == NULL )
    for( size_t i = 0; i < mByteLength; ++i )
      os << '\n' << setw( indent ) << ""
         << i << ": "
         << mpData[ i ];
  else
    for( int i = 0; i < mpStateList->Size(); ++i )
    {
      const State& state = ( *mpStateList )[ i ];
      os << '\n' << setw( indent ) << ""
         << state.Name() << ": "
         << StateValue( state.Location(), state.Length() );
    }
  return os;
}

// **************************************************************************
// Function:   ReadBinary
// Purpose:    Member function for input of a state vector
//             from a binary stream, as in a state vector message.
// Parameters: Input stream to read from.
// Returns:    Input stream.
// **************************************************************************
istream&
StateVector::ReadBinary( istream& is )
{
  // Reading the last byte with is.get() avoids possible problems with tcp
  // stream buffers. See the comments in TCPStream.cpp, tcpbuf::underflow()
  // for details.
  if( mByteLength > 1 )
    is.read( reinterpret_cast<istream::char_type*>( mpData ), mByteLength - 1 );
  mpData[ mByteLength - 1 ] = is.get();
  return is;
}

// **************************************************************************
// Function:   WriteBinary
// Purpose:    Member function for output of a state vector
//             into a binary stream, as in a state vector message.
// Parameters: Output stream to write into.
// Returns:    Output stream.
// **************************************************************************
ostream&
StateVector::WriteBinary( ostream& os ) const
{
  return os.write( reinterpret_cast<istream::char_type*>( mpData ), mByteLength );
}



