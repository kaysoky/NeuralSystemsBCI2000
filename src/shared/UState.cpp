/******************************************************************************
 * Program:   BCI2000                                                         *
 * Module:    UState.cpp                                                      *
 * Comment:   This unit provides support for system-wide states,              *
 *            lists of states, and the state vector                           *
 * Version:   0.08                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.08 - 03/30/2000 - First commented version                               *
 ******************************************************************************/
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "defines.h"

#include "UState.h"
#include "UCoremessage.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)


// **************************************************************************
// Function:   UpdateState
// Purpose:    Send an updated state to a destination module
// Parameters: statename - name of the state to modify
//             newvalue - new value for this state
// Returns:    ERR_NOERR - if everything OK
//             ERR_STATENOTFOUND - if the state was not found
//             ERR_SOURCENOTCONNECTED - if the socket is not connected;
// **************************************************************************
int STATELIST::UpdateState(char *statename, unsigned short newvalue, TCustomWinSocket *socket)
{
COREMESSAGE     *coremessage;
char            statelinebuf[LENGTH_STATELINE];
int             ret;
STATE           *cur_state;

 // is the destination connected ?
 try {
 if ((!socket) || (!socket->Connected))
    return(ERR_SOURCENOTCONNECTED);
 } catch(...) { return(ERR_SOURCENOTCONNECTED); }

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_STATE);

 ret=ERR_NOERR;

 cur_state=GetStatePtr(statename);
 if (cur_state)
    {
    sprintf(statelinebuf, "%s %d %d %d %d\r\n", statename, cur_state->GetLength(), newvalue, cur_state->GetByteLocation(), cur_state->GetBitLocation());
    coremessage->SetLength((unsigned short)strlen(statelinebuf));
    strncpy(coremessage->GetBufPtr(), statelinebuf, strlen(statelinebuf));
    coremessage->SendCoreMessage(socket);
    }
 else
    ret=ERR_STATENOTFOUND;

 delete coremessage;
 return(ret);
}


// **************************************************************************
// Function:   PublishStates
// Purpose:    this method publishes this module's states
// Parameters: statelist - filled list of states
//             Socket - socket descriptor for the connection to the operator
// Returns:    always 0
// **************************************************************************
int STATELIST::PublishStates(TCustomWinSocket *Socket)
{
COREMESSAGE     *coremessage;
char            line[512];
int             i;
STATE           *cur_state;

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_STATE);

 // now, let's publish the list of states
 // that the ADC board has requested so far ...
 for (i=0; i<GetNumStates(); i++)
  {
  cur_state=GetStatePtr(i);                                     // get the i'th state
  strcpy(line, cur_state->GetStateLine());                      // copy its ASCII representation to variable line
  strncpy(coremessage->GetBufPtr(), line, strlen(line));        // copy line into the coremessage
  coremessage->SetLength((unsigned short)strlen(line));         // set the length of the coremessage
  coremessage->SendCoreMessage((TCustomWinSocket *)Socket);     // and send it out
  }

 // the system command "EndOfState" terminates the list of states
 coremessage->SetDescriptor(COREMSG_SYSCMD);
 strcpy(line, "EndOfState");
 coremessage->SetLength((unsigned short)strlen(line));
 strncpy(coremessage->GetBufPtr(), line, strlen(line));
 coremessage->SendCoreMessage((TCustomWinSocket *)Socket);

 delete coremessage;
 return(0);
}


// **************************************************************************
// Function:   STATELIST
// Purpose:    the constructor for the STATELIST object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
STATELIST::STATELIST()
{
 state_list=new TList;
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
 delete state_list;
}


// **************************************************************************
// Function:   GetNumStates
// Purpose:    Returns the number of states in the list
// Parameters: N/A
// Returns:    the number of states
// **************************************************************************
int STATELIST::GetNumStates()
{
return(state_list->Count);
}


// **************************************************************************
// Function:   ClearStateList
// Purpose:    clears and frees all entries in the state list
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void STATELIST::ClearStateList()
{
int     i;
STATE   *cur_state;

  // Clean up – must free memory for the items as well as the list
  for (i=0; i<state_list->Count; i++)
   {
   cur_state=(STATE *)state_list->Items[i];
   delete cur_state;
   }
  state_list->Clear();
}


// **************************************************************************
// Function:   GetStatePtr
// Purpose:    given an index (0..GetListCount()-1), returns the pointer to a STATE object
// Parameters: idx - index of the state
// Returns:    pointer to a STATE object or
//             NULL, if the specified index is out of range
// **************************************************************************
STATE *STATELIST::GetStatePtr(int idx)
{
 if ((idx < state_list->Count) && (idx >= 0))
    return((STATE *)state_list->Items[idx]);
 else
    return(NULL);
}


// **************************************************************************
// Function:   GetStatePtr
// Purpose:    given a state's name, returns the pointer to a STATE object
// Parameters: name - name of the state
// Returns:    pointer to a STATE object or
//             NULL, if no state with this name exists in the list
// **************************************************************************
STATE *STATELIST::GetStatePtr(char *name)
{
char    *statename;
int     i;

 for (i=0; i<state_list->Count; i++)
  {
  statename=GetStatePtr(i)->GetName();
  if (strcmpi(name, statename) == 0)
     return(GetStatePtr(i));
  }

 return(NULL);
}


// **************************************************************************
// Function:   GetListPtr
// Purpose:    Returns a pointer to this state's list of values
// Parameters: N/A
// Returns:    pointer to the list
// **************************************************************************
TList *STATELIST::GetListPtr()
{
return(state_list);
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
void STATELIST::AddState2List(char *statestring)
{
STATE   *new_state, *existing_state;

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
// Function:   DeleteParam
// Purpose:    deletes a state of a given name in the list of states
//             it also frees the memory for this particular state
//             it does not do anything, if the state does not exist
// Parameters: name - name of the state
// Returns:    N/A
// **************************************************************************
void STATELIST::DeleteState(char *name)
{
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
void STATELIST::AddState2List(STATE *state)
{
STATE           *newstate;

 // if we can find a state with the same name, delete the old state and
 // thereafter add the new one
 if (GetStatePtr(state->GetName()))
    DeleteState(state->GetName());

 // create a new state and copy the content of the other one
 newstate=new STATE;
 memcpy(newstate, state, sizeof(STATE));

 GetListPtr()->Add(newstate);
}


// **************************************************************************
// Function:   STATE
// Purpose:    the constructor for the STATE object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
STATE::STATE()
{
}


// **************************************************************************
// Function:   ~STATE
// Purpose:    the destructor for the STATE object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
STATE::~STATE()
{
}


// **************************************************************************
// Function:   GetStateLine
// Purpose:    Returns a state line in ASCII format
//             This state line is constructed, based upon the current
//             values in the STATE object
// Parameters: N/A
// Returns:    a pointer to the state line
// **************************************************************************
char *STATE::GetStateLine()
{
 ConstructStateLine();
 return(buffer);
}


// **************************************************************************
// Function:   GetName
// Purpose:    Returns the name of this state
// Parameters: N/A
// Returns:    a pointer to the state's name
// **************************************************************************
char *STATE::GetName()
{
 return(name);
}


// **************************************************************************
// Function:   GetLength
// Purpose:    Returns this state's length
// Parameters: N/A
// Returns:    this state's length
// **************************************************************************
int STATE::GetLength()
{
 return(length);
}


// **************************************************************************
// Function:   GetValue
// Purpose:    Returns this state's value
// Parameters: N/A
// Returns:    this state's value
// **************************************************************************
unsigned short STATE::GetValue()
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
}


// **************************************************************************
// Function:   GetByteLocation
// Purpose:    Returns this state's byte location in the state vector
// Parameters: N/A
// Returns:    this state's byte location
// **************************************************************************
int STATE::GetByteLocation()
{
 return(byteloc);
}


// **************************************************************************
// Function:   GetBitLocation
// Purpose:    Returns this state's bit location in the state vector
// Parameters: N/A
// Returns:    this state's bit location
// **************************************************************************
int STATE::GetBitLocation()
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
// Function:   get_argument
// Purpose:    parses the state line that is being sent in the core
//             communication
//             it returns the next token that is being delimited by either
//             a ' ' or '='
// Parameters: ptr - index into the line of where to start
//             buf - destination buffer for the token
//             line - the whole line
//             maxlen - maximum length of the line
// Returns:    the index into the line where the returned token ends
// **************************************************************************
int STATE::get_argument(int ptr, char *buf, char *line, int maxlen)
{
 // skip trailing spaces, if any
 while ((line[ptr] == '=') || (line[ptr] == ' ') && (ptr < maxlen))
  ptr++;

 // go through the string, until we either hit a space, a '=', or are at the end
 while ((line[ptr] != '=') && (line[ptr] != ' ') && (line[ptr] != '\n') && (line[ptr] != '\r') && (ptr < maxlen))
  {
  *buf=line[ptr];
  ptr++;
  buf++;
  }

 *buf=0;
 return(ptr);
}


// **************************************************************************
// Function:   ConstructStateLine
// Purpose:    Construct a state line, based upon the current values
//             in the STATE object
// Parameters: N/A
// Returns:    always ERRPARAM_NOERR 
// **************************************************************************
int STATE::ConstructStateLine()
{
 sprintf(buffer, "%s %d %d %d %d", name, length, value, byteloc, bitloc);
 return(ERRSTATE_NOERR);
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
int STATE::ParseState(char *line, int statelength)
{
int     ptr;
char    *buf;

 valid=false;
 buf=(char *)malloc(statelength+1);
 // buf=new char[statelength+1];

 ptr=0;
 ptr=get_argument(ptr, buf, line, statelength);
 strncpy(name, buf, LENGTH_NAME);
 if (name[0] == 0)
    {
    free(buf);
    return(ERRSTATE_INVALIDSTATE);
    }
 ptr=get_argument(ptr, buf, line, statelength);
 length=atoi(buf);
 ptr=get_argument(ptr, buf, line, statelength);
 value=atoi(buf);
 ptr=get_argument(ptr, buf, line, statelength);
 byteloc=atoi(buf);
 ptr=get_argument(ptr, buf, line, statelength);
 bitloc=atoi(buf);

 free(buf);
 // delete buf;
 valid=true;
 return(ERRSTATE_NOERR);
}


/*
// **************************************************************************
// Function:   Set_StateVector
// Purpose:    sets the content of a whole state vector
// Parameters: src - pointer to the source state vector content
//             length - length of the state vector in bytes
//             src_list - pointer to the list of states describing this state vector
// Returns:    N/A
// **************************************************************************
void STATEVECTOR::SetStateVector(BYTE *src, int length, STATELIST *src_list)
{
 if (length < MAX_STATEVECTORLENGTH)
    {
    memcpy(&state_vector[0], src, length);
    state_vector_length=length;
    state_list=src_list;
    }
 else
    {
    state_vector_length=0;
    state_list=NULL;
    }
}
*/

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
unsigned short STATEVECTOR::GetStateValue(int byteloc, int bitloc, int length)
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
unsigned short STATEVECTOR::GetStateValue(char *statename)
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
int STATEVECTOR::SetStateValue(char *statename, unsigned short stateval)
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
int STATEVECTOR::GetStateVectorLength()
{
return(state_vector_length);
}


// **************************************************************************
// Function:   GetStateVectorPtr
// Purpose:    returns a BYTE ptr to the state vector data
// Parameters: N/A
// Returns:    a pointer to the state vector data
// **************************************************************************
BYTE *STATEVECTOR::GetStateVectorPtr()
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


