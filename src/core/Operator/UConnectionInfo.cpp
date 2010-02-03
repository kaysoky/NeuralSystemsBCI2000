/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "UConnectionInfo.h"

#include "USysStatus.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfConnectionInfo *fConnectionInfo;
//---------------------------------------------------------------------------
__fastcall TfConnectionInfo::TfConnectionInfo(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void
TfConnectionInfo::UpdateDisplay( const SYSSTATUS& s )
{
  #define DISPLAY( name )                                  \
  t##name##1->Caption = AnsiString( s.name[ EEGSource ] ); \
  t##name##2->Caption = AnsiString( s.name[ SigProc ] );   \
  t##name##3->Caption = AnsiString( s.name[ App ] );
  DISPLAY( NumMessagesRecv );
  DISPLAY( NumParametersRecv );
  DISPLAY( NumStatesRecv );
  DISPLAY( NumDataRecv );
  DISPLAY( NumMessagesSent );
  DISPLAY( NumParametersSent );
  DISPLAY( NumStatesSent );
  DISPLAY( NumStateVecsSent );

  #define CONNECTION( name )                                 \
  c##name##Connected->Checked = ( s.Address[ name ] != "" ); \
  t##name##Connected->Caption = s.Address[ name ];
  CONNECTION( EEGSource );
  CONNECTION( SigProc );
  CONNECTION( App );
}

