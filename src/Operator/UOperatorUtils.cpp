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
//          - Created file.
//          - Moved UpdateState() from ../shared/UState.h to here.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include <ScktComp.hpp>
#include <stdio.h>

#include "UOperatorUtils.h"
#include "UState.h"
#include "UCoreMessage.h"
#include "UBCIError.h"

int
OperatorUtils::UpdateState( const STATELIST*   statelist,
                            const char*        statename,
                            unsigned short     newvalue,
                            TCustomWinSocket*  socket )
{
  COREMESSAGE     *coremessage;
  char            statelinebuf[LENGTH_STATELINE];
  int             ret;
  STATE           *cur_state;

   // is the destination connected ?
   try
   {
     if ((!socket) || (!socket->Connected))
       return(ERR_SOURCENOTCONNECTED);
   }
   catch( TooGeneralCatch& )
   {
     return(ERR_SOURCENOTCONNECTED);
   }

   coremessage=new COREMESSAGE;
   coremessage->SetDescriptor(COREMSG_STATE);

   ret=ERR_NOERR;

   cur_state = NULL;
   if( statelist != NULL )
     cur_state=statelist->GetStatePtr(statename);
     
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
   return ret;
}
