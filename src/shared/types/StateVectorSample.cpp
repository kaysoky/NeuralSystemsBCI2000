////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for the binary representation of a list of
//   state variables corresponding to a single sample.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "StateVectorSample.h"

#include <sstream>
#include <cassert>
#include <iomanip>

using namespace std;

StateVectorSample::StateVectorSample( const StateVectorSample& s )
: mByteLength( 0 ),
  mpData( NULL )
{
  this->operator=( s );
}

StateVectorSample::StateVectorSample( size_t inByteLength )
: mByteLength( inByteLength ),
  mpData( NULL )
{
  mpData = new unsigned char[ mByteLength ];
  // at the very beginning, initialize the state vector to all 0 bytes
  for( size_t i = 0; i < mByteLength; ++i )
    mpData[ i ] = 0;
}

StateVectorSample::~StateVectorSample()
{
  delete[] mpData;
}

// **************************************************************************
// Function:   operator=
// Purpose:    Make a deep copy of a StateVectorSample object.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
const StateVectorSample&
StateVectorSample::operator=( const StateVectorSample& s )
{
  if( &s != this )
  {
    mByteLength = s.mByteLength;
    delete[] mpData;
    mpData = new unsigned char[ mByteLength ];
    ::memcpy( mpData, s.mpData, mByteLength );
  }
  return *this;
}

// **************************************************************************
// Function:   StateValue
// Purpose:    returns a state's value, based upon the state's location and size
// Parameters: location ... bit location of the state
//             length   ... bit length of the state
// Returns:    the value of the state
// **************************************************************************
State::ValueType
StateVectorSample::StateValue( size_t inLocation, size_t inLength ) const
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
// Function:   SetStateValue
// Purpose:    sets a state's value in the state vector
// Parameters: location ... bit location of the state
//             length   ... bit length of the state
//             value   ... value of the state
// Returns:    N/A
// **************************************************************************
void
StateVectorSample::SetStateValue( size_t inLocation, size_t inLength, State::ValueType inValue )
{
  State::ValueType valueMask = ( -1 >> ( 8* sizeof( State::ValueType ) - inLength ) );
  if( inValue < 0 || ( inValue & valueMask ) != inValue )
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
// Function:   ReadBinary
// Purpose:    Member function for input of a state vector
//             from a binary stream, as in a state vector message.
// Parameters: Input stream to read from.
// Returns:    Input stream.
// **************************************************************************
istream&
StateVectorSample::ReadBinary( istream& is )
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
StateVectorSample::WriteBinary( ostream& os ) const
{
  return os.write( reinterpret_cast<istream::char_type*>( mpData ), mByteLength );
}



