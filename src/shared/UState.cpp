/******************************************************************************
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
 ******************************************************************************/
//---------------------------------------------------------------------------
#include "PCHIncludes.h"
#pragma hdrstop

#include "UState.h"

#include <stdio.h>
#include <assert>
#include <sstream>
#include <iomanip>
using namespace std;
//---------------------------------------------------------------------------
#pragma package(smart_init)


// **************************************************************************
// Function:   STATELIST
// Purpose:    the constructor for the STATELIST object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
STATELIST::STATELIST()
{
#ifndef NO_VCL
 state_list=new TList;
#endif
}

// **************************************************************************
// Function:   ~STATELIST
// Purpose:    The destructor for the STATELIST object. It deletes both
//             all STATE objects in the list and also the list object itself
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
STATELIST::~STATELIST()
{
 ClearStateList();
#ifndef NO_VCL
 delete state_list;
#endif
}


// **************************************************************************
// Function:   GetNumStates
// Purpose:    Returns the number of states in the list
// Parameters: N/A
// Returns:    the number of states
// **************************************************************************
int STATELIST::GetNumStates() const
{
#ifdef NO_VCL
  return state_list.size();
#else
return(state_list->Count);
#endif
}


// **************************************************************************
// Function:   ClearStateList
// Purpose:    clears and frees all entries in the state list
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void STATELIST::ClearStateList()
{
#ifdef NO_VCL
  for( _state_list::const_iterator i = state_list.begin();
          i != state_list.end(); ++i )
    delete *i;
  state_list.clear();
#else
int     i;
STATE   *cur_state;

  // Clean up – must free memory for the items as well as the list
  for (i=0; i<state_list->Count; i++)
   {
   cur_state=(STATE *)state_list->Items[i];
   delete cur_state;
   }
  state_list->Clear();
#endif // NO_VCL
}


// **************************************************************************
// Function:   GetStatePtr
// Purpose:    given an index (0..GetListCount()-1), returns the pointer to a STATE object
// Parameters: idx - index of the state
// Returns:    pointer to a STATE object or
//             NULL, if the specified index is out of range
// **************************************************************************
STATE *STATELIST::GetStatePtr(int idx)  const
{
#ifdef NO_VCL
  return ( idx < GetNumStates() && idx >= 0 ) ? state_list[ idx ] : NULL;
#else
 if ((idx < state_list->Count) && (idx >= 0))
    return((STATE *)state_list->Items[idx]);
 else
    return(NULL);
#endif
}

#ifdef NO_VCL
// **************************************************************************
// Function:   GetStateIndex
// Purpose:    given a state's name, returns the index to an appropriate
//             element in the vector
// Parameters: name - name of the state
// Returns:    index of the first pointer to a state with the given name,
//             or state_list.size(), if no state with this name exists in the
//             vector
// **************************************************************************
STATELIST::_state_list::size_type STATELIST::GetStateIndex( const char *name ) const
{
  string s_name( name );
  _state_list::size_type i = 0;
  while( i < state_list.size() && stricmp( s_name.c_str(), state_list[ i ]->GetName() ) )
    ++i;
  return i;
}
#endif // NO_VCL

// **************************************************************************
// Function:   GetStatePtr
// Purpose:    given a state's name, returns the pointer to a STATE object
// Parameters: name - name of the state
// Returns:    pointer to a STATE object or
//             NULL, if no state with this name exists in the list
// **************************************************************************
STATE *STATELIST::GetStatePtr(const char *name) const
{
#ifdef NO_VCL
  return GetStatePtr( GetStateIndex( name ) );
#else
const char* statename;
int         i;

 for (i=0; i<state_list->Count; i++)
  {
  statename=GetStatePtr(i)->GetName();
  if (strcmpi(name, statename) == 0)
     return(GetStatePtr(i));
  }

 return(NULL);
#endif
}

// **************************************************************************
// Function:   AddState2List
// Purpose:    adds a new state to the list of states
//             if a state with the same name already exists,
//             it updates the currently stored state with the provided values
// Parameters: statestring - ASCII string, as defined in project description,
//                           defining this new state
// Returns:    N/A
// **************************************************************************
void STATELIST::AddState2List(const char *statestring)
{
STATE* new_state,
     * existing_state;

 new_state=new STATE();
 new_state->ParseState(statestring, strlen(statestring));
 existing_state=GetStatePtr(new_state->GetName());
 if (existing_state)            // state already exists -> only update the value
    existing_state->SetValue(new_state->GetValue());
 else
    AddState2List(new_state);   // if the state does not exist, add the new one to the list
 delete new_state;
}


// **************************************************************************
// Function:   DeleteState
// Purpose:    deletes a state of a given name in the list of states
//             it also frees the memory for this particular state
//             it does not do anything, if the state does not exist
// Parameters: name - name of the state
// Returns:    N/A
// **************************************************************************
void STATELIST::DeleteState(const char *name)
{
#ifdef NO_VCL
  size_t i = GetStateIndex( name );
  if( i < state_list.size() )
  {
    delete state_list[ i ];
    state_list.erase( state_list.begin() + i );
  }
#else // NO_VCL
int     i;
STATE   *cur_state;

 // Clean up – must free memory for the item as well as the list
 for (i=0; i<state_list->Count; i++)
  {
  cur_state=(STATE *)state_list->Items[i];
  if (strcmpi(name, cur_state->GetName()) == 0)
     {
     state_list->Delete(i);
     delete cur_state;
     }
  }
#endif // NO_VCL
}


// **************************************************************************
// Function:   AddState2List
// Purpose:    Adds a new state to the list of parameters
//             It physically copies the STATE object (the specified STATE
//             object can then be freed elsewhere)
//             if a state with the same name already exists,
//             it updates the currently stored state with the provided values
// Parameters: state - pointer to an existing STATE object
// Returns:    N/A
// **************************************************************************
void STATELIST::AddState2List(const STATE *state)
{
#ifdef NO_VCL
  DeleteState( state->GetName() );
  state_list.push_back( new STATE( *state ) );
#else // NO_VCL
STATE           *newstate;

 // if we can find a state with the same name, delete the old state and
 // thereafter add the new one
 if (GetStatePtr(state->GetName()))
    DeleteState(state->GetName());

 // create a new state and copy the content of the other one
  newstate = new STATE( *state );

 state_list->Add(newstate);
#endif // NO_VCL
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
// Returns:    N/A
// **************************************************************************
void
STATELIST::WriteToStream( ostream& os ) const
{
  for( int i = 0; i < GetNumStates(); ++i )
    os << *GetStatePtr( i ) << '\n';
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
// Returns:    N/A
// **************************************************************************
void
STATELIST::ReadFromStream( istream& is )
{
  ClearStateList();
  STATE state;
  is >> ws;
  while( !is.eof() && is >> state >> ws )
    AddState2List( &state );
}

// **************************************************************************
// Function:   STATE
// Purpose:    the constructor for the STATE object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
STATE::STATE()
: length( 0 ),
  value( 0 ),
  byteloc( 0 ),
  bitloc( 0 ),
  valid( false ),
  modified( false )
{
  name[ 0 ] = '\0';
}

// **************************************************************************
// Function:   GetStateLine
// Purpose:    Returns a state line in ASCII format
//             This state line is constructed, based upon the current
//             values in the STATE object
// Parameters: N/A
// Returns:    a pointer to the state line
// **************************************************************************
std::string
STATE::GetStateLine() const
{
 ostringstream oss;
 oss << *this;
 return oss.str();
}

// **************************************************************************
// Function:   ReadFromStream
// Purpose:    Member function for formatted stream input of a single
//             state.
//             All formatted input functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Input stream to read from.
// Returns:    N/A
// **************************************************************************
void
STATE::ReadFromStream( std::istream& is )
{
  *this = STATE();
  modified = true;
  is >> name >> length >> value >> byteloc >> bitloc;
  valid = !is.fail();
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted stream output of a single
//             state.
//             All formatted output functions are, for consistency's sake,
//             supposed to use this function.
// Parameters: Output stream to write into.
// Returns:    N/A
// **************************************************************************
void
STATE::WriteToStream( std::ostream& os ) const
{
  os << name << " " << length << " " << value << " " << byteloc << " " << bitloc;
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
  if( getline( is, line, '\x0a' ) )
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
  os << '\x0d' << '\x0a';
  return os;
}

// **************************************************************************
// Function:   GetName
// Purpose:    Returns the name of this state
// Parameters: N/A
// Returns:    a pointer to the state's name
// **************************************************************************
const char *STATE::GetName() const
{
 return(name);
}


// **************************************************************************
// Function:   GetLength
// Purpose:    Returns this state's length
// Parameters: N/A
// Returns:    this state's length
// **************************************************************************
int STATE::GetLength() const
{
 return(length);
}


// **************************************************************************
// Function:   GetValue
// Purpose:    Returns this state's value
// Parameters: N/A
// Returns:    this state's value
// **************************************************************************
unsigned short STATE::GetValue() const
{
 return(value);
}


// **************************************************************************
// Function:   SetValue
// Purpose:    Sets this state's value
// Parameters: value
// Returns:    N/A
// **************************************************************************
void STATE::SetValue(unsigned short new_value)
{
 value=new_value;
 modified = true;
}


// **************************************************************************
// Function:   GetByteLocation
// Purpose:    Returns this state's byte location in the state vector
// Parameters: N/A
// Returns:    this state's byte location
// **************************************************************************
int STATE::GetByteLocation() const
{
 return(byteloc);
}


// **************************************************************************
// Function:   GetBitLocation
// Purpose:    Returns this state's bit location in the state vector
// Parameters: N/A
// Returns:    this state's bit location
// **************************************************************************
int STATE::GetBitLocation() const
{
 return(bitloc);
}


// **************************************************************************
// Function:   SetByteLocation
// Purpose:    Sets this state's byte location in the state vector
// Parameters: this state's byte location
// Returns:    N/A
// **************************************************************************
void STATE::SetByteLocation(int loc)
{
 byteloc=loc;
}


// **************************************************************************
// Function:   SetBitocation
// Purpose:    Sets this state's bit location in the state vector
// Parameters: this state's bit location
// Returns:    N/A
// **************************************************************************
void STATE::SetBitLocation(int loc)
{
 bitloc=loc;
}

// **************************************************************************
// Function:   ParseState
// Purpose:    This routine is called by coremessage->ParseMessage()
//             it parses the provided ASCII state line and initializes
//             the STATE object accordingly, i.e., it fills in value, name,
//             byte location, bit location, etc.
// Parameters: line - pointer to the ASCII state line
//             length - length of this state line
// Returns:    ERRSTATE_INVALIDSTATE if the state line is invalid, or
//             ERRSTATE_NOERR
// **************************************************************************
int STATE::ParseState( const char* stateline, int length )
{
  if( stateline == NULL )
    return ERRSTATE_INVALIDSTATE;
  int err = ERRSTATE_NOERR;
  string line( stateline, length );
  if( length == 0 )
    line = string( stateline );
  istringstream linestream( line );
  linestream >> *this;
  if( !linestream )
    err = ERRSTATE_INVALIDSTATE;
  return err;
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
  if( modified )
  {
    assert( stateVector != NULL );
    int err = stateVector->SetStateValue( byteloc, bitloc, length, value );
    assert( err == ERRSTATEVEC_NOERR );
    modified = false;
  }
}

// **************************************************************************
// Function:   Initialize_StateVector
// Purpose:    assigns locations in the state vector for each state; in addition, it
//             initializes the state vector with the values of each state in the list
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void STATEVECTOR::Initialize_StateVector()
{
 // initialize the statevector by calculating the positions in the statevector
 Initialize_StateVector(false);
}

// **************************************************************************
// Function:   Initialize_StateVector
// Purpose:    It initializes the state vector with the values of each state in the list
// Parameters: use_assigned_positions = true  ... use the Byte/Bit of the states for the statevector
//                                      false ... calculate the byte and bit position of each state in the statevector
// Returns:    N/A
// **************************************************************************
void STATEVECTOR::Initialize_StateVector(bool use_assigned_positions)
{
int     i, cur_vector_length;
STATE   *cur_state;

 cur_vector_length=0;

 // at the very beginning, initialize the state vector to all 0 bytes
 for (i=0; i<MAX_STATEVECTORLENGTH; i++)
  state_vector[i]=0;

 // calculate the state vector length
 for (i=0; i<state_list->GetNumStates(); i++)
  {
  cur_state=state_list->GetStatePtr(i);
  if (!use_assigned_positions)
     {
     cur_state->SetByteLocation(cur_vector_length>>3);
     cur_state->SetBitLocation(cur_vector_length%8);
     }
  cur_vector_length += cur_state->GetLength();
  }

 state_vector_length=(cur_vector_length>>3)+1;        // divide it by eight, add one to get the number of needed bytes
 if (state_vector_length >= MAX_STATEVECTORLENGTH)
    {
    state_vector_length=0;
    return;
    }

 // initialize the content in the state vector, according to the content
 // of the current states in the state list
 for (i=0; i<state_list->GetNumStates(); i++)
  {
  SetStateValue(state_list->GetStatePtr(i)->GetName(), state_list->GetStatePtr(i)->GetValue());
  // if (state_list->GetStatePtr(i)->GetValue() != GetStateValue(state_list->GetStatePtr(i)->GetName()))
  //   Application->MessageBox("Inconsistent values before and after", "INTERNAL ERROR", MB_OK);
  }
}

STATEVECTOR::STATEVECTOR(STATELIST *list)
{
 state_list=list;       // so the state vector knows which values it holds
 Initialize_StateVector();
}

// can specify, whether or not defined byte/bit positions should be used
// true: use already specified ones
STATEVECTOR::STATEVECTOR(STATELIST *list, bool usepositions)
{
 state_list=list;       // so the state vector knows which values it holds
 Initialize_StateVector(usepositions);
}

STATEVECTOR::~STATEVECTOR()
{
}

// **************************************************************************
// Function:   GetStateValue
// Purpose:    returns a state's value, based upon the state's location and size
// Parameters: byteloc ... location of the byte of the state
//             bitloc  ... location of the bit of the state
//             length  ... length of the state
// Returns:    ERRSTATEVEC_NOSTATE on error (e.g., location outside range)
//             the value of the state otherwise
// **************************************************************************
unsigned short STATEVECTOR::GetStateValue(int byteloc, int bitloc, int length) const
{
int     i, dest_byte, dest_bit;
char    buf[2];
unsigned short *result;

 // location is outside the state vector or length > 16 bit
 if ((byteloc+length/8 >= MAX_STATEVECTORLENGTH) || (length > 16))
    return(ERRSTATEVEC_NOSTATE);

 result=0;
 dest_byte=0;
 dest_bit=0;
 buf[0]=buf[1]=0;
 result=(unsigned short *)buf;
 for (i=0; i<length; i++)
  {
  if (state_vector[byteloc]&(1<<bitloc))
     buf[dest_byte] |= (1<<dest_bit);
  dest_bit++;
  if (dest_bit > 7)
     {
     dest_bit=0;
     dest_byte++;
     }
  bitloc++;
  if (bitloc > 7)
     {
     bitloc=0;
     byteloc++;
     }
  }

 return(*result);
}


// **************************************************************************
// Function:   GetStateValue
// Purpose:    returns a state's value from the state vector
// Parameters: statename - the name of a state
// Returns:    the value of the state
//             0 on error (e.g., state not found)
// **************************************************************************
unsigned short STATEVECTOR::GetStateValue(const char *statename) const
{
int     i, value;
STATE   *cur_state;

 value=0;
 cur_state=state_list->GetStatePtr(statename);
 if (cur_state)
    value=GetStateValue(cur_state->GetByteLocation(), cur_state->GetBitLocation(), cur_state->GetLength());

 return(value);
}


// **************************************************************************
// Function:   SetStateValue
// Purpose:    sets a state's value in the state vector
// Parameters: byteloc ... location of the byte of the state
//             bitloc  ... location of the bit of the state
//             length  ... length of the state
//             value   ... value of the state
// Returns:    ERRSTATEVEC_NOSTATE on error (e.g., location out of range)
//             ERRSTATEVEC_NOERR otherwise
// **************************************************************************
int STATEVECTOR::SetStateValue(int byteloc, int bitloc, int length, unsigned short value)
{
int     i, src_byte, src_bit;
char    *valueptr;

 // location is outside the state vector or length > 16 bit
 if ((byteloc+length/8 >= MAX_STATEVECTORLENGTH) || (length > 16))
    return(ERRSTATEVEC_NOSTATE);

 src_byte=0;
 src_bit=0;
 valueptr=(char *)&value;
 for (i=0; i<length; i++)
  {
  if (valueptr[src_byte]&(1<<src_bit))
     state_vector[byteloc] |= (1<<bitloc);
  else
     state_vector[byteloc] &= ~(1<<bitloc);
  src_bit++;
  if (src_bit > 7)
     {
     src_bit=0;
     src_byte=1;
     }
  bitloc++;
  if (bitloc > 7)
     {
     bitloc=0;
     byteloc++;
     }
  }

 return(ERRSTATEVEC_NOERR);
}


// **************************************************************************
// Function:   SetStateValue
// Purpose:    sets a state's value in the state vector
// Parameters: statename - name of the state
// Returns:    ERRSTATEVEC_NOSTATE on error (e.g., did not recognize state name)
//             ERRSTATEVEC_NOERR otherwise
// **************************************************************************
int STATEVECTOR::SetStateValue(const char *statename, unsigned short stateval)
{
int     i, value;
STATE   *cur_state;

 value=ERRSTATEVEC_NOSTATE;
 for (i=0; i<state_list->GetNumStates(); i++)
  {
  cur_state=state_list->GetStatePtr(i);
  if (strcmp(statename, cur_state->GetName()) == 0)
     {
     value=SetStateValue(cur_state->GetByteLocation(), cur_state->GetBitLocation(), cur_state->GetLength(), stateval);
     break;
     }
  }

 return(value);
}


// **************************************************************************
// Function:   GetStateVectorLength
// Purpose:    returns the length of the state vector in bytes
// Parameters: N/A
// Returns:    the length of the state vector in bytes
// **************************************************************************
int STATEVECTOR::GetStateVectorLength() const
{
return(state_vector_length);
}


// **************************************************************************
// Function:   GetStateVectorPtr
// Purpose:    returns a BYTE ptr to the state vector data
// Parameters: N/A
// Returns:    a pointer to the state vector data
// **************************************************************************
BYTE*
STATEVECTOR::GetStateVectorPtr()
{
return(state_vector);
}

const BYTE*
STATEVECTOR::GetStateVectorPtr() const
{
return(state_vector);
}

// **************************************************************************
// Function:   GetStateList
// Purpose:    returns a ptr to the list of states describing the state vector
// Parameters: N/A
// Returns:    pointer to the list of states
// **************************************************************************
STATELIST *STATEVECTOR::GetStateListPtr()
{
return(state_list);
}

// **************************************************************************
// Function:   CommitStateChanges
// Purpose:    have all states commit their changes, if any
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
STATEVECTOR::CommitStateChanges()
{
  if( !state_list )
    return;

  for( int i = 0; i < state_list->GetNumStates(); ++i )
  {
    STATE* state = state_list->GetStatePtr( i );
    assert( state != NULL );
    state->Commit( this );
  }
}

// **************************************************************************
// Function:   WriteToStream
// Purpose:    Member function for formatted output of a state vector
//             into a stream.
// Parameters: Output stream to write into.
// Returns:    Output stream.
// **************************************************************************
void
STATEVECTOR::WriteToStream( std::ostream& os ) const
{
  int indent = os.width();
  const STATELIST* pStatelist = state_list;
  if( pStatelist == NULL )
    for( int i = 0; i < GetStateVectorLength(); ++i )
      os << '\n' << std::setw( indent ) << ""
         << i << ": "
         << GetStateVectorPtr()[ i ];
  else
    for( int i = 0; i < pStatelist->GetNumStates(); ++i )
    {
      const STATE* pState = pStatelist->GetStatePtr( i );
      os << '\n' << std::setw( indent ) << ""
         << pState->GetName() << ": "
         << GetStateValue( pState->GetByteLocation(), pState->GetBitLocation(), pState->GetLength() );
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
STATEVECTOR::ReadBinary( istream& is )
{
  // Reading the last byte with is.get() avoids possible problems with tcp
  // stream buffers. See the comments in TCPStream.cpp, tcpbuf::underflow()
  // for details.
  int   length = GetStateVectorLength();
  char* stateVectorData = GetStateVectorPtr();
  if( length > 1 )
    is.read( stateVectorData, length - 1 );
  stateVectorData[ length - 1 ] = is.get();
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
  os.write( ( const char* )GetStateVectorPtr(), GetStateVectorLength() );
  return os;
}



