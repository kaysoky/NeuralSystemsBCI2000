////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for system states (event markers).
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "State.h"
#include "StateVector.h"

#include <sstream>
#include <map>
#include <cassert>

using namespace std;

// **************************************************************************
// Function:   State
// Purpose:    the constructor for the State object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
State::State()
: mValue( 0 ),
  mLocation( 0 ),
  mLength( 0 ),
  mModified( false )
{
}

// **************************************************************************
// Function:   operator==
// Purpose:    Compare to another State object.
// Parameters: Reference to second State.
// Returns:    True if equal, false otherwise.
// **************************************************************************
bool
State::operator==( const State& s ) const
{
  return this->Name() == s.Name()
      && this->Location() == s.Location()
      && this->Length() == s.Length()
      && this->Value() == s.Value();
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
State::ReadFromStream( istream& is )
{
  *this = State();
  mModified = true;
  int byteLocation = 0,
      bitLocation = 0;
  is >> mName >> mLength >> mValue >> byteLocation >> bitLocation;
  if( mLength > 8 * sizeof( ValueType ) )
    throw "State length exceeds size of State::ValueType";
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
State::WriteToStream( ostream& os ) const
{
  return os << mName << " "
            << mLength << " "
            << mValue << " "
            << ByteLocation() << " "
            << BitLocation();
}

// **************************************************************************
// Function:   ReadBinary
// Purpose:    Member function for input of a single
//             state from a binary stream, as in a state message.
// Parameters: Input stream to read from.
// Returns:    N/A
// **************************************************************************
istream&
State::ReadBinary( istream& is )
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
State::WriteBinary( ostream& os ) const
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
State&
State::SetValue( State::ValueType inValue )
{
 mValue = inValue;
 mModified = true;
 return *this;
}

// **************************************************************************
// Function:   State::Commit
// Purpose:    if the write cache has been modified, write its content into
//             the given state vector and clear the "modified" flag
// Parameters: stateVector - the state vector that calls this function
// Returns:    N/A
// **************************************************************************
void
State::Commit( StateVector* stateVector )
{
  if( mModified )
  {
    assert( stateVector != NULL );
    stateVector->SetStateValue( mLocation, mLength, mValue );
    mModified = false;
  }
}


