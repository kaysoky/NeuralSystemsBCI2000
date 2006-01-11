/******************************************************************************
 * $Id$                                                                       *
 * Program:   BCI2000                                                         *
 * Module:    UState.cpp                                                      *
 * Comment:   This unit provides support for system-wide states,              *
 *            lists of states, and the state vector                           *
 * Version:   0.10                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.08 - 03/30/2000 - First commented version                               *
 * V0.09 - 07/22/2003 - Replaced VCL's TList with STL's std::vector<>         *
 *                      to avoid linking against the VCL in command line      *
 *                      tools using the STATELIST class, jm                   *
 * V0.10 - 07/24/2003 - Introduced stream based i/o, jm                       *
 * $Log$
 * Revision 1.18  2006/01/11 19:07:28  mellinger
 * Revision of interface style to match corresponding parameter classes.
 *
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "UState.h"

#include <assert>
#include <sstream>
#include <iomanip>
#include <mem.h>

using namespace std;

// **************************************************************************
// Function:   operator[]
// Purpose:    Access a state by its name.
// Parameters: State name.
// Returns:    Returns a reference to a state with a given name.
// **************************************************************************
STATE&
STATELIST::operator[]( const std::string& inName )
{
  state_index::const_iterator i = mIndex.find( inName );
  if( i == mIndex.end() )
  {
    mIndex[ inName ] = size();
    resize( size() + 1 );
    i = mIndex.find( inName );
  }
  return state_container::operator[]( i->second );
}

const STATE&
STATELIST::operator[]( const std::string& inName ) const
{
  static STATE defaultState;
  const STATE* result = &defaultState;
  state_index::const_iterator i = mIndex.find( inName );
  if( i != mIndex.end() )
    result = &state_container::operator[]( i->second );
  return *result;
}
// **************************************************************************
// Function:   Clear
// Purpose:    Clear all entries from the state list.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
STATELIST::Clear()
{
  clear();
  RebuildIndex();
}

// **************************************************************************
// Function:   Add
// Purpose:    adds a new state to the list of states
//             if a state with the same name already exists,
//             it updates the currently stored state with the provided values
// Parameters: statestring - ASCII string, as defined in project description,
//                           defining this new state
// Returns:    N/A
// **************************************************************************
bool
STATELIST::Add( const string& inDefinition )
{
  istringstream line( inDefinition );
  STATE s;
  if( line >> s )
  {
    if( Exists( s.mName ) )
      ( *this )[ s.mName ].SetValue( s.GetValue() );
    else
      Add( s );
  }
  return line;
}

// **************************************************************************
// Function:   Delete
// Purpose:    deletes a state of a given name in the list of states
//             it also frees the memory for this particular state
//             it does not do anything, if the state does not exist
// Parameters: name - name of the state
// Returns:    N/A
// **************************************************************************
void
STATELIST::Delete( const string& inName )
{
  if( mIndex.find( inName ) != mIndex.end() )
  {
    erase( &at( mIndex[ inName ] ) );
    RebuildIndex();
  }
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of the entire
//             state list.
//             For partial output, use another instance of type STATELIST
//             to hold the desired subset.
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
STATELIST::WriteToStream( ostream& os ) const
{
  for( size_t i = 0; i < Size(); ++i )
    os << ( *this )[ i ] << '\n';
  return os;
}

// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of the entire
//             state list. The list is cleared before reading.
//             For partial input, use another instance of type STATELIST
//             to hold the desired subset.
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
STATELIST::ReadFromStream( istream& is )
{
  Clear();
  STATE state;
  is >> ws;
  while( !is.eof() && is >> state >> ws )
    Add( state );
  return is;
}

// **************************************************************************
// Function:   WriteBinary
// Purpose:    Member function for binary stream output of the entire
//             state list.
//             For partial output, use another instance of type STATELIST
//             to hold the desired subset.
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
STATELIST::WriteBinary( ostream& os ) const
{
  for( size_t i = 0; i < Size(); ++i )
    ( *this )[ i ].WriteBinary( os );
  return os;
}

// **************************************************************************
// Function:   ReadBinary
// Purpose:    Member function for binary stream input of the entire
//             state list. The list is cleared before reading.
//             For partial input, use another instance of type STATELIST
//             to hold the desired subset.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
STATELIST::ReadBinary( istream& is )
{
  return ReadFromStream( is );
}

// **************************************************************************
// Function:   RebuildIndex
// Purpose:    Rebuilds the name-to-position lookup index.
//             This function must be called after any change to the state
//             container.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
STATELIST::RebuildIndex()
{
  mIndex.clear();
  for( size_t i = 0; i < size(); ++i )
    mIndex[ operator[]( i ).mName ] = i;
}

// **************************************************************************
// Function:   STATE
// Purpose:    the constructor for the STATE object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
STATE::STATE()
: mValue( 0 ),
  mLocation( 0 ),
  mLength( 0 ),
  mModified( false )
{
}

// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of a single
//             state.
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
STATE::ReadFromStream( istream& is )
{
  *this = STATE();
  mModified = true;
  int byteLocation = 0,
      bitLocation = 0;
  is >> mName >> mLength >> mValue >> byteLocation >> bitLocation;
  SetByteLocation( byteLocation );
  SetBitLocation( bitLocation );
  return is;
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of a single
//             state.
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
STATE::WriteToStream( ostream& os ) const
{
  return os << mName << " "
            << mLength << " "
            << mValue << " "
            << GetByteLocation() << " "
            << GetBitLocation();
}

// **************************************************************************
// Function:   ReadBinary
// Purpose:    Member function for input of a single
//             state from a binary stream, as in a state message.
// Parameters: Input stream to read from.
// Returns:    N/A
// **************************************************************************
istream&
STATE::ReadBinary( istream& is )
{
  string line;
  if( getline( is, line, '\n' ) )
  {
    istringstream linestream( line );
    ReadFromStream( linestream );
    if( !linestream )
      is.setstate( ios::failbit );
  }
  return is;
}

// **************************************************************************
// Function:   WriteBinary
// Purpose:    Member function for output of a single
//             state into a binary stream, as in a state message.
// Parameters: Output stream to write into.
// Returns:    N/A
// **************************************************************************
ostream&
STATE::WriteBinary( ostream& os ) const
{
  WriteToStream( os );
  return os.write( "\r\n", 2 );
}

// **************************************************************************
// Function:   SetValue
// Purpose:    Sets this state's value
// Parameters: value
// Returns:    N/A
// **************************************************************************
void
STATE::SetValue( STATE::value_type inValue )
{
 mValue = inValue;
 mModified = true;
}

// **************************************************************************
// Function:   STATE::Commit
// Purpose:    if the write cache has been modified, write its content into
//             the given state vector and clear the "modified" flag
// Parameters: stateVector - the state vector that calls this function
// Returns:    N/A
// **************************************************************************
void
STATE::Commit( STATEVECTOR* stateVector )
{
  if( mModified )
  {
    assert( stateVector != NULL );
    stateVector->SetStateValue( mLocation, mLength, mValue );
    mModified = false;
  }
}

// **************************************************************************
// Function:   Initialize
// Purpose:    It initializes the state vector with the values of each state in the list
// Parameters: use_assigned_positions = true  ... use the Byte/Bit of the states for the statevector
//                                      false ... calculate the byte and bit position of each state in the statevector
// Returns:    N/A
// **************************************************************************
void
STATEVECTOR::Initialize( bool inUseAssignedPositions )
{
  delete[] mpData;
  mpData = NULL;

  // calculate the state vector length
  int bitLength = 0;
  for( size_t i = 0; i < mrStatelist.Size(); ++i )
  {
    if( !inUseAssignedPositions )
      mrStatelist[ i ].SetLocation( bitLength );
    bitLength += mrStatelist[ i ].GetLength();
  }
  mByteLength = ( bitLength / 8 ) + 1; // divide it by eight, add one to get the number of needed bytes
  mpData = new unsigned char[ mByteLength ];
  // at the very beginning, initialize the state vector to all 0 bytes
  for( size_t i = 0; i < mByteLength; ++i )
    mpData[ i ] = 0;

  // initialize the content in the state vector, according to the content
  // of the current states in the state list
  for( size_t i = 0; i < mrStatelist.Size(); ++i )
    SetStateValue( mrStatelist[ i ].GetName(), mrStatelist[ i ].GetValue() );
}

STATEVECTOR::STATEVECTOR( const STATEVECTOR& s )
: mByteLength( 0 ),
  mpData( NULL ),
  mrStatelist( s.mrStatelist )
{
  this->operator =( s );
}

// can specify, whether or not defined byte/bit positions should be used
// true: use already specified ones
STATEVECTOR::STATEVECTOR( STATELIST& inList, bool inUsepositions )
: mByteLength( 0 ),
  mpData( NULL ),
  mrStatelist( inList )
{
  Initialize( inUsepositions );
}

STATEVECTOR::STATEVECTOR( STATELIST* inList, bool inUsepositions )
: mByteLength( 0 ),
  mpData( NULL ),
  mrStatelist( *inList )
{
  Initialize( inUsepositions );
}

STATEVECTOR::~STATEVECTOR()
{
  delete[] mpData;
}

// **************************************************************************
// Function:   operator=
// Purpose:    Make a deep copy of a STATEVECTOR object.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
const STATEVECTOR&
STATEVECTOR::operator=( const STATEVECTOR& s )
{
  if( &s != this )
  {
    delete[] mpData;
    *this = s;
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
STATE::value_type
STATEVECTOR::GetStateValue( size_t inLocation, size_t inLength ) const
{
  if( inLocation + inLength > 8 * mByteLength )
    throw "Accessing non-existent state vector data";

  STATE::value_type result = 0;
  for( size_t bitIndex = inLocation; bitIndex < inLocation + inLength; ++bitIndex )
  {
    result <<= 1;
    if( ( mpData[ bitIndex / 8 ] & 1 << ( bitIndex % 8 ) ) != 0 )
      result |= 1;
  }
  return result;
}


// **************************************************************************
// Function:   GetStateValue
// Purpose:    returns a state's value from the state vector
// Parameters: statename - the name of a state
// Returns:    the value of the state
//             0 on error (e.g., state not found)
// **************************************************************************
STATE::value_type
STATEVECTOR::GetStateValue( const string& inName ) const
{
  STATE::value_type result = 0;
  if( mrStatelist.Exists( inName ) )
  {
    STATE& s = mrStatelist[ inName ];
    result = GetStateValue( s.GetLocation(), s.GetLength() );
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
STATEVECTOR::SetStateValue( size_t inLocation, size_t inLength, STATE::value_type inValue )
{
  if( inLocation + inLength > 8 * mByteLength )
    throw "Accessing non-existent state vector data";

  STATE::value_type value = inValue;
  for( size_t bitIndex = inLocation; bitIndex < inLocation + inLength; ++bitIndex )
  {
    if( value & 1 )
      mpData[ bitIndex / 8 ] |= 1 << ( bitIndex % 8 );
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
STATEVECTOR::SetStateValue( const string& inName, STATE::value_type inValue )
{
  if( mrStatelist.Exists( inName ) )
  {
    STATE& s = mrStatelist[ inName ];
    SetStateValue( s.GetLocation(), s.GetLength(), inValue );
  };
}

// **************************************************************************
// Function:   PostStateChange
// Purpose:    Have a state changed upon the next call to CommitStateChanges().
// Parameters: Name of the state to be changed; requested value
// Returns:    N/A
// **************************************************************************
void
STATEVECTOR::PostStateChange( const string& inName, unsigned short inValue )
{
  if( !mrStatelist.Exists( inName ) )
    throw __FUNC__ " called for undeclared state";

  mrStatelist[ inName ].SetValue( inValue ); // We use STATE::value as a buffer.
}

// **************************************************************************
// Function:   CommitStateChanges
// Purpose:    Have all states commit their changes, if any.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
STATEVECTOR::CommitStateChanges()
{
  for( size_t i = 0; i < mrStatelist.Size(); ++i )
    mrStatelist[ i ].Commit( this );
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted output of a state vector
//             into a stream.
// Parameters: Output stream to write into.
// Returns:    Output stream.
// **************************************************************************
ostream&
STATEVECTOR::WriteToStream( ostream& os ) const
{
  int indent = os.width();
  for( size_t i = 0; i < mrStatelist.Size(); ++i )
  {
    const STATE& state = mrStatelist[ i ];
    os << '\n' << setw( indent ) << ""
       << state.GetName() << ": "
       << GetStateValue( state.GetLocation(), state.GetLength() );
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
STATEVECTOR::ReadBinary( istream& is )
{
  // Reading the last byte with is.get() avoids possible problems with tcp
  // stream buffers. See the comments in TCPStream.cpp, tcpbuf::underflow()
  // for details.
  if( mByteLength > 1 )
    is.read( mpData, mByteLength - 1 );
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
STATEVECTOR::WriteBinary( ostream& os ) const
{
  return os.write( mpData, mByteLength );
}



