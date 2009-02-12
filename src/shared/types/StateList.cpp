////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for a list of BCI2000 states (event markers).
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "StateList.h"

#include <sstream>

using namespace std;

// **************************************************************************
// Function:   operator==
// Purpose:    Compare to another StateList object.
// Parameters: Reference to second StateList.
// Returns:    True if equal, false otherwise.
// **************************************************************************
bool
StateList::operator==( const StateList& s ) const
{
  bool result = true;
  const_iterator i = begin(),
                 j = s.begin();
  while( result && i != end() && j != s.end() )
    result = result && *i == *j;
  return result;
}

// **************************************************************************
// Function:   operator[]
// Purpose:    Access a state by its name.
// Parameters: State name.
// Returns:    Returns a reference to a state with a given name.
// **************************************************************************
State&
StateList::operator[]( const std::string& inName )
{
  StateIndex::const_iterator i = mIndex.find( inName );
  if( i == mIndex.end() )
  {
    mIndex[ inName ] = size();
    resize( size() + 1 );
    i = mIndex.find( inName );
  }
  return StateContainer::operator[]( i->second );
}

const State&
StateList::operator[]( const std::string& inName ) const
{
  static State defaultState;
  const State* result = &defaultState;
  StateIndex::const_iterator i = mIndex.find( inName );
  if( i != mIndex.end() )
    result = &StateContainer::operator[]( i->second );
  return *result;
}
// **************************************************************************
// Function:   Clear
// Purpose:    Clear all entries from the state list.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
StateList::Clear()
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
StateList::Add( const string& inDefinition )
{
  istringstream line( inDefinition );
  State s;
  if( line >> s )
  {
    if( Exists( s.Name() ) )
      ( *this )[ s.Name() ].SetValue( s.Value() );
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
StateList::Delete( const string& inName )
{
  if( mIndex.find( inName ) != mIndex.end() )
  {
    erase( begin() + mIndex[ inName ] );
    RebuildIndex();
  }
}

// **************************************************************************
// Function:   AssignPositions
// Purpose:    assigns positions and lengths to states contained in the list
// Parameters: none
// Returns:    N/A
// **************************************************************************
void
StateList::AssignPositions()
{
  int bitLength = 0;
  for( int i = 0; i < Size(); ++i )
  {
    ( *this )[ i ].SetLocation( bitLength );
    bitLength += ( *this )[ i ].Length();
  }
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of the entire
//             state list.
//             For partial output, use another instance of type StateList
//             to hold the desired subset.
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
StateList::WriteToStream( ostream& os ) const
{
  for( int i = 0; i < Size(); ++i )
    os << ( *this )[ i ] << '\n';
  return os;
}

// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of the entire
//             state list. The list is cleared before reading.
//             For partial input, use another instance of type StateList
//             to hold the desired subset.
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
StateList::ReadFromStream( istream& is )
{
  Clear();
  State state;
  is >> ws;
  while( !is.eof() && is >> state >> ws )
    Add( state );
  return is;
}

// **************************************************************************
// Function:   WriteBinary
// Purpose:    Member function for binary stream output of the entire
//             state list.
//             For partial output, use another instance of type StateList
//             to hold the desired subset.
// Parameters: Output stream to write into.
// Returns:    Output stream written into.
// **************************************************************************
ostream&
StateList::WriteBinary( ostream& os ) const
{
  for( int i = 0; i < Size(); ++i )
    ( *this )[ i ].WriteBinary( os );
  return os;
}

// **************************************************************************
// Function:   ReadBinary
// Purpose:    Member function for binary stream input of the entire
//             state list. The list is cleared before reading.
//             For partial input, use another instance of type StateList
//             to hold the desired subset.
// Parameters: Input stream to read from.
// Returns:    Input stream read from.
// **************************************************************************
istream&
StateList::ReadBinary( istream& is )
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
StateList::RebuildIndex()
{
  mIndex.clear();
  for( size_t i = 0; i < size(); ++i )
    mIndex[ operator[]( i ).mName ] = i;
}
