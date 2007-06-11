/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include <Registry.hpp>

#include "operator.h"                    // operator specific defines

#include "USysStatus.h"
#include "UPreferences.h"
#include "UBCIError.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TfPreferences *fPreferences;

//---------------------------------------------------------------------------
__fastcall TfPreferences::TfPreferences(TComponent* Owner)
        : TForm(Owner)
{
}

//---------------------------------------------------------------------------
void __fastcall TfPreferences::TrackBar1Change(TObject *Sender)
{
 if (TrackBar1->Position == 1)
    {
    tUserLevel->Caption=USERLEVELTXT_BEGINNER;
    preferences->UserLevel=USERLEVEL_BEGINNER;
    }
 if (TrackBar1->Position == 2)
    {
    tUserLevel->Caption=USERLEVELTXT_INTERMEDIATE;
    preferences->UserLevel=USERLEVEL_INTERMEDIATE;
    }
 if (TrackBar1->Position == 3)
    {
    tUserLevel->Caption=USERLEVELTXT_ADVANCED;
    preferences->UserLevel=USERLEVEL_ADVANCED;
    }
}

//---------------------------------------------------------------------------
void __fastcall TfPreferences::bCloseClick(TObject *Sender)
{
  #define SAVE_SCRIPT( name ) \
  if( !preferences->mCmdlineSpecified[ PREFERENCES::name ] ) \
    preferences->Script[ PREFERENCES::name ] = e##name->Text.Trim();
  SAVE_SCRIPT( AfterModulesConnected );
  SAVE_SCRIPT( OnExit );
  SAVE_SCRIPT( OnSetConfig );
  SAVE_SCRIPT( OnResume );
  SAVE_SCRIPT( OnSuspend );
  SAVE_SCRIPT( OnStart );

  #define SAVE_BUTTON( number ) \
  preferences->Buttons[ number ].Name = eButton##number##Name->Text.Trim(); \
  preferences->Buttons[ number ].Cmd = eButton##number##Cmd->Text.Trim();
  SAVE_BUTTON( 1 );
  SAVE_BUTTON( 2 );
  SAVE_BUTTON( 3 );
  SAVE_BUTTON( 4 );

  Close();
}
//---------------------------------------------------------------------------


void __fastcall TfPreferences::FormShow(TObject *Sender)
{
 if (preferences->UserLevel == USERLEVEL_BEGINNER)
    {
    TrackBar1->Position=1;
    tUserLevel->Caption=USERLEVELTXT_BEGINNER;
    }
 if (preferences->UserLevel == USERLEVEL_INTERMEDIATE)
    {
    TrackBar1->Position=2;
    tUserLevel->Caption=USERLEVELTXT_INTERMEDIATE;
    }
 if (preferences->UserLevel == USERLEVEL_ADVANCED)
    {
    TrackBar1->Position=3;
    tUserLevel->Caption=USERLEVELTXT_ADVANCED;
    }

  #define RESTORE_SCRIPT( name ) \
  e##name->Text = preferences->Script[ PREFERENCES::name ]; \
  if( preferences->mCmdlineSpecified[ PREFERENCES::name ] ) \
    e##name->ReadOnly = true;
  RESTORE_SCRIPT( AfterModulesConnected );
  RESTORE_SCRIPT( OnExit );
  RESTORE_SCRIPT( OnSetConfig );
  RESTORE_SCRIPT( OnResume );
  RESTORE_SCRIPT( OnSuspend );
  RESTORE_SCRIPT( OnStart );

  #define RESTORE_BUTTON( number ) \
  eButton##number##Name->Text = preferences->Buttons[ number ].Name; \
  eButton##number##Cmd->Text = preferences->Buttons[ number ].Cmd;
  RESTORE_BUTTON( 1 );
  RESTORE_BUTTON( 2 );
  RESTORE_BUTTON( 3 );
  RESTORE_BUTTON( 4 );
}
//---------------------------------------------------------------------------



PREFERENCES::PREFERENCES()
{
  UserLevel=USERLEVEL_BEGINNER;
  for( int i = 0; i < PREFERENCES::numScriptEvents; ++i )
    mCmdlineSpecified[ i ] = false;

  const struct
  {
    const char* name;
    int         event;
  } paramNames[] =
  {
    { "--OnConnect",   AfterModulesConnected },
    { "--OnExit",      OnExit },
    { "--OnSetConfig", OnSetConfig },
    { "--OnSuspend",   OnSuspend },
    { "--OnResume",    OnResume },
    { "--OnStart",     OnStart }
  };
  int i = 1;
  while( i + 1 < __argc )
  {
    for( int j = 0; j < sizeof( paramNames ) / sizeof( *paramNames ); ++j )
    {
      if( std::string( __argv[ i ] ).find( paramNames[ j ].name ) == 0 )
      {
        mCmdlineSpecified[ paramNames[ j ].event ] = true;
        Script[ paramNames[ j ].event ] = __argv[ ++i ];
      }
    }
    ++i;
  }
}


// retrieves the default user settings
void PREFERENCES::GetDefaultSettings()
{
  TIniFile* storage = new TIniFile( ExtractFilePath( Application->ExeName ) + INIFILENAME_PREFERENCES );
  try
  {
    UserLevel = storage->ReadInteger( "Various", "DefaultUserLevel", USERLEVEL_ADVANCED );
    #define READ_SCRIPT( name ) \
    if( !mCmdlineSpecified[ name ] ) \
      Script[ name ] = storage->ReadString( "Scripts", #name, "" );
    READ_SCRIPT( AfterModulesConnected );
    READ_SCRIPT( OnExit );
    READ_SCRIPT( OnSetConfig );
    READ_SCRIPT( OnResume );
    READ_SCRIPT( OnSuspend );
    READ_SCRIPT( OnStart );

    #define READ_BUTTON( number ) \
    Buttons[ number ].Name = storage->ReadString( "Buttons", "Button" #number "Name", "" );\
    Buttons[ number ].Cmd = storage->ReadString( "Buttons", "Button" #number "Cmd", "" );
    READ_BUTTON( 1 );
    READ_BUTTON( 2 );
    READ_BUTTON( 3 );
    READ_BUTTON( 4 );
  }
  catch(...) {}

  delete storage;
}

// sets the default user settings
void PREFERENCES::SetDefaultSettings()
{
  TIniFile* storage = new TIniFile( ExtractFilePath( Application->ExeName ) + INIFILENAME_PREFERENCES );
  try
  {
    storage->WriteInteger( "Various", "DefaultUserLevel", UserLevel );
    #define WRITE_SCRIPT( name ) \
    if( !mCmdlineSpecified[ name ] ) \
      storage->WriteString( "Scripts", #name, Script[ name ] );
    WRITE_SCRIPT( AfterModulesConnected );
    WRITE_SCRIPT( OnExit );
    WRITE_SCRIPT( OnSetConfig );
    WRITE_SCRIPT( OnResume );
    WRITE_SCRIPT( OnSuspend );
    WRITE_SCRIPT( OnStart );

    #define WRITE_BUTTON( number ) \
    storage->WriteString( "Buttons", "Button" #number "Name", Buttons[ number ].Name ); \
    storage->WriteString( "Buttons", "Button" #number "Cmd", Buttons[ number ].Cmd );
    WRITE_BUTTON( 1 );
    WRITE_BUTTON( 2 );
    WRITE_BUTTON( 3 );
    WRITE_BUTTON( 4 );
  }
  catch(...) {}

  delete storage;
}



