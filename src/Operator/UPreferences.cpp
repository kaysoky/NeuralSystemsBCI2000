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
 strncpy(preferences->Script_AfterModulesConnected, eAfterModulesConnected->Text.c_str(), 255);
 strncpy(preferences->Script_OnExit, eExit->Text.c_str(), 255);
 strncpy(preferences->Script_OnResume, eOnResume->Text.c_str(), 255);
 strncpy(preferences->Script_OnSuspend, eOnSuspend->Text.c_str(), 255);
 strncpy(preferences->Script_OnStart, eOnStart->Text.c_str(), 255);

 strncpy(preferences->Button1_Name, eButton1Name->Text.c_str(), 255);
 strncpy(preferences->Button1_Cmd, eButton1Cmd->Text.c_str(), 255);
 strncpy(preferences->Button2_Name, eButton2Name->Text.c_str(), 255);
 strncpy(preferences->Button2_Cmd, eButton2Cmd->Text.c_str(), 255);
 strncpy(preferences->Button3_Name, eButton3Name->Text.c_str(), 255);
 strncpy(preferences->Button3_Cmd, eButton3Cmd->Text.c_str(), 255);
 strncpy(preferences->Button4_Name, eButton4Name->Text.c_str(), 255);
 strncpy(preferences->Button4_Cmd, eButton4Cmd->Text.c_str(), 255);

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

 eAfterModulesConnected->Text=preferences->Script_AfterModulesConnected;
 eExit->Text=preferences->Script_OnExit;
 eOnResume->Text=preferences->Script_OnResume;
 eOnSuspend->Text=preferences->Script_OnSuspend;
 eOnStart->Text=preferences->Script_OnStart;

 eButton1Name->Text=preferences->Button1_Name;
 eButton1Cmd->Text=preferences->Button1_Cmd;
 eButton2Name->Text=preferences->Button2_Name;
 eButton2Cmd->Text=preferences->Button2_Cmd;
 eButton3Name->Text=preferences->Button3_Name;
 eButton3Cmd->Text=preferences->Button3_Cmd;
 eButton4Name->Text=preferences->Button4_Name;
 eButton4Cmd->Text=preferences->Button4_Cmd;
}
//---------------------------------------------------------------------------



PREFERENCES::PREFERENCES()
{
 UserLevel=USERLEVEL_BEGINNER;
}


// retrieves the default user settings
void PREFERENCES::GetDefaultSettings()
{
TIniFile        *my_registry;

 my_registry=new TIniFile(ExtractFilePath(Application->ExeName)+AnsiString(INIFILENAME_PREFERENCES));

 try
  {
  UserLevel=my_registry->ReadInteger("Various", "DefaultUserLevel", USERLEVEL_ADVANCED);
  strcpy(Script_AfterModulesConnected, my_registry->ReadString("Scripts", "AfterModulesConnected", "").c_str());
  strcpy(Script_OnExit, my_registry->ReadString("Scripts", "OnExit", "").c_str());
  strcpy(Script_OnResume, my_registry->ReadString("Scripts", "OnResume", "").c_str());
  strcpy(Script_OnSuspend, my_registry->ReadString("Scripts", "OnSuspend", "").c_str());
  strcpy(Script_OnStart, my_registry->ReadString("Scripts", "OnStart", "").c_str());
  strcpy(Button1_Name, my_registry->ReadString("Buttons", "Button1Name", "").c_str());
  strcpy(Button1_Cmd, my_registry->ReadString("Buttons", "Button1Cmd", "").c_str());
  strcpy(Button2_Name, my_registry->ReadString("Buttons", "Button2Name", "").c_str());
  strcpy(Button2_Cmd, my_registry->ReadString("Buttons", "Button2Cmd", "").c_str());
  strcpy(Button3_Name, my_registry->ReadString("Buttons", "Button3Name", "").c_str());
  strcpy(Button3_Cmd, my_registry->ReadString("Buttons", "Button3Cmd", "").c_str());
  strcpy(Button4_Name, my_registry->ReadString("Buttons", "Button4Name", "").c_str());
  strcpy(Button4_Cmd, my_registry->ReadString("Buttons", "Button4Cmd", "").c_str());
  }
 catch(...) {;}

 delete my_registry;
}



// sets the default user settings
void PREFERENCES::SetDefaultSettings()
{
TIniFile       *my_registry;

 my_registry=new TIniFile(ExtractFilePath(Application->ExeName)+AnsiString(INIFILENAME_PREFERENCES));

 try
  {
  my_registry->WriteInteger("Various", "DefaultUserLevel", UserLevel);
  my_registry->WriteString("Scripts", "AfterModulesConnected", AnsiString(Script_AfterModulesConnected));
  my_registry->WriteString("Scripts", "OnExit", AnsiString(Script_OnExit));
  my_registry->WriteString("Scripts", "OnResume", AnsiString(Script_OnResume));
  my_registry->WriteString("Scripts", "OnSuspend", AnsiString(Script_OnSuspend));
  my_registry->WriteString("Scripts", "OnStart", AnsiString(Script_OnStart));
  my_registry->WriteString("Buttons", "Button1Name", AnsiString(Button1_Name));
  my_registry->WriteString("Buttons", "Button1Cmd", AnsiString(Button1_Cmd));
  my_registry->WriteString("Buttons", "Button2Name", AnsiString(Button2_Name));
  my_registry->WriteString("Buttons", "Button2Cmd", AnsiString(Button2_Cmd));
  my_registry->WriteString("Buttons", "Button3Name", AnsiString(Button3_Name));
  my_registry->WriteString("Buttons", "Button3Cmd", AnsiString(Button3_Cmd));
  my_registry->WriteString("Buttons", "Button4Name", AnsiString(Button4_Name));
  my_registry->WriteString("Buttons", "Button4Cmd", AnsiString(Button4_Cmd));
  }
 catch(...)
  {;}

 delete my_registry;
}

