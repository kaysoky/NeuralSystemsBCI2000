////////////////////////////////////////////////////////////////////////////////
//
// File: UOperatorUtils.cpp
//
// Date: June 27, 2002
//
// Description: A file intended to hold global utility functions common to
//              different operator sources.
//
// Changes: June 27, 2002, juergen.mellinger@uni-tuebingen.de:
//          Created file.
//          Moved UpdateState() from ../shared/UState.h to here.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef UOperatorUtilsH
#define UOperatorUtilsH

class Scktcomp::TCustomWinSocket;
class STATELIST;

namespace OperatorUtils
{
// **************************************************************************
// Function:   UpdateState
// Purpose:    Send an updated state to a destination module
// Parameters: statelist - the statelist containing the state to modify
//             statename - name of the state to modify
//             newvalue - new value for this state
//             socket - socket used for communication
// Returns:    ERR_NOERR - if everything OK
//             ERR_STATENOTFOUND - if the state was not found
//             ERR_SOURCENOTCONNECTED - if the socket is not connected;
// **************************************************************************
  int     UpdateState( const STATELIST*   statelist,
                       const char*        statename,
                       unsigned short     newvalue,
                       TCustomWinSocket*  socket );
};

#endif
