////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Main Form class for a drag and drop converter program that
//   reads BCI2000 compliant EEG files and converts them into other formats.
//   To introduce support for additional formats, add a converter class
//   to the converters directory that derives from BCIReader, and register it
//   below in the sOutputFormats variable.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ASCIIConverter.h"
#include "BrainVisionConverter.h"
#include "MainForm.h"
#include "BCIError.h"
#include "AboutBox.h"
#include "ExecutableHelp.h"
#include "defines.h"
#include "VCLDefines.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

#include <registry.hpp>
#include <shellapi.hpp>

TImporterForm* ImporterForm;

const char* cProgramName = "BCI2000Export";

AnsiString appKey = KEY_BCI2000 KEY_EXPORT;
const AnsiString outputFormatKey = "OutputFormat";
const AnsiString channelNamesKey = "ChannelNames";
const AnsiString stateListKey = "States";

static std::string ErrContent;
static std::string OutContent;

static struct
{
  const char* Name;
  BCIReader* (*CreateInstance)();
  BCIReader* ConverterInstance;
} sOutputFormats[] =
{
  { "BrainVisionAnalyzer",        BrainVisionGDRConverter::CreateInstance,     NULL },
  { "ASCII (standard precision)", ASCIIConverter::CreateInstance,              NULL },
  { "ASCII (high precision)",     ASCIIConverter::CreateInstanceHighPrecision, NULL },
};


void
BCIError::Warning( const std::string& s )
{
  OutContent += s;
}

void
BCIError::ConfigurationError( const std::string& s )
{
  ErrContent += s;
}

void
BCIError::RuntimeError( const std::string& s )
{
  ErrContent += s;
}

void
BCIError::LogicError( const std::string& s )
{
  ErrContent += s;
}

__fastcall
TImporterForm::TImporterForm( TComponent* inOwner )
: TForm( inOwner )
{
  for( size_t i = 0; i < sizeof( sOutputFormats ) / sizeof( *sOutputFormats ); ++i )
  {
    FormatsBox->Items->Add( sOutputFormats[ i ].Name );
    sOutputFormats[ i ].ConverterInstance = sOutputFormats[ i ].CreateInstance();
  }
  FormatsBox->ItemIndex = 0;

  ReadSettings();

  StatesList->Hint =
    "Drop BCI2000 files here to populate the list of import states.\n"
    "Delete selected items using the Delete key.";
  StatesList->ShowHint = true;
  StatesList->MultiSelect = true;
  defaultStatesListWindowProc = StatesList->WindowProc;
  StatesList->WindowProc = StatesListWindowProc;
  ::DragAcceptFiles( StatesList->Handle, true );

  TLabel* dropLabel = new TLabel( this );
  dropLabel->Parent = DropPanel;
  dropLabel->Align = alClient;
  dropLabel->Alignment = taCenter;
  dropLabel->Layout = tlCenter;
  dropLabel->Caption =
    "Dropping files here\n"
    "will create converted files\n"
    "at the original files' location.";
  DropPanel->Hint = dropLabel->Caption;
  DropPanel->ShowHint = true;
  defaultDropPanelWindowProc = DropPanel->WindowProc;
  DropPanel->WindowProc = DropPanelWindowProc;
  ::DragAcceptFiles( DropPanel->Handle, true );
  ::PostMessage( DropPanel->Handle, WM_USER, 0, 0 );

  ChannelNamesMemo->Hint =
    "Entering channel names in the usual notation allows\n"
    "creating topographic maps from the Analyzer program.\n"
    "If present, channel names from the BCI2000 file\n"
    "will override the ones defined here.";
  ChannelNamesMemo->ShowHint = true;

  Panel->Top = 0;
  Panel->Left = 0;
  Panel->Width = ClientWidth;
}

__fastcall
TImporterForm::~TImporterForm()
{
  WriteSettings();
  for( size_t i = 0; i < sizeof( sOutputFormats ) / sizeof( *sOutputFormats ); ++i )
    delete sOutputFormats[ i ].ConverterInstance;
}

void
TImporterForm::ProcessFiles( StringSet& inFilesToProcess, bool scanOnly )
{
  bcierr__.clear();
  bciout__.clear();
  Application->BringToFront();
  DisableAll();
  StatusBar->Panels->Items[ 0 ]->Text = "Processing";
  Refresh();

  StringSet  statesToIgnore;
  for( int i = 0; i < StatesList->Items->Count; ++i )
      if( !StatesList->Checked[ i ] )
          statesToIgnore.insert( AnsiString( StatesList->Items->Strings[ i ] ).c_str() );

  StringList  channelNames;
  for( int i = 0; i < ChannelNamesMemo->Lines->Count; ++i )
      channelNames.push_back( AnsiString( ChannelNamesMemo->Lines->Strings[ i ] ).c_str() );

  int idx = FormatsBox->ItemIndex;
  BCIReader& Converter = *sOutputFormats[ idx >= 0 ? idx : 0 ].ConverterInstance;
  int curFile = 0;
  for( StringSet::iterator i = inFilesToProcess.begin();
          i != inFilesToProcess.end(); ++i )
  {
    StatusBar->Panels->Items[ 1 ]->Text
        = IntToStr( ++curFile ) + "/" + IntToStr( int( inFilesToProcess.size() ) );
    StatusBar->Refresh();

    Converter.Open( i->c_str() );
    if( ErrContent.length() == 0 )
      Converter.Process( channelNames, statesToIgnore, scanOnly );
    if( OutContent.length() )
    {
      Application->BringToFront();
	  AnsiString title = Application->Title;
	  Application->MessageBox(
		VCLSTR( OutContent.c_str() ),
		VCLSTR( title.c_str() ),
		MB_ICONWARNING|MB_OK );
      OutContent = "";
    }
    if( ErrContent.length() )
    {
      Application->BringToFront();
	  AnsiString title = Application->Title;
	  Application->MessageBox(
		VCLSTR( ErrContent.c_str() ),
		VCLSTR( title.c_str() ),
		MB_ICONERROR|MB_OK );
      ErrContent = "";
      break;
    }

    StringSet newStates = Converter.GetStates();
    for( int i = 0; i < StatesList->Items->Count; ++i )
      newStates.erase( AnsiString( StatesList->Items->Strings[ i ] ).c_str() );
    for( StringSet::const_iterator i = newStates.begin(); i != newStates.end(); ++i )
      StatesList->Checked[ StatesList->Items->Add( i->c_str() ) ] = true;
  }

  EnableAll();
  StatusBar->Panels->Items[ 0 ]->Text = "Idle";
  StatusBar->Panels->Items[ 1 ]->Text = "";
  Refresh();
}

void
TImporterForm::EnableAll()
{
  for( int i = 0; i < ControlCount; ++i )
    Controls[ i ]->Enabled = true;
}

void
TImporterForm::DisableAll()
{
  for( int i = 0; i < ControlCount; ++i )
    Controls[ i ]->Enabled = false;
}

void
TImporterForm::ReadSettings()
{
  TRegistry   *Registry = new TRegistry;
  TStrings    *StringList = NULL;
  try
  {
    Registry->Access = KEY_READ;
    Registry->RootKey = HKEY_CURRENT_USER;
    if( Registry->OpenKey( appKey + "\\" + outputFormatKey, false ) )
    {
      int idx = FormatsBox->Items->IndexOf( Registry->ReadString( outputFormatKey ) );
      if( idx != -1 )
        FormatsBox->ItemIndex = idx;
    }
    if( Registry->OpenKey( appKey + "\\" + channelNamesKey, false ) )
    {
      ChannelNamesMemo->Text = "";
      int i = 0;
      AnsiString s = Registry->ReadString( IntToStr( i ) );
      while( s != "" )
      {
        ChannelNamesMemo->Lines->Add( s );
        s = Registry->ReadString( IntToStr( ++i ) );
      }
    }
    Registry->Access = KEY_READ | KEY_WRITE;
    if( Registry->OpenKey( appKey + "\\" + stateListKey, false ) )
    {
      StringList = new TStringList;
      Registry->GetValueNames( StringList );
      for( int i = 0; i < StatesList->Items->Count; ++i )
        if( StringList->IndexOf( StatesList->Items->Strings[ i ] ) == -1 )
        {
          StringList->Add( StatesList->Items->Strings[ i ] );
          Registry->WriteBool( StatesList->Items->Strings[ i ], StatesList->Checked[ i ] );
        }
      StatesList->Items->Assign( StringList );
      for( int i = 0; i < StatesList->Items->Count; ++i )
        StatesList->Checked[ i ] = Registry->ReadBool( StatesList->Items->Strings[ i ] );
    }
  }
  __finally
  {
    delete Registry;
    delete StringList;
  }
}

void
TImporterForm::WriteSettings()
{
  TRegistry *Registry = new TRegistry;
  try
  {
    Registry->Access = KEY_ALL_ACCESS;
    Registry->RootKey = HKEY_CURRENT_USER;

    Registry->DeleteKey( appKey + "\\" + outputFormatKey );
    if( Registry->OpenKey( appKey + "\\" + outputFormatKey, true ) )
      Registry->WriteString( outputFormatKey, FormatsBox->Items->Strings[ FormatsBox->ItemIndex ] );

    Registry->DeleteKey( appKey + "\\" + channelNamesKey );
    if( Registry->OpenKey( appKey + "\\" + channelNamesKey, true ) )
      for( int i = 0; i < ChannelNamesMemo->Lines->Count; ++i )
        Registry->WriteString( IntToStr( i ), ChannelNamesMemo->Lines->Strings[ i ] );

    Registry->DeleteKey( appKey + "\\" + stateListKey );
    if( Registry->OpenKey( appKey + "\\" + stateListKey, true ) )
      for( int i = 0; i < StatesList->Items->Count; ++i )
        Registry->WriteBool( StatesList->Items->Strings[ i ], StatesList->Checked[ i ] );
  }
  __finally
  {
    delete Registry;
  }
}

void
__fastcall
TImporterForm::DropPanelWindowProc( TMessage& msg )
{
  switch( msg.Msg )
  {
    case WM_DROPFILES:
      {
        HDROP handle = ( HDROP )msg.WParam;
        size_t numFiles = ::DragQueryFile( handle, -1, NULL, 0 );
        if( numFiles > 0 )
        {
          StringSet s;
          for( size_t i = 0; i < numFiles; ++i )
          {
            size_t nameLen = ::DragQueryFile( handle, i, NULL, 0 );
            char* name = new char[ nameLen + 1 ];
            ::DragQueryFile( handle, i, name, nameLen + 1 );
            s.insert( std::string( name ) );
            delete[] name;
          }
          ProcessFiles( s, false );
        }
        ::DragFinish( handle );
      }
      break;
    case WM_USER:
      // If called with command line arguments, process the files and quit.
      if( ParamCount() > 0 )
      {
        StringSet s;
        for( int i = 1; i <= ParamCount(); ++i )
            s.insert( std::string( AnsiString( ParamStr( i ) ).c_str() ) );
        ProcessFiles( s, false );
        Application->Terminate();
      }
      break;
    default:
      defaultDropPanelWindowProc( msg );
  }
}

void
__fastcall
TImporterForm::StatesListWindowProc( TMessage& msg )
{
  switch( msg.Msg )
  {
    case WM_DROPFILES:
      {
        HDROP handle = ( HDROP )msg.WParam;
        size_t numFiles = ::DragQueryFile( handle, -1, NULL, 0 );
        if( numFiles > 0 )
        {
          StringSet s;
          for( size_t i = 0; i < numFiles; ++i )
          {
            size_t nameLen = ::DragQueryFile( handle, i, NULL, 0 );
            char* name = new char[ nameLen + 1 ];
            ::DragQueryFile( handle, i, name, nameLen + 1 );
            s.insert( std::string( name ) );
            delete[] name;
          }
          ProcessFiles( s, true );
        }
        ::DragFinish( handle );
      }
      break;
    default:
      defaultStatesListWindowProc( msg );
  }
}

void
__fastcall
TImporterForm::StatesListKeyDown( TObject*, WORD &Key, TShiftState )
{
  if( Key == VK_DELETE || Key == VK_CLEAR )
    StatesList->DeleteSelected();
}
//---------------------------------------------------------------------------

void
__fastcall
TImporterForm::HelpAboutClick( TObject* )
{
  AboutBox()
  .SetApplicationName( cProgramName )
  .Display();
}
//---------------------------------------------------------------------------

void
__fastcall
TImporterForm::HelpOpenHelpClick( TObject* )
{
  ExecutableHelp().Display();
}
//---------------------------------------------------------------------------

void
__fastcall
TImporterForm::FileOpenClick( TObject* )
{
  TOpenDialog* pDialog = new TOpenDialog( static_cast<TComponent*>( NULL ) );
  pDialog->Title = "Choose BCI2000 data file(s) to convert...";
  pDialog->Options << ofAllowMultiSelect << ofFileMustExist;
  pDialog->Filter = "BCI2000 data files|*.dat|All files|*.*";
  if( pDialog->Execute() )
  {
    StringSet files;
    for( int i = 0; i < pDialog->Files->Count; ++i )
      files.insert( AnsiString( pDialog->Files->Strings[ i ] ).c_str() );
    if( !files.empty() )
      ProcessFiles( files, false );
  }
  delete pDialog;
}
//---------------------------------------------------------------------------

void
__fastcall
TImporterForm::FileQuitClick( TObject* )
{
  Application->Terminate();
}
//---------------------------------------------------------------------------


