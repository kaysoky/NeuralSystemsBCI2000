/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "BCI2000FileReader.h"
#include "UOperatorCfg.h"
#include "UMain.h"
#include "AboutBox.h"
#include "ExecutableHelp.h"
#include "VCLDefines.h"

#include <sstream>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

using namespace std;

TfMain *fMain;

static const char* cProgramName = "BCI2000FileInfo";

//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
: TForm(Owner)
{
}
//---------------------------------------------------------------------------


void TfMain::EnableAll()
{
    for( int i = 0; i < ControlCount; ++i )
        Controls[ i ]->Enabled = true;
}

void TfMain::DisableAll()
{
    for( int i = 0; i < ControlCount; ++i )
        Controls[ i ]->Enabled = false;
}



void __fastcall TfMain::DropPanelWindowProc( TMessage& msg )
{
  switch( msg.Msg )
  {
    case WM_DROPFILES:
      {
        HDROP handle = ( HDROP )msg.WParam;
        size_t numFiles = ::DragQueryFile( handle, -1, NULL, 0 );
        if( numFiles > 1 )
		  Application->MessageBox(
			VCLSTR( "You can only drop one file at a time" ),
			VCLSTR( "Warning" ),
			MB_OK );
        if( numFiles > 0 )
        {
          size_t nameLen = ::DragQueryFile( handle, 0, NULL, 0 );
          char* name = new char[ nameLen + 1 ];
          ::DragQueryFile( handle, 0, name, nameLen + 1 );
          ProcessFile( name );
          delete[] name;
        }
        ::DragFinish( handle );
      }
      break;
    case WM_USER:
      // If called with command line arguments, process the files and quit.
      if( ParamCount() > 0 )
        ProcessFile( AnsiString( ParamStr( 1 ) ).c_str() );
      break;
    default:
      defaultDropPanelWindowProc( msg );
  }
}


void TfMain::ProcessFile( const char* inFileToProcess )
{
    Application->BringToFront();

    tFileName->Caption = inFileToProcess;
    if( RetrieveFileInfo( inFileToProcess ) )
      DisplayFileInfo();
}


bool TfMain::RetrieveFileInfo( const char* inFileName )
{
  mFile.Open( inFileName );
  if( !mFile.IsOpen() )
  {
    ostringstream oss;
    oss << "Could not open \"" << inFileName << "\" as a BCI2000 data file.";
	Application->MessageBox(
	  VCLSTR( oss.str().c_str() ),
	  VCLSTR( "Error" ),
	  MB_OK );
    bShowParams->Enabled=false;
    return(false);
  }
  else
    mPreferences.GetDefaultSettings();

  bShowParams->Enabled=true;
  return(true);
}

void __fastcall TfMain::bShowParamsClick(TObject*)
{
 mParamList = *mFile.Parameters();
 fConfig->Initialize( &mParamList, &mPreferences );
 fConfig->Show();
}
//---------------------------------------------------------------------------


void __fastcall TfMain::FormShow(TObject*)
{
 DropPanel->Hint = "Drop BCI2000 Files here";
 DropPanel->ShowHint = true;
 defaultDropPanelWindowProc = DropPanel->WindowProc;
 DropPanel->WindowProc = DropPanelWindowProc;
 ::DragAcceptFiles( DropPanel->Handle, true );
 ::PostMessage( DropPanel->Handle, WM_USER, 0, 0 );
}
//---------------------------------------------------------------------------

void __fastcall TfMain::FormClose(TObject*, TCloseAction&)
{
 DropPanel->WindowProc = defaultDropPanelWindowProc;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::Quit1Click(TObject*)
{
 Close();
}
//---------------------------------------------------------------------------

void TfMain::DisplayFileInfo()
{
  if( mFile.IsOpen() )
  {
    float blockSize = mFile.SignalProperties().Elements();
    if( blockSize > 0 )
    {
      tSampleBlockSize->Caption = mFile.Parameter( "SampleBlockSize" ).c_str();
      tSamplingRate->Caption = AnsiString( mFile.Parameter( "SamplingRate" ).c_str() ) + " Hz";
      float updaterate = mFile.SamplingRate() / blockSize;
      tUpdateRate->Caption=AnsiString::FormatFloat("#.##", updaterate)+" Hz";
      tFileFormat->Caption=mFile.FileFormatVersion().c_str();
      tDataFormat->Caption=mFile.SignalProperties().Type().Name();
    }
  }
}


void __fastcall TfMain::Open1Click(TObject*)
{
if (OpenDialog1->Execute())
   {
   AnsiString name = OpenDialog1->FileName;
   tFileName->Caption = name;
   if (RetrieveFileInfo(name.c_str()))
      DisplayFileInfo();
   }
}
//---------------------------------------------------------------------------

void __fastcall TfMain::HelpAbout(TObject*)
{
  AboutBox().SetApplicationName( cProgramName )
            .Display();
}


void __fastcall TfMain::HelpOpenHelp(TObject*)
{
  ExecutableHelp().Display();    
}
//---------------------------------------------------------------------------

