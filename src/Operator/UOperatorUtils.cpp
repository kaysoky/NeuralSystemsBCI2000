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

#include "UOperatorUtils.h"
#include "UState.h"
#include "UCoreMessage.h"
#include "UBCIError.h"
#include "defines.h"

#include <ScktComp.hpp>
#include <Registry.hpp>
#include <stdio.h>
#include <string>
#include <typeinfo>

using namespace std;

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

void
OperatorUtils::SaveControl( const TControl* c )
{
  TRegistry* reg = NULL;
  try
  {
    reg = new TRegistry( KEY_WRITE );
    reg->RootKey = HKEY_CURRENT_USER;
    reg->OpenKey( ( string( KEY_BCI2000 KEY_OPERATOR KEY_CONFIG "\\" ) + typeid( *c ).name() ).c_str(), true );
    reg->WriteInteger( "Left", c->Left );
    reg->WriteInteger( "Top", c->Top );
    reg->WriteInteger( "Height", c->Height );
    reg->WriteInteger( "Width", c->Width );
  }
  catch( ERegistryException& )
  {
  }
  delete reg;
}

void
OperatorUtils::RestoreControl( TControl* c )
{
  TRegistry* reg = NULL;
  try
  {
    reg = new TRegistry( KEY_READ );
    reg->RootKey = HKEY_CURRENT_USER;
    reg->OpenKey( ( string( KEY_BCI2000 KEY_OPERATOR KEY_CONFIG "\\" ) + typeid( *c ).name() ).c_str(), false );
    TRect storedRect;
    storedRect.Left = reg->ReadInteger( "Left" );
    storedRect.Top = reg->ReadInteger( "Top" );
    storedRect.Right = storedRect.Left + reg->ReadInteger( "Width" );
    storedRect.Bottom = storedRect.Top + reg->ReadInteger( "Height" );
    const int minDistance = 10; // Make sure at least that much of the window is
                                // inside the screen.
    TRect commonRect;
    if( Types::IntersectRect( commonRect, storedRect, Screen->WorkAreaRect )
        && commonRect.Height() >= minDistance
        && commonRect.Width() >= minDistance )
    {
      c->Left = storedRect.Left;
      c->Top = storedRect.Top;
      c->Height = storedRect.Height();
      c->Width = storedRect.Width();
    }
  }
  catch( ERegistryException& )
  {
  }
  delete reg;
}


