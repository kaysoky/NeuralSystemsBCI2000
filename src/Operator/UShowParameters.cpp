//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <Registry.hpp>

#include "..\..\shared\defines.h"       // global defines
#include "defines.h"                    // operator specific defines

#include "UShowParameters.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfShowParameters *fShowParameters;
//---------------------------------------------------------------------------
__fastcall TfShowParameters::TfShowParameters(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------


void __fastcall TfShowParameters::FormShow(TObject *Sender)
{
int parameter;

 try{
 ParameterListBox->Clear();
 // show all the parameters
 for (parameter=0; parameter < parameterlist->GetNumParameters(); parameter++)
  ParameterListBox->Items->Add(parameterlist->GetParamPtr(parameter)->GetName());
 // check the parameters that shouldn't be loaded/saved
 for (parameter=0; parameter < parameterlist->GetNumParameters(); parameter++)
  {
  if (GetFilterStatus(parameterlist->GetParamPtr(parameter), filtertype) == 1)
     ParameterListBox->Checked[parameter]=true;
  else
     ParameterListBox->Checked[parameter]=false;
  }
 } catch(...) {;}
}
//---------------------------------------------------------------------------


// retrieves the filter status of one particular parameter
// filtertype == 1 ... load filter
// filtertype == 2 ... save filter
// returns 0 (not set) or 1 (set)
int TfShowParameters::GetFilterStatus(PARAM *param, int filtertype)
{
/* registry approach ... works well, but we want an ini file
TRegistry       *my_registry;
AnsiString      keyname;
int             ret;

 my_registry=new TRegistry();

 keyname=AnsiString(KEY_BCI2000)+AnsiString(KEY_OPERATOR)+AnsiString(KEY_PARAMETERS)+"\\"+AnsiString(param->GetName());
 ret=0;
 if (my_registry->OpenKey(keyname, false))
    {
    try
     {
     if (filtertype == 1)
        ret=my_registry->ReadInteger("LoadFilter");
     else
        ret=my_registry->ReadInteger("SaveFilter");
     }
    catch(...) {;}
    }

 delete my_registry;
 return(ret); */

TIniFile        *my_registry;
int             ret;

 my_registry=new TIniFile(ExtractFilePath(Application->ExeName)+AnsiString(INIFILENAME_PARAMETERS));

 ret=0;
 try
  {
  if (filtertype == 1)
     ret=my_registry->ReadInteger(AnsiString(param->GetName()), "LoadFilter", 0);
  else
     ret=my_registry->ReadInteger(AnsiString(param->GetName()), "SaveFilter", 0);
  }
 catch(...) {;}

 delete my_registry;
 return(ret);
}



// sets the filter status of one particular parameter
// filtertype ... see GetFilterStatus
// filterstatus ... 0 ... not set; 1 ... set
void TfShowParameters::SetFilterStatus(PARAM *param, int filtertype, int filterstatus)
{
/* we dumped the registry approach ... see above
TRegistry       *my_registry;
AnsiString      keyname;

 my_registry=new TRegistry();

 try
  {
  keyname=AnsiString(KEY_BCI2000)+AnsiString(KEY_OPERATOR)+AnsiString(KEY_PARAMETERS)+"\\"+AnsiString(param->GetName());
  my_registry->CreateKey(keyname);
  }
 catch (...)
  {;}

 if (my_registry->OpenKey(keyname, false))
    {
    try
     {
     if (filtertype == 1)
        my_registry->WriteInteger("LoadFilter", filterstatus);
     else
        my_registry->WriteInteger("SaveFilter", filterstatus);
     }
    catch(...)
     {;}
    }

 delete my_registry;
 return;            */

TIniFile        *my_registry;

 my_registry=new TIniFile(ExtractFilePath(Application->ExeName)+AnsiString(INIFILENAME_PARAMETERS));

 try
  {
  if (filtertype == 1)
     my_registry->WriteInteger(AnsiString(param->GetName()), "LoadFilter", filterstatus);
  else
     my_registry->WriteInteger(AnsiString(param->GetName()), "SaveFilter", filterstatus);
  }
 catch(...)
  {;}

 delete my_registry;
 return;
}



// sets the tag value for the parameters; to be used in subsequent calls to Load/SaveParameterList
void TfShowParameters::UpdateParameterTags(PARAMLIST *paramlist, int filtertype)
{
int parameter;

 try{
 // tag each parameter, if the registry "says so"
 // these tags will be used as a filter by Load/SaveParameterList
 for (parameter=0; parameter < paramlist->GetNumParameters(); parameter++)
  {
  if (GetFilterStatus(paramlist->GetParamPtr(parameter), filtertype) == 1)
     paramlist->GetParamPtr(parameter)->tag=true;
  else
     paramlist->GetParamPtr(parameter)->tag=false;
  }
 } catch(...) {;}
}
//---------------------------------------------------------------------------



void __fastcall TfShowParameters::FormClose(TObject *Sender,
      TCloseAction &Action)
{
int parameter;

 try{
 // store the filter setting for the parameters that shouldn't be loaded/saved
 // in the registry
 for (parameter=0; parameter < parameterlist->GetNumParameters(); parameter++)
  {
  if (ParameterListBox->Checked[parameter] == true)
     SetFilterStatus(parameterlist->GetParamPtr(parameter), filtertype, 1);
  else
     SetFilterStatus(parameterlist->GetParamPtr(parameter), filtertype, 0);
  }
 } catch(...) {;}
}
//---------------------------------------------------------------------------

