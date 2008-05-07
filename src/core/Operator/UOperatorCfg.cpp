/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "defines.h"
#include "UShowParameters.h"
#include "UEditMatrix.h"
#include "UOperatorCfg.h"
#include "BCIError.h"
#include "UOperatorUtils.h"
#include "UPreferences.h"
#include "BCI2000FileReader.h"
#include "ExecutableHelp.h"

#include <string>
#include <vector>
#include <algorithm>

using namespace std;
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfConfig *fConfig;

//---------------------------------------------------------------------------

__fastcall TfConfig::TfConfig(TComponent* Owner) : TForm(Owner)
{
  OperatorUtils::RestoreControl( this, KEY_OPERATOR );
  if( ExecutableHelp().ParamHelp().Empty() )
    this->bHelp->Visible = false;
}

__fastcall TfConfig::~TfConfig()
{
  OperatorUtils::SaveControl( this, KEY_OPERATOR );
  DisposeControls();
}

//---------------------------------------------------------------------------

static bool CompareTabNames( const string& s1, const string& s2 )
{
  static const char* leftTabs_[] =
  {
    "Visualize", "System", "Storage",
  };
  static vector<string> leftTabs(
    leftTabs_, leftTabs_ + sizeof( leftTabs_ ) / sizeof( *leftTabs_ )
  );
  vector<string>::const_iterator i1 = find( leftTabs.begin(), leftTabs.end(), s1 ),
                                 i2 = find( leftTabs.begin(), leftTabs.end(), s2 );
  return i1 < i2;
}

int TfConfig::Initialize( ParamList *my_paramlist, PREFERENCES* new_preferences )
{
  paramlist = my_paramlist;
  preferences = new_preferences;

  ParamList& Parameters = *paramlist;
  Parameters.Sort();
  DeleteAllTabs();

  vector<string> tabNames;
  for( int i = 0; i < Parameters.Size(); ++i )
    if( OperatorUtils::GetUserLevel( Parameters[ i ].Name().c_str() ) <= preferences->UserLevel )
    {
      string tabName = Parameters[ i ].Section();
      if( find( tabNames.begin(), tabNames.end(), tabName ) == tabNames.end() )
        tabNames.push_back( tabName );
    }
  stable_sort( tabNames.begin(), tabNames.end(), CompareTabNames );

  for( vector<string>::const_iterator i = tabNames.begin(); i != tabNames.end(); ++i )
    CfgTabControl->Tabs->Append( i->c_str() );

  if( CfgTabControl->Tabs->Count == 0 )
  {
    CfgTabControl->Tabs->Insert( 0, "No parameter visible" );
    Application->MessageBox( "No parameter visible! Increase user level", "Message", MB_OK );
  }

  if( find( tabNames.begin(), tabNames.end(), mCurTab ) == tabNames.end() )
    mCurTab = "Storage";
  size_t curTabIdx = find( tabNames.begin(), tabNames.end(), mCurTab ) - tabNames.begin();
  if( curTabIdx != tabNames.size() )
    CfgTabControl->TabIndex = curTabIdx;
  mCurTab = CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex].c_str();
  RenderParameters( mCurTab );
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
void TfConfig::RenderParameters( const string& inSection )
{
  DisposeControls();
  mParamDisplays.clear();

  int currentTop = ParamDisplay::buttonHeight / 2;
  string lastSubsection = "";
  TBevel* lastBevel = NULL;

  map<string, int>      subsectionIndex;
  vector<string>        subsectionTable;
  vector< vector<int> > subsectionGroups;

  for( int i = 0; i < paramlist->Size(); ++i )
  {
    const Param& p = ( *paramlist )[ i ];
    if( inSection == p.Section()
        && OperatorUtils::GetUserLevel( p.Name().c_str() ) <= preferences->UserLevel )
    {
      string subsection;
      if( p.Sections().size() > 1 )
        subsection = p.Sections()[ 1 ];
      if( subsectionIndex.find( subsection ) == subsectionIndex.end() )
      {
        subsectionIndex[ subsection ] = subsectionTable.size();
        subsectionTable.push_back( subsection );
        subsectionGroups.resize( subsectionGroups.size() + 1 );
      }
      subsectionGroups[ subsectionIndex[ subsection ] ].push_back( i );
    }
  }

  for( size_t i = 0; i < subsectionTable.size(); ++i )
  {
    const string& subsection = subsectionTable[ i ];
    if( preferences->UserLevel > USERLEVEL_BEGINNER )
    { // A bevel for each subsection.
      if( lastBevel != NULL )
      {
        lastBevel->Height = ParamDisplay::buttonHeight / 2 + currentTop - lastBevel->Top;
        lastBevel = NULL;
        currentTop += ParamDisplay::buttonHeight;
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
        label->Font->Style = TFontStyles() << fsItalic;
        label->Caption = subsection.c_str();
        label->Left = LABELS_OFFSETX / 2 + 2;
        label->Top = currentTop + 2;
        label->Parent = ScrollBox;
        label->Anchors << akLeft << akTop;

        currentTop += label->Height;
      }
    }
    int groupIndex = subsectionIndex[ subsection ];
    for( size_t j = 0; j < subsectionGroups[ groupIndex ].size(); ++j )
    {
      int paramIndex = subsectionGroups[ groupIndex ][ j ];
      const Param& p = ( *paramlist )[ paramIndex ];
      ParamDisplay paramDisplay( p, ScrollBox );
      paramDisplay.SetLeft( LABELS_OFFSETX );
      paramDisplay.SetTop( currentTop );
      paramDisplay.ReadValuesFrom( p );
      mParamDisplays[ p.Name() ] = paramDisplay;
      currentTop = paramDisplay.GetBottom();
    }
  }
  if( lastBevel != NULL )
    lastBevel->Height = ParamDisplay::buttonHeight / 2 + currentTop - lastBevel->Top;

  for( DisplayContainer::iterator i = mParamDisplays.begin();
                                   i != mParamDisplays.end(); ++i )
    i->second.Show();
    
  TBevel* bevel = new TBevel( NULL );
  mControls.insert( bevel );
  bevel->Shape = bsSpacer;
  bevel->Top = currentTop;
  bevel->Height = ParamDisplay::buttonHeight;
  bevel->Anchors.Clear() << akLeft << akTop;
  bevel->Parent = ScrollBox;
}

// update one particular parameter on the screen
// useful, for example, if parameters change while stuff on screen
void TfConfig::RenderParameter( Param *inParam )
{
  if( !Visible )
    return;

  if( mParamDisplays.find( inParam->Name() ) != mParamDisplays.end() )
    mParamDisplays[ inParam->Name() ].ReadValuesFrom( *inParam );
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

void __fastcall TfConfig::CfgTabControlChange(TObject*)
{
  mCurTab = CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex].c_str();
  RenderParameters( mCurTab );
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::FormClose(TObject*, TCloseAction&)
{
 if (CfgTabControl->TabIndex > -1)
    UpdateParameters();
 mParamDisplays.clear();
 DisposeControls();
 DeleteAllTabs();
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::CfgTabControlChanging(TObject*, bool &AllowChange)
{
 if (CfgTabControl->TabIndex > -1)
    UpdateParameters();
 AllowChange = true;
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::bSaveParametersClick(TObject*)
{
 if (SaveDialog->Execute())
  {
  if (CfgTabControl->TabIndex > -1)
     UpdateParameters();

  ParamList paramsToSave;
  for( int i = 0; i < paramlist->Size(); ++i )
    if( !fShowParameters->GetFilterStatus( &( *paramlist )[ i ], 2 ) )
      paramsToSave.Add( ( *paramlist )[ i ] );

  bool ret=paramsToSave.Save(SaveDialog->FileName.c_str());  // save parameters using the filter
  if (!ret)
     Application->MessageBox("Error writing parameter file", "Error", MB_OK);
  }
}
//---------------------------------------------------------------------------

bool
TfConfig::LoadParameters( const AnsiString& inName )
{
  ParamList paramsFromFile = *paramlist;
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
    BCI2000FileReader file( inName.c_str() );
    if( file.IsOpen() )
      file.Parameters()->Save( name.c_str() );
    // do not import non-existing parameters
    result = paramsFromFile.Load( name.c_str(), false );
    ::unlink( name.c_str() );
  }
  else
    result = paramsFromFile.Load( inName.c_str(), false );

  for( int i = 0; i < paramsFromFile.Size(); ++i )
    if( !fShowParameters->GetFilterStatus( &paramsFromFile[ i ], 1 ) )
      ( *paramlist )[ paramsFromFile[ i ].Name() ].AssignValues( paramsFromFile[ i ] );

  return  result;
}

void
TfConfig::DisposeControls()
{
  for( ControlContainer::iterator i = mControls.begin(); i != mControls.end(); ++i )
    delete *i;
  mControls.clear();
}

void __fastcall TfConfig::bLoadParametersClick(TObject*)
{
bool    ret;

 LoadDialog->DefaultExt=".prm";
 LoadDialog->Filter="BCI2000 parameter file (*.prm)|*.prm|Any file (*.*)|*.*";
 if (LoadDialog->Execute())
    {
    if (CfgTabControl->TabIndex > -1)
       UpdateParameters();

    ret=LoadParameters(LoadDialog->FileName);
    if (!ret)
       Application->MessageBox("Error reading parameter file", "Error", MB_OK);
    else
       {
       RenderParameters(CfgTabControl->Tabs->Strings[CfgTabControl->TabIndex].c_str());
       ParameterChange();
       }
    }
}

//---------------------------------------------------------------------------

void __fastcall TfConfig::bConfigureSaveFilterClick(TObject*)
{
 fShowParameters->parameterlist=paramlist;
 fShowParameters->filtertype=2;                 // filter for saving parameters
 fShowParameters->Caption="Save Filter";
 Application->MessageBox("The parameters that you select here will NOT be saved !", "Reminder", MB_OK);
 fShowParameters->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TfConfig::bConfigureLoadFilterClick(TObject*)
{
 fShowParameters->parameterlist=paramlist;
 fShowParameters->filtertype=1;                 // filter for loading parameters
 fShowParameters->Caption="Load Filter";
 Application->MessageBox("The parameters that you select here will NOT be loaded !", "Reminder", MB_OK);
 fShowParameters->ShowModal();
}

void __fastcall TfConfig::bHelpClick(TObject*)
{
  Perform( WM_SYSCOMMAND, SC_CONTEXTHELP, 0 );
}
//---------------------------------------------------------------------------

