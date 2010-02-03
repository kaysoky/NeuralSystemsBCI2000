/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include <Registry.hpp>

#include "defines.h"       // global defines
#include "operator.h"      // operator specific defines

#include "UShowParameters.h"
#include "BCIError.h"
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


void __fastcall TfShowParameters::FormShow(TObject*)
{
 ParameterListBox->Clear();
 // show all the parameters
 // check the parameters that shouldn't be loaded/saved
 for (int i=0; i < parameterlist->Size(); i++)
  {
  ParameterListBox->Items->Add((*parameterlist)[i].Name().c_str());
  if (GetFilterStatus(&(*parameterlist)[i], filtertype) == 1)
     ParameterListBox->Checked[i]=true;
  else
     ParameterListBox->Checked[i]=false;
  }
}
//---------------------------------------------------------------------------


// retrieves the filter status of one particular parameter
// filtertype == 1 ... load filter
// filtertype == 2 ... save filter
// returns 0 (not set) or 1 (set)
int TfShowParameters::GetFilterStatus(Param *param, int filtertype)
{
TIniFile        *my_registry;
int             ret;

 my_registry=new TIniFile(ExtractFilePath(Application->ExeName)+AnsiString(INIFILENAME_PARAMETERS));

 ret=0;
 try
  {
  if (filtertype == 1)
     ret=my_registry->ReadInteger(AnsiString(param->Name().c_str()), "LoadFilter", 0);
  else
     ret=my_registry->ReadInteger(AnsiString(param->Name().c_str()), "SaveFilter", 0);
  }
 catch(...) {;}

 delete my_registry;
 return(ret);
}



// sets the filter status of one particular parameter
// filtertype ... see GetFilterStatus
// filterstatus ... 0 ... not set; 1 ... set
void TfShowParameters::SetFilterStatus(Param *param, int filtertype, int filterstatus)
{
TIniFile        *my_registry;

 my_registry=new TIniFile(ExtractFilePath(Application->ExeName)+AnsiString(INIFILENAME_PARAMETERS));

 try
  {
  if (filtertype == 1)
     my_registry->WriteInteger(AnsiString(param->Name().c_str()), "LoadFilter", filterstatus);
  else
     my_registry->WriteInteger(AnsiString(param->Name().c_str()), "SaveFilter", filterstatus);
  }
 catch(...)
  {;}

 delete my_registry;
 return;
}


void __fastcall TfShowParameters::FormClose(TObject*, TCloseAction&)
{
 // store the filter setting for the parameters that shouldn't be loaded/saved
 // in the registry
 for (int parameter=0; parameter < parameterlist->Size(); parameter++)
  {
  if (ParameterListBox->Checked[parameter] == true)
     SetFilterStatus(&(*parameterlist)[parameter], filtertype, 1);
  else
     SetFilterStatus(&(*parameterlist)[parameter], filtertype, 0);
  }
}
//---------------------------------------------------------------------------

