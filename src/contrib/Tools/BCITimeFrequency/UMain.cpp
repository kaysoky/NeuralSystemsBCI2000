/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
/**************************************************************
        UMain is the main module of the BCITime application
***************************************************************/


//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <dir.h>

#include "BCI2000FileReader.h"
#include "ParamList.h"

#include "UMain.h"
#include "StateForm1.h"
#include "OutputForm1.h"
#include "InputForm1.h"
#include "ProcessForm1.h"
#include "VCLDefines.h"



//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CGAUGES"
#pragma link "cgauges"
#pragma resource "*.dfm"

TfMain *fMain;


//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
        : TForm(Owner),
        bciinput(NULL),
        bcioutput(NULL),
        bci2000data(NULL),
        mem(NULL)
{
}

//---------------------------------------------------------------------------
__fastcall TfMain::~TfMain( void )
{
        delete bciinput;
        delete bcioutput;
        delete bci2000data;
        delete mem;
}
//---------------------------------------------------------------------------        



void __fastcall TfMain::ConvertClick(TObject *Sender)
{
        if( Process() )
          Application->MessageBox(VCLSTR("Conversion process finished successfully"), VCLSTR("Message"), MB_OK);
}

bool TfMain::Process()
{
        FILE *pfile;
        char CurrentFile[256];
        int nfiles;
        int i,j;
        char filetype[16];
        float fstart,fend;
        int count;
        char line[32];
        int timeval;
        float start,bandwidth;
        FILE *lpfile;
        char l2[32];
        char l3[32];
        char l4[32];
        int cneigh;
        int current_signal;

        ParamList *cvplp;

        if ( ! bciinput    ) bciinput= new BCIInput;
        if ( ! bcioutput   ) bcioutput= new BCIOutput;
        if ( ! bci2000data ) bci2000data=new BCI2000FileReader;
        if ( ! mem ) mem= new MEM;

        bcioutput->memptr= mem;

        UseStateForm->SetVals();

//  going to get first path name from list !!!

      nfiles= FileList->Items->Count;

      if( nfiles > 0 )
        strcpy( CurrentFile, AnsiString(FileList->Items->Strings[0]).c_str() );
      else
        return false;

       
        ret= bci2000data->Open(CurrentFile, 50000).ErrorState();

        if (ret != BCI2000FileReader::NoError)
        {
                Application->MessageBox(VCLSTR("Error opening input file"), VCLSTR("Error"), MB_OK);
                delete bci2000data;
                bci2000data = NULL;
                return false;
        }

        bciinput->CheckCalibration( bci2000data, AnsiString(vCalibrationFile->Text).c_str(), FileType->Checked );

        channels=bci2000data->SignalProperties().Channels();
        samplingrate=bci2000data->SamplingRate();

		fstart= atof( AnsiString(OutputForm->vStart->Text).c_str() );
        fend=   atof( AnsiString(OutputForm->vEnd->Text).c_str() );

        bciinput->start= ( fstart * samplingrate ) / 1000.0;
        bciinput->end= ( fend * samplingrate ) / 1000.0;

        bciinput->nchans= InputForm->ChanList->Lines->Count;
        bciinput->Channels= channels;

        if( InputForm->CheckStateList->Checked == true )
        {
                bciinput->state_flag= 1;
                bciinput->nstates= InputForm->StateList->Lines->Count;
        }
        else
        {
                bciinput->nstates= 0;
        }

        for(i=0;i<bciinput->nchans;i++)
        {
                bciinput->chan_list[i]= atoi( AnsiString(InputForm->ChanList->Lines->Strings[i]).c_str() ) - 1;
        }

        for(i=0;i<bciinput->nstates;i++)
        {
                strcpy( bciinput->state_list[i], AnsiString(InputForm->StateList->Lines->Strings[i]).c_str()  );

        }

        bciinput->ntimes= OutputForm->Times->Lines->Count;
        bcioutput->ntimes= OutputForm->Times->Lines->Count;

        for(i=0;i<bciinput->ntimes;i++)
        {
                timeval= atoi( AnsiString(OutputForm->Times->Lines->Strings[i]).c_str() );

                if( ProcessForm->UseMEM->Checked )
                {
				   start= atof( AnsiString(ProcessForm->vStart->Text).c_str() );
                   bandwidth= atof( AnsiString(ProcessForm->vBandwidth->Text).c_str() );
                   bcioutput->value_list[i]= (int)( ( ( (float)timeval ) - ( (float)start ) + 0.5 ) / bandwidth ) ;
                }
                else
                {
                     bcioutput->value_list[i]= ( samplingrate * timeval ) / 1000;
                }

                bcioutput->time_list[i]= timeval;
        }

        bcioutput->Config( AnsiString(eDestinationFile->Text).c_str(), samplingrate, fstart,
                OutputForm->OutputOrder->ItemIndex,
                OutputForm->Statistics->ItemIndex );

        bciinput->Config( bcioutput );
        bciinput->outorder= OutputForm->OutputOrder->ItemIndex;

        bciinput->tfilterflag= InputForm->CheckTemporalFilter->Checked;
        if(  InputForm->CheckTemporalFilter->Checked == true)
        {
                if( (pfile= fopen( AnsiString(InputForm->vTemporalFile->Text).c_str(),"r") ) == NULL )
                {
                        Application->MessageBox(VCLSTR("Error"),VCLSTR("Opening Temporal Filter File"),MB_OK);
                }
                else
                {
                        count= 0;
                        while(fscanf(pfile,"%s",line) != EOF)
                        {
                                bciinput->tcoff[count]= atof( line );
                                if( count < MAXLTH ) count++;
                                else
                                    Application->MessageBox(VCLSTR("Error"),VCLSTR("Temporal Coefficients Exceed Limit"),MB_OK);
                        }
                        bciinput->tcount= count;
                        fclose( pfile );
                }

        }


        bciinput->BaselineUse= InputForm->Baseline->ItemIndex;
		bciinput->BaseStart= (int)(( atof( AnsiString(InputForm->vStartBase->Text).c_str() ) * samplingrate) / 1000.0);
        bciinput->BaseEnd= (int)((atof( AnsiString(InputForm->vEndBase->Text).c_str() )* samplingrate) / 1000.0);

        bciinput->sfilterflag= InputForm->CheckSpatialFilter->Checked;
        bciinput->alignflag= InputForm->CheckAlign->Checked;

        if( InputForm->CheckSpatialFilter->Checked == true )
        {
                if( (lpfile=fopen(AnsiString(InputForm->vSpatialFile->Text).c_str(),"r"))==NULL )
                {
                        Application->MessageBox(VCLSTR("Error"),VCLSTR("Opening Spatial Filter File"),MB_OK);
                        return false;
                }

                while( ( fscanf(lpfile,"%s",line) )!= EOF )
                {
                        if( strcmp(line,"Nchans=" ) == 0 )
                        {
                                fscanf(lpfile,"%s",line);
                                bciinput->signal_count= atoi( line );
                        }
                        else if( strcmp(line,"Chan[" ) == 0 )
                        {
                                fscanf(lpfile,"%s %s %s %s",line,l2,l3,l4);
                                current_signal= atoi( line ) -1;
                                bciinput->lap[current_signal][0]= atoi( l3 ) -1;         // added -1  5/4/05
                                bciinput->lapn[current_signal]= atoi( l4 ) ;              // removed  - 1  8/10/05
                        }
                        else if( strcmp(line,"Neigh[" ) == 0 )
                        {
                                fscanf(lpfile,"%s %s %s %s",line,l2,l3,l4);
                                cneigh= atoi( line );
                                bciinput->lap[current_signal][cneigh]= atoi( l3 ) -1;       // added -1  5/4/05
                                bciinput->d_lap[current_signal][cneigh]= atof( l4 );       // removed  -1  8/10/05
                        }

                }

                fclose(lpfile);
        }

		bcioutput->decimate=  atoi( AnsiString(OutputForm->vDecimate->Text).c_str() );

        bcioutput->memflag= ProcessForm->UseMEM->Checked;
        bciinput->memflag= ProcessForm->UseMEM->Checked;

        if( ProcessForm->UseMEM->Checked == true )
        {
				mem->setStart( atof( AnsiString(ProcessForm->vStart->Text).c_str() ) );
				mem->setStop( atof( AnsiString(ProcessForm->vEnd->Text).c_str() ) );
				mem->setDelta( atof( AnsiString(ProcessForm->vDensity->Text).c_str() ) );
				mem->setBandWidth( atof( AnsiString(ProcessForm->vBandwidth->Text).c_str() ) );
				mem->setModelOrder( atoi( AnsiString(ProcessForm->vModel->Text).c_str() ) );
                mem->setTrend( ProcessForm->Remove->ItemIndex );
                bcioutput->setWindow( ProcessForm->MemWinType->ItemIndex,
								 atoi( AnsiString(ProcessForm->vMemWindows->Text).c_str()),
								 atoi( AnsiString(ProcessForm->vMemBlockSize->Text).c_str() ),
                                 atoi( AnsiString(ProcessForm->vMemDataLength->Text).c_str() ),
                                 ProcessForm->cbSidelobeSuppression->ItemIndex );
        }

        // go through each run
        Gauge->MinValue=0;
        Gauge->Progress=0;
        Gauge->MaxValue=nfiles;

        bcioutput->ClearVals();

        for (cur_run=0; cur_run < nfiles; cur_run++)
        {
                strcpy(CurrentFile, AnsiString(FileList->Items->Strings[cur_run]).c_str() );

                if( bci2000data->Open(CurrentFile, 50000).ErrorState() == BCI2000FileReader::NoError )
                {
                        numsamples=bci2000data->NumSamples();

                        bciinput->ReadFile( bci2000data, numsamples );

                        Gauge->Progress=cur_run;
                }
        }

// print out anything left over !!!

        bcioutput->PrintVals( 1 );
        bcioutput->ClearVals();

        bcioutput->CloseFiles();

        Gauge->Progress=0;
        return true;
}
//---------------------------------------------------------------------------



void __fastcall TfMain::bOpenFileClick(TObject *Sender)
{
 if (OpenDialog->Execute())
    eSourceFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::Button1Click(TObject *Sender)
{
        if (SaveDialog->Execute())
                eDestinationFile->Text=SaveDialog->FileName;
}
//---------------------------------------------------------------------------


void __fastcall TfMain::Button2Click(TObject *Sender)
{
        if (OpenParameter->Execute())
                vCalibrationFile->Text=OpenParameter->FileName;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::AddFileClick(TObject *Sender)
{
        FileList->Items->Add( eSourceFile->Text );
}
//---------------------------------------------------------------------------


void __fastcall TfMain::AddDirectoryClick(TObject *Sender)
{
        char filename[256];
        int pos,idx;
        char prefix[256];
        char cur_run[256];
        char path[256];

        struct ffblk ffblk;
        int done;

		strcpy( filename, AnsiString(eSourceFile->Text).c_str() );

		// if the last 4 characters in the filename are not ".dat", then the file name
		// does not follow the BCI2000 filename conventions
		idx=strlen(filename)-4;
		if (idx < 0) return;
        pos=stricmp(&(filename[idx]), ".DAT");
        if (pos != 0) return;

        // get the position of the first character of the run number
        idx-=2;
        if (idx < 0) return;

        strncpy(prefix, filename, idx);
        prefix[idx]=0;

        // get the directory name

        while( prefix[idx] != 0x5c )
                idx--;
        strncpy(path, filename, idx);        
        path[idx]= 0x5c;
        path[idx+1]= 0;

        done= findfirst("*.dat",&ffblk,0);

        while(!done)
        {
                strcpy(cur_run,path);
                strcat(cur_run, ffblk.ff_name );

                FileList->Items->Add( cur_run );
                done= findnext(&ffblk);
        }

}
//---------------------------------------------------------------------------

void __fastcall TfMain::ClearClick(TObject *Sender)
{
        FileList->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::StateControlClick(TObject *Sender)
{
        UseStateForm->Show();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::Button3Click(TObject *Sender)
{
        Close();        
}
//---------------------------------------------------------------------------

void __fastcall TfMain::OutputControlClick(TObject *Sender)
{
        OutputForm->Show();

}
//---------------------------------------------------------------------------


void __fastcall TfMain::InputControlClick(TObject *Sender)
{
        InputForm->Show();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::ProcessControlClick(TObject *Sender)
{
        ProcessForm->Show();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::Button5Click(TObject *Sender)
{
        FILE *saveIO;
      //  pario= new ParIO;

        ParIO pario;

        SaveParameterFile->Execute();
        vParmFile->Text= SaveParameterFile->FileName;

        if( ( saveIO= fopen( AnsiString(vParmFile->Text).c_str(), "w+" ) ) == NULL )
        {
                Application->MessageBox(VCLSTR("Error"),VCLSTR("Opening Parameter File "),MB_OK);
                return;
        }

        fprintf(saveIO,"BCITime Parameters \n");
        pario.SaveF( saveIO, UseStateForm , InputForm, ProcessForm , OutputForm);

        fclose( saveIO );
   //     delete pario;
   //     pario= NULL;

}
//---------------------------------------------------------------------------

void __fastcall TfMain::Button4Click(TObject *Sender)
{
        if( OpenParameterFile->Execute() )
        {
          vParmFile->Text= OpenParameterFile->FileName;
          ApplyParamFile( AnsiString(vParmFile->Text).c_str() );
        }
}

//---------------------------------------------------------------------------
void TfMain::ApplyParamFile( const char* inParamFile )
{
        FILE *getIO;
        ParIO pario;

        if( ( getIO= fopen( inParamFile, "r" )) == NULL )
        {
                Application->MessageBox(VCLSTR("Error"),VCLSTR("Opening Parameter File "),MB_OK);
                return;
        }

        pario.GetF( getIO, UseStateForm, InputForm, ProcessForm, OutputForm );

        fclose( getIO );
      //  delete pario;
      //  pario= NULL;
}
//---------------------------------------------------------------------------

// This is called from WinMain() to make sure that all forms are instantiated
// before a parameter file is loaded.
void TfMain::ProcessCommandLineOptions()
{
  AnsiString programName = ExtractFileName( ChangeFileExt( ParamStr( 0 ), "" ) );
  const char* messageMode = "error";
  bool batchMode = false;

  for( int i = 1; i <= ParamCount(); ++i )
  {
    char* optionBuffer = new char[ ParamStr( i ).Length() + 1 ],
        * option = optionBuffer;
    ::strcpy( optionBuffer, AnsiString(ParamStr( i )).c_str() );
    switch( *option )
    {
      case '\0':
        break;

      case '-':
        switch( *++option )
        {
          case 'b':
          case 'B':
            batchMode = true;
            break;

          case 'p':
          case 'P':
            vParmFile->Text = ++option;
            ApplyParamFile( option );
            break;

          case 'o':
          case 'O':
            eDestinationFile->Text = ++option;
            break;

          case 'c':
          case 'C':
            vCalibrationFile->Text = ++option;
            FileType->Checked = ( *option != '\0' );
            break;

          case 'h':
          case 'H':
          case '?':
            messageMode = "help";
            /* no break */
          default:
			Application->MessageBox( VCLSTR(
              "The following parameters are accepted:\n\n"
              " -h\t                 \tshow this help\n"
              " -b\t                 \tprocess and quit (batch mode)\n"
              " -o<output file>      \toutput file name\n"
              " -p<parameter file>   \tload named parameter file at startup\n"
              " -c<calibration file> \tcalibration file name\n"
			  " <file1> <file2> ...  \tany number of input file names\n"
			  ),
			  VCLSTR( ( programName + " command line " + messageMode ).c_str() ),
              MB_OK | MB_ICONINFORMATION
			);
            Application->Terminate();
        }
        break;

      default:
        eSourceFile->Text = option;
        FileList->Items->Add( option );
    }
    delete[] optionBuffer;
  }
  if( batchMode )
  {
    Process();
    Application->Terminate();
  }
}
