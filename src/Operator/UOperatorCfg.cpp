#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "..\shared\defines.h"
#include "UShowParameters.h"
#include "UEditMatrix.h"
#include "UOperatorCfg.h"
#include "UBCIError.h"
#include "UOperatorUtils.h"
#include "UPreferences.h"

using namespace std;
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfConfig *fConfig;
//---------------------------------------------------------------------------

__fastcall TfConfig::TfConfig(TComponent* Owner) : TForm(Owner)
{
  OperatorUtils::RestoreControl( this );
}

__fastcall TfConfig::~TfConfig()
{
  OperatorUtils::SaveControl( this );
}
//---------------------------------------------------------------------------


int TfConfig::Initialize(PARAMLIST *my_paramlist, PREFERENCES* new_preferences )
{
int             i, num_param;
PARAM           *cur_param;
AnsiString      tabname;

 paramlist=my_paramlist;
 num_param=paramlist->GetNumParameters();
 preferences = new_preferences;
 
 DeleteAllTabs();

 // first, set archive to false
 for (i=0; i<num_param; i++)
  {
  cur_param=paramlist->GetParamPtr(i);
  if (cur_param)
     cur_param->archive=false;
  }

 // go through all the parameters and find out about the section names
 // set the Tab captions to the section names
 // use archive to flag, whether we already processed a parameter's section
 tabname="x";
 while (tabname != " ")
  {
  tabname=" ";
  for (i=0; i<num_param; i++)
   {
   cur_param=paramlist->GetParamPtr(i);
   if (cur_param)
      if (cur_param->Valid())
         {
         // parameter has not been 'touched' yet and it's user level is smaller than the operator's user level
         if ((cur_param->archive == false)
           && (OperatorUtils::GetUserLevel(cur_param->GetName()) <= preferences->UserLevel))
            {
            // a 'new' Section name ?
            if (tabname == " ")
               {
               tabname=cur_param->GetSection();
               cur_param->archive=true;
               }
            else
               {
               // 'touch' all parameters with the same section name
               if (tabname == cur_param->GetSection())
                  cur_param->archive=true;
               }
            }
         }
   }
  // create a new tab with the section name
  if (tabname != " ")
     CfgTabControl->Tabs->Insert(0, tabname);
  }

 if (CfgTabControl->Tabs->Count == 0)
    {
    CfgTabControl->Tabs->Insert(0, "No parameter visible");
    Application->MessageBox("No parameter visible! Increase user level", "Message", MB_OK);
    }

 RenderParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
 return(0);
}

void TfConfig::DeleteAllTabs()
{
int     i, num_tabs;

 // delete old Tabs, if present
 num_tabs=CfgTabControl->Tabs->Count;
 for (i=0; i<num_tabs; i++)
  CfgTabControl->Tabs->Delete(0);
}

// render all parameters in a particular section on the screen
void TfConfig::RenderParameters( const AnsiString& section )
{
  mParamDisplays.clear();
  int currentTop = LABELS_OFFSETY;
  for( PARAMLIST::const_iterator i = paramlist->begin(); i != paramlist->end(); ++i )
  {
    const PARAM& p = i->second;
    if( section == p.GetSection()
        && OperatorUtils::GetUserLevel( p.GetName() ) <= preferences->UserLevel )
    {
      ParamDisplay paramDisplay( p, CfgTabControl );
      paramDisplay.SetLeft( LABELS_OFFSETX );
      paramDisplay.SetTop( currentTop );
      paramDisplay.ReadValuesFrom( p );
      mParamDisplays[ i->first ] = paramDisplay;
      currentTop = paramDisplay.GetBottom();
    }
  }
  for( DisplayContainer::iterator i = mParamDisplays.begin();
                                                i != mParamDisplays.end(); ++i )
    i->second.Show();
    
  if( CfgTabControl->Height < currentTop )
    CfgTabControl->Height = currentTop + ParamDisplay::BUTTON_HEIGHT;
}

// update one particular parameter on the screen
// useful, for example, if parameters change while stuff on screen
void TfConfig::RenderParameter( PARAM *inParam )
{
  if( !Visible )
    return;

  if( mParamDisplays.find( inParam->GetName() ) != mParamDisplays.end() )
    mParamDisplays[ inParam->GetName() ].ReadValuesFrom( *inParam );
}

// go through the parameters on the screen and update the parameters using the data on the screen
void TfConfig::UpdateParameters()
{
  for( DisplayContainer::const_iterator i = mParamDisplays.begin();
         i != mParamDisplays.end(); ++i )
    i->second.WriteValuesTo( ( *paramlist )[ i->first ] );
}

void __fastcall TfConfig::CfgTabControlChange(TObject *Sender)
{
 RenderParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::FormClose(TObject *Sender, TCloseAction &Action)
{
 if (CfgTabControl->TabIndex > -1)
    UpdateParameters();
 mParamDisplays.clear();
 DeleteAllTabs();
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::CfgTabControlChanging(TObject *Sender,
      bool &AllowChange)
{
 if (CfgTabControl->TabIndex > -1)
    UpdateParameters();
 AllowChange = true;
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::bSaveParametersClick(TObject *Sender)
{
bool    ret;

 if (SaveDialog->Execute())
    {
    if (CfgTabControl->TabIndex > -1)
       UpdateParameters();
    fShowParameters->UpdateParameterTags(paramlist, 2);                         // update the tag (= filter) values in each parameter
    ret=paramlist->SaveParameterList(SaveDialog->FileName.c_str(), true);       // save parameters using the filter
    if (!ret)
       Application->MessageBox("Error writing parameter file", "Error", MB_OK);
    }
 else
    return;
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::bLoadParametersClick(TObject *Sender)
{
bool    ret;

 LoadDialog->DefaultExt=".prm";
 LoadDialog->Filter="BCI2000 parameter file (*.prm)|*.prm|Any file (*.*)|*.*";
 if (LoadDialog->Execute())
    {
    if (CfgTabControl->TabIndex > -1)
       UpdateParameters();
    fShowParameters->UpdateParameterTags(paramlist, 1);    // update the tag (= filter) values in each parameter
    ret=paramlist->LoadParameterList(LoadDialog->FileName.c_str(), true, false);        // do not import non-existing parameters; use the filter
    if (!ret)
       Application->MessageBox("Error reading parameter file", "Error", MB_OK);
    else
       RenderParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
    }
 else
    return;
}

//---------------------------------------------------------------------------

void __fastcall TfConfig::bConfigureSaveFilterClick(TObject *Sender)
{
 fShowParameters->parameterlist=paramlist;
 fShowParameters->filtertype=2;                 // filter for saving parameters
 fShowParameters->Caption="Save Filter";
 Application->MessageBox("The parameters that you select here will NOT be saved !", "Reminder", MB_OK);
 fShowParameters->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::bConfigureLoadFilterClick(TObject *Sender)
{
 fShowParameters->parameterlist=paramlist;
 fShowParameters->filtertype=1;                 // filter for loading parameters
 fShowParameters->Caption="Load Filter";
 Application->MessageBox("The parameters that you select here will NOT be loaded !", "Reminder", MB_OK);
 fShowParameters->ShowModal();
}

