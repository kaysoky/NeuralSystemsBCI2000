//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <Registry.hpp>

#include "..\shared\defines.h"
// #include "defines.h"

#include "UVisConfig.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CSPIN"
#pragma resource "*.dfm"
TfVisConfig *fVisConfig;
//---------------------------------------------------------------------------
__fastcall TfVisConfig::TfVisConfig(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------


bool TfVisConfig::SetVisualPrefs(int sourceID, int vis_type, const char* variable, int value)
{
TRegistry       *my_registry;
AnsiString      keyname;
bool    ret;

 my_registry=new TRegistry();
 ret=true;

 try
  {
  keyname=AnsiString(KEY_BCI2000)+AnsiString(KEY_OPERATOR)+AnsiString(KEY_VISUALIZATION)+"\\"+AnsiString(sourceID);
  my_registry->CreateKey(keyname);
  }
 catch (...)
  {
  ret=false;
  }

 if (my_registry->OpenKey(keyname, false))
    {
    try
     {
     my_registry->WriteInteger(variable, value);
     }
    catch(...)
     {
     ret=false;
     }
    }

 delete my_registry;
 return(ret);
}


bool TfVisConfig::SetVisualPrefs(int sourceID, int vis_type, const char* variable, double value)
{
TRegistry       *my_registry;
AnsiString      keyname;
bool    ret;

 my_registry=new TRegistry();
 ret=true;

 try
  {
  keyname=AnsiString(KEY_BCI2000)+AnsiString(KEY_OPERATOR)+AnsiString(KEY_VISUALIZATION)+"\\"+AnsiString(sourceID);
  my_registry->CreateKey(keyname);
  }
 catch (...)
  {
  ret=false;
  }

 if (my_registry->OpenKey(keyname, false))
    {
    try
     {
     my_registry->WriteFloat(variable, value);
     }
    catch(...)
     {
     ret=false;
     }
    }

 delete my_registry;
 return(ret);
}


bool TfVisConfig::SetVisualPrefs(int sourceID, int vis_type, const char* variable, const char* value)
{
TRegistry       *my_registry;
AnsiString      keyname;
bool    ret;

 my_registry=new TRegistry();
 ret=true;

 try
  {
  keyname=AnsiString(KEY_BCI2000)+AnsiString(KEY_OPERATOR)+AnsiString(KEY_VISUALIZATION)+"\\"+AnsiString(sourceID);
  my_registry->CreateKey(keyname);
  }
 catch (...)
  {
  ret=false;
  }

 if (my_registry->OpenKey(keyname, false))
    {
    try
     {
     my_registry->WriteString(variable, value);
     }
    catch(...)
     {
     ret=false;
     }
    }

 delete my_registry;
 return(ret);
}


bool TfVisConfig::GetVisualPrefs(int sourceID, int vis_type, const char* variable, int& value) const
{
TRegistry       *my_registry;
AnsiString      keyname;
bool            ret;

 my_registry=new TRegistry();

 keyname=AnsiString(KEY_BCI2000)+AnsiString(KEY_OPERATOR)+AnsiString(KEY_VISUALIZATION)+"\\"+AnsiString(sourceID);
 if (my_registry->OpenKey(keyname, false))
    {
    try
     {
     ret=false;
     if (my_registry->ValueExists(variable))
        {
        value=my_registry->ReadInteger(variable);
        ret=true;
        }
     }
    catch(...) {
     ret=false;
     }
    }

 delete my_registry;
 return(ret);
}


bool TfVisConfig::GetVisualPrefs(int sourceID, int vis_type, const char* variable, double& value) const
{
TRegistry       *my_registry;
AnsiString      keyname;
bool            ret;

 my_registry=new TRegistry();

 keyname=AnsiString(KEY_BCI2000)+AnsiString(KEY_OPERATOR)+AnsiString(KEY_VISUALIZATION)+"\\"+AnsiString(sourceID);
 ret=false;
 if (my_registry->OpenKey(keyname, false))
    {
    try
     {
     ret=false;
     if (my_registry->ValueExists(variable))
        {
        value=my_registry->ReadFloat(variable);
        ret=true;
        }
     }
    catch(...) {
     ret=false;
     }
    }

 delete my_registry;
 return(ret);
}


bool TfVisConfig::GetVisualPrefs(int sourceID, int vis_type, const char* variable, AnsiString& value) const
{
TRegistry       *my_registry;
AnsiString      keyname;
bool            ret;

 my_registry=new TRegistry();

 keyname=AnsiString(KEY_BCI2000)+AnsiString(KEY_OPERATOR)+AnsiString(KEY_VISUALIZATION)+"\\"+AnsiString(sourceID);
 if (my_registry->OpenKey(keyname, false))
    {
    try
     {
     ret=false;
     if (my_registry->ValueExists(variable))
        {
        value = my_registry->ReadString(variable);
        ret=true;
        }
     }
    catch(...) {
     ret=false;
     }
    }

 delete my_registry;
 return(ret);
}


void __fastcall TfVisConfig::bSetClick(TObject *Sender)
{
 // SetVisualPrefs(cSourceID->Value, VISTYPE_GRAPH, "Top", (int)cTop->Value);
 // SetVisualPrefs(cSourceID->Value, VISTYPE_GRAPH, "Left", (int)cLeft->Value);
 // SetVisualPrefs(cSourceID->Value, VISTYPE_GRAPH, "Width", (int)cWidth->Value);
 // SetVisualPrefs(cSourceID->Value, VISTYPE_GRAPH, "Height", (int)cHeight->Value);
 SetVisualPrefs(cSourceID->Value, VISTYPE_GRAPH, "MinValue", atof(eDisplayMin->Text.c_str()));
 SetVisualPrefs(cSourceID->Value, VISTYPE_GRAPH, "MaxValue", atof(eDisplayMax->Text.c_str()));
 SetVisualPrefs(cSourceID->Value, VISTYPE_GRAPH, "NumSamples", (int)cNumSamples->Value);
}
//---------------------------------------------------------------------------
