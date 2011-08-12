////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for the binary representation of a list of
//   state variables corresponding to a single sample.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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

#include "StateVectorSample.h"
#include "BCIException.h"

#include <sstream>
#include <iomanip>
#include <climits>

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
    throw bciexception( "Invalid state length: " << inLength );
  if( inLocation + inLength > 8 * mByteLength )
    throw bciexception( "Accessing non-existent state vector data, location: " << inLocation );

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
  State::ValueType valueMask = ( ULONG_MAX >> ( 8* sizeof( State::ValueType ) - inLength ) );
  if( inValue < 0 || ( inValue & valueMask ) != inValue )
    throw bciexception(
      "Illegal value " 
      << inValue
      << " was passed to "
      << inLength
      << "-bit state at address "
      << inLocation
      );
  if( inLength > 8 * sizeof( State::ValueType ) )
    throw bciexception( "Invalid state length of " << inLength );
  if( inLocation + inLength > 8 * mByteLength )
    throw bciexception( "Accessing non-existent state vector data, location: " << inLocation );

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



