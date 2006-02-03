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
#include "UBCI2000Data.h"

#include <string>

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
  DisposeControls();
}
//---------------------------------------------------------------------------


int TfConfig::Initialize( PARAMLIST *my_paramlist, PREFERENCES* new_preferences )
{
  paramlist = my_paramlist;
  preferences = new_preferences;

  PARAMLIST& Parameters = *paramlist;
  Parameters.Sort();
  DeleteAllTabs();

  set<string> tabNames;
  for( size_t i = 0; i < Parameters.Size(); ++i )
    if( OperatorUtils::GetUserLevel( Parameters[ i ].GetName() ) <= preferences->UserLevel )
      tabNames.insert( Parameters[ i ].GetSection() );

  for( set<string>::const_iterator i = tabNames.begin(); i != tabNames.end(); ++i )
    CfgTabControl->Tabs->Insert( 0, i->c_str() );

  if( CfgTabControl->Tabs->Count == 0 )
  {
    CfgTabControl->Tabs->Insert( 0, "No parameter visible" );
    Application->MessageBox( "No parameter visible! Increase user level", "Message", MB_OK );
  }
  RenderParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
  return 0;
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
void TfConfig::RenderParameters( const AnsiString& inSection )
{
  DisposeControls();
  mParamDisplays.clear();

  int currentTop = ParamDisplay::BUTTON_HEIGHT / 2;
  string lastSubsection = "";
  TBevel* lastBevel = NULL;
  
  for( size_t i = 0; i < paramlist->Size(); ++i )
  {
    const PARAM& p = ( *paramlist )[ i ];
    if( inSection == p.GetSection()
        && OperatorUtils::GetUserLevel( p.GetName() ) <= preferences->UserLevel )
    {
      if( preferences->UserLevel > USERLEVEL_BEGINNER )
      {
        string subsection = p.GetSection( 1 );
        if( subsection != lastSubsection )
        {
          if( lastBevel != NULL )
          {
            lastBevel->Height = ParamDisplay::BUTTON_HEIGHT / 2 + currentTop - lastBevel->Top;
            lastBevel = NULL;
            currentTop += ParamDisplay::BUTTON_HEIGHT;
          }
          if( subsection != "" )
          {
            TBevel* bevel = new TBevel( NULL );
            mControls.insert( bevel );
            bevel->Left = LABELS_OFFSETX / 2;
            bevel->Top = currentTop;
            bevel->Parent = ScrollBox;
            bevel->Width = ScrollBox->ClientWidth - LABELS_OFFSETX;
            bevel->Anchors << akLeft << akRight << akTop;
            lastBevel = bevel;

            TLabel* label = new TLabel( NULL );
            mControls.insert( label );
            label->Font->Style << fsBold << fsItalic;
            label->Caption = subsection.c_str();
            label->Left = LABELS_OFFSETX / 2 + 2;
            label->Top = currentTop + 2;
            label->Parent = ScrollBox;
            label->Anchors << akLeft << akTop;

            currentTop += label->Height;
          }
          lastSubsection = subsection;
        }
      }
      ParamDisplay paramDisplay( p, ScrollBox );
      paramDisplay.SetLeft( LABELS_OFFSETX );
      paramDisplay.SetTop( currentTop );
      paramDisplay.ReadValuesFrom( p );
      mParamDisplays[ p.GetName() ] = paramDisplay;
      currentTop = paramDisplay.GetBottom();
    }
  }
  if( lastBevel != NULL )
    lastBevel->Height = ParamDisplay::BUTTON_HEIGHT / 2 + currentTop - lastBevel->Top;

  for( DisplayContainer::iterator i = mParamDisplays.begin();
                                                i != mParamDisplays.end(); ++i )
    i->second.Show();
    
  TBevel* bevel = new TBevel( NULL );
  mControls.insert( bevel );
  bevel->Shape = bsSpacer;
  bevel->Top = currentTop;
  bevel->Height = ParamDisplay::BUTTON_HEIGHT;
  bevel->Anchors.Clear() << akLeft << akTop;
  bevel->Parent = ScrollBox;
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
  bool modified = false;
  for( DisplayContainer::const_iterator i = mParamDisplays.begin();
         i != mParamDisplays.end(); ++i )
    if( i->second.Modified() )
    {
      i->second.WriteValuesTo( ( *paramlist )[ i->first ] );
      modified = true;
    }
  if( modified )
    ParameterChange();
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
 DisposeControls();
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
    fShowParameters->UpdateParameterTags(paramlist, 2);       // update the tag (= filter) values in each parameter
    ret=paramlist->Save(SaveDialog->FileName.c_str(), true);  // save parameters using the filter
    if (!ret)
       Application->MessageBox("Error writing parameter file", "Error", MB_OK);
    }
 else
    return;
}
//---------------------------------------------------------------------------

bool
TfConfig::LoadParameters( const AnsiString& inName )
{
  bool result = false;
  AnsiString fileExtension = ExtractFileExt( inName );
  if( fileExtension == ".dat" )
  {
    const char* tempdir = ::getenv( "TEMP" );
    if( tempdir == NULL )
      tempdir = ::getenv( "TMP" );
    if( tempdir == NULL )
      tempdir = "\\tmp";
    AnsiString name = tempdir;
    name = name + "\\" + ::tmpnam( NULL );
    BCI2000DATA file;
    if( file.Initialize( inName.c_str() ) == BCI2000ERR_NOERR )
      file.GetParamListPtr()->Save( name.c_str() );
    // do not import non-existing parameters; use the filter
    result = paramlist->Load( name.c_str(), true, false );
    ::unlink( name.c_str() );
  }
  else
    result = paramlist->Load( inName.c_str(), true, false );
  return  result;
}

void
TfConfig::DisposeControls()
{
  for( ControlContainer::iterator i = mControls.begin(); i != mControls.end(); ++i )
    delete *i;
  mControls.clear();
}

void __fastcall TfConfig::bLoadParametersClick(TObject *Sender)
{
bool    ret;

 LoadDialog->DefaultExt=".prm";
 LoadDialog->Filter="BCI2000 parameter file (*.prm)|*.prm|Any file (*.*)|*.*";
 if (LoadDialog->Execute())
    {
    if (CfgTabControl->TabIndex > -1)
       UpdateParameters();
    fShowParameters->UpdateParameterTags(paramlist, 1); // update the tag (= filter) values in each parameter
    ret=LoadParameters(LoadDialog->FileName);
    if (!ret)
       Application->MessageBox("Error reading parameter file", "Error", MB_OK);
    else
       {
       RenderParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex]);
       ParameterChange();
       }
    }
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



