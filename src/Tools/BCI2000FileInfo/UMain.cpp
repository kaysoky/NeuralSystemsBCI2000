//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UBCI2000Data.h"
#include "UOperatorCfg.h"
#include "UMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfMain *fMain;
//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
        : TForm(Owner)
{
 bci2000data=NULL;
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


void TfMain::ProcessFiles( TStrSet& inFilesToProcess, bool scanOnly )
{
    Application->BringToFront();
    /* DisableAll();
    StatusBar->Panels->Items[ 0 ]->Text = "Processing";
    Refresh(); */

    if (inFilesToProcess.size() > 1)
       Application->MessageBox("You can only drop one file at a time", "Warning", MB_OK);
    else
       {
       TStrSet::iterator i = inFilesToProcess.begin();
       // StatusBar->Panels->Items[ 1 ]->Text = AnsiString(i->c_str());
       tFileName->Caption = AnsiString(i->c_str());
       if (RetrieveFileInfo(AnsiString(i->c_str())))
          DisplayFileInfo();
       }

    /* EnableAll();
    StatusBar->Panels->Items[ 0 ]->Text = "Idle";
    StatusBar->Panels->Items[ 1 ]->Text = "";
    Refresh(); */
}


bool TfMain::RetrieveFileInfo(AnsiString filename)
{
 if (bci2000data) delete bci2000data;
 bci2000data=new BCI2000DATA();
 int ret=bci2000data->Initialize(filename.c_str(), 50000);
 if (ret != BCI2000ERR_NOERR)
    {
    switch (ret)
     {
     case BCI2000ERR_FILENOTFOUND:
          Application->MessageBox("File not found", "Error", MB_OK);
          break;
     case BCI2000ERR_MALFORMEDHEADER:
          Application->MessageBox("Not a BCI2000 data file", "Error", MB_OK);
          break;
     case BCI2000ERR_NOBUFMEM:
          Application->MessageBox("Internal Error: Could not allocate buffer memory", "Error", MB_OK);
          break;
     default:
          Application->MessageBox("Unknown Error!", "Error", MB_OK);
     }
    delete bci2000data;
    bci2000data=NULL;
    bShowParams->Enabled=false;
    return(false);
    }
 else
  preferences.GetDefaultSettings();

 bShowParams->Enabled=true;
 return(true);
}

void __fastcall TfMain::bShowParamsClick(TObject *Sender)
{
 assert(bci2000data);
 fConfig->Initialize((PARAMLIST *)bci2000data->GetParamListPtr(), &preferences);
 fConfig->Show();
}
//---------------------------------------------------------------------------


void __fastcall TfMain::FormShow(TObject *Sender)
{
 DropPanel->Hint = "Drop BCI2000 Files here";
 DropPanel->ShowHint = true;
 defaultDropPanelWindowProc = DropPanel->WindowProc;
 DropPanel->WindowProc = DropPanelWindowProc;
 ::DragAcceptFiles( DropPanel->Handle, true );
 ::PostMessage( DropPanel->Handle, WM_USER, 0, 0 );
}
//---------------------------------------------------------------------------

void __fastcall TfMain::FormClose(TObject *Sender, TCloseAction &Action)
{
 DropPanel->WindowProc = defaultDropPanelWindowProc;
}
//---------------------------------------------------------------------------




void __fastcall TfMain::Quit1Click(TObject *Sender)
{
 Close();
}
//---------------------------------------------------------------------------

void TfMain::DisplayFileInfo()
{
 assert(bci2000data);

 const PARAMLIST *paramlist=bci2000data->GetParamListPtr();
 assert(paramlist->GetParamPtr("SampleBlockSize"));
 assert(paramlist->GetParamPtr("SamplingRate"));

 AnsiString prmval_blocksize=paramlist->GetParamPtr("SampleBlockSize")->GetValue();
 AnsiString prmval_samplingrate=paramlist->GetParamPtr("SamplingRate")->GetValue();
 int blocksize=atoi(prmval_blocksize.c_str());
 float samplingrate=atof(prmval_samplingrate.c_str());
 assert(samplingrate > 0 && blocksize > 0);

 float updaterate=samplingrate/(float)blocksize;

 tSampleBlockSize->Caption=prmval_blocksize;
 tSamplingRate->Caption=prmval_samplingrate+" Hz";
 tUpdateRate->Caption=AnsiString::FormatFloat("#.##", updaterate)+" Hz";
}


void __fastcall TfMain::Open1Click(TObject *Sender)
{
if (OpenDialog1->Execute())
   {
   tFileName->Caption = OpenDialog1->FileName;
   if (RetrieveFileInfo(OpenDialog1->FileName))
      DisplayFileInfo();
   }
}
//---------------------------------------------------------------------------





