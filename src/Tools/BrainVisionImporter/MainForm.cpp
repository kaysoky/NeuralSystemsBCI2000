////////////////////////////////////////////////////////////////////////////////
//
// File: MainForm.cpp
//
// Date: May 25, 2002
//
// Author: Juergen Mellinger
//
// Description: Main Form class for a drag and drop converter program that
//              reads BCI 2000 compliant EEG files and outputs files needed
//              for data import into the BrainVision Analyzer program
//
// Changes: Feb 20, 2003, jm: Removed use of PJDropFiles component.
//
////////////////////////////////////////////////////////////////////////////////

#include <vcl.h>
#pragma hdrstop

#include "BrainVisionConverters.h"
#include "MainForm.h"
#include "UBCIError.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

#include <registry.hpp>
#include <shellapi.hpp>

TImporterForm *ImporterForm;

const AnsiString appKey = "\\Software\\medpsych.uni-tuebingen.de\\BrainVisionImporter\\";
const AnsiString channelNamesKey = "ChannelNames";
const AnsiString stateListKey = "States";

static std::string ErrContent;
static std::string OutContent;

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

__fastcall TImporterForm::TImporterForm(TComponent* Owner)
    : TForm(Owner)
{
    Application->Title = Caption;
    Caption = Caption + " " + __DATE__;

    ReadSettings();

    StatesList->Hint =
      "Drop BCI2000 files here to populate the list of import states.\n"
      "Delete selected items with the Backspace or Delete key.";
    StatesList->ShowHint = true;
    StatesList->MultiSelect = true;
    defaultStatesListWindowProc = StatesList->WindowProc;
    StatesList->WindowProc = StatesListWindowProc;
    ::DragAcceptFiles( StatesList->Handle, true );

    DropPanel->Hint = Label3->Caption;
    DropPanel->ShowHint = true;
    defaultDropPanelWindowProc = DropPanel->WindowProc;
    DropPanel->WindowProc = DropPanelWindowProc;
    ::DragAcceptFiles( DropPanel->Handle, true );
    ::PostMessage( DropPanel->Handle, WM_USER, 0, 0 );

    ChannelNamesMemo->Hint =
      "Entering channel names in the usual notation allows\n"
      "creating topographic maps from the Analyzer program.";
    ChannelNamesMemo->ShowHint = true;
}

__fastcall TImporterForm::~TImporterForm()
{
    WriteSettings();
}

void TImporterForm::ProcessFiles( TStrSet& inFilesToProcess, bool scanOnly )
{
    Application->BringToFront();
    DisableAll();
    StatusBar->Panels->Items[ 0 ]->Text = "Processing";
    Refresh();

    TStrSet  statesToIgnore;
    for( int i = 0; i < StatesList->Items->Count; ++i )
        if( !StatesList->Checked[ i ] )
            statesToIgnore.insert( StatesList->Items->Strings[ i ].c_str() );

    TStrList  channelNames;
    for( int i = 0; i < ChannelNamesMemo->Lines->Count; ++i )
        channelNames.push_back( ChannelNamesMemo->Lines->Strings[ i ].c_str() );

    TBrainVisionGDRConverter    Converter;
    int curFile = 0;
    for( TStrSet::iterator i = inFilesToProcess.begin();
            i != inFilesToProcess.end(); ++i )
    {
        StatusBar->Panels->Items[ 1 ]->Text
            = IntToStr( ++curFile ) + "/" + IntToStr( inFilesToProcess.size() );
        StatusBar->Refresh();

        Converter.Open( i->c_str() );
        if( ErrContent.length() == 0 )
          Converter.Process( channelNames, statesToIgnore, scanOnly );
        if( OutContent.length() )
        {
          Application->BringToFront();
          Application->MessageBox( OutContent.c_str(), Application->Title.c_str(), MB_ICONWARNING|MB_OK );
          OutContent = "";
        }
        if( ErrContent.length() )
        {
          Application->BringToFront();
          Application->MessageBox( ErrContent.c_str(), Application->Title.c_str(), MB_ICONERROR|MB_OK );
          ErrContent = "";
          break;
        }

        TStrSet newStates = Converter.GetStates();
        for( int i = 0; i < StatesList->Items->Count; ++i )
          newStates.erase( StatesList->Items->Strings[ i ].c_str() );
        for( TStrSet::const_iterator i = newStates.begin();
                                                i != newStates.end(); ++i )
          StatesList->Checked[ StatesList->Items->Add( i->c_str() ) ] = true;
    }

    EnableAll();
    StatusBar->Panels->Items[ 0 ]->Text = "Idle";
    StatusBar->Panels->Items[ 1 ]->Text = "";
    Refresh();
}

void TImporterForm::EnableAll()
{
    for( int i = 0; i < ControlCount; ++i )
        Controls[ i ]->Enabled = true;
}

void TImporterForm::DisableAll()
{
    for( int i = 0; i < ControlCount; ++i )
        Controls[ i ]->Enabled = false;
}

void TImporterForm::ReadSettings()
{
    TRegistry   *Registry = new TRegistry;
    TStrings    *StringList = NULL;
    try
    {
        Registry->Access = KEY_READ;
        Registry->RootKey = HKEY_CURRENT_USER;
        if( Registry->OpenKey( appKey + channelNamesKey, false ) )
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
        if( Registry->OpenKey( appKey + stateListKey, false ) )
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

void TImporterForm::WriteSettings()
{
    TRegistry *Registry = new TRegistry;
    try
    {
        Registry->Access = KEY_ALL_ACCESS;
        Registry->RootKey = HKEY_CURRENT_USER;
        Registry->DeleteKey( appKey + channelNamesKey );
        if( Registry->OpenKey( appKey + channelNamesKey, true ) )
            for( int i = 0; i < ChannelNamesMemo->Lines->Count; ++i )
                Registry->WriteString( IntToStr( i ), ChannelNamesMemo->Lines->Strings[ i ] );
                
        Registry->DeleteKey( appKey + stateListKey );
        if( Registry->OpenKey( appKey + stateListKey, true ) )
            for( int i = 0; i < StatesList->Items->Count; ++i )
                Registry->WriteBool( StatesList->Items->Strings[ i ], StatesList->Checked[ i ] );
    }
    __finally
    {
        delete Registry;
    }
}

void __fastcall TImporterForm::DropPanelWindowProc( TMessage& msg )
{
  switch( msg.Msg )
  {
    case WM_DROPFILES:
      {
        HDROP handle = ( HDROP )msg.WParam;
        size_t numFiles = ::DragQueryFile( handle, -1, NULL, 0 );
        if( numFiles > 0 )
        {
          TStrSet s;
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
        TStrSet s;
        for( int i = 1; i <= ParamCount(); ++i )
            s.insert( std::string( ParamStr( i ).c_str() ) );
        ProcessFiles( s, false );
        Application->Terminate();
      }
      break;
    default:
      defaultDropPanelWindowProc( msg );
  }
}

void __fastcall TImporterForm::StatesListWindowProc( TMessage& msg )
{
  switch( msg.Msg )
  {
    case WM_DROPFILES:
      {
        HDROP handle = ( HDROP )msg.WParam;
        size_t numFiles = ::DragQueryFile( handle, -1, NULL, 0 );
        if( numFiles > 0 )
        {
          TStrSet s;
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

void __fastcall TImporterForm::StatesListKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
  if( Key == VK_DELETE || Key == VK_CLEAR )
    StatesList->DeleteSelected();
}
//---------------------------------------------------------------------------

