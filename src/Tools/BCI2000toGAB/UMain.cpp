//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "UBCI2000DATA.h"
#include "UParameter.h"

#include "UMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CGAUGES"
#pragma resource "*.dfm"

TfMain *fMain;


//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
        : TForm(Owner)
{

}
//---------------------------------------------------------------------------

void __fastcall TfMain::CheckCalibrationFile( void )
{
        int i;
        int n_chans;
        PARAMLIST parmlist;

        if( parmlist.LoadParameterList( ParameterFile->Text.c_str() ) == false )
        {
                for(i=0;i<MAXCHANS;i++)
                {
                        offset[i]= 0.0;
                        gain[i]= 0.008;
                }
        }
        else
        {

                n_chans= parmlist.GetParamPtr("SourceChOffset")->GetNumValues();

                for(i=0;i<n_chans;i++)
                {
                        offset[i]=atoi(parmlist.GetParamPtr("SourceChOffset")->GetValue(i));
                        gain[i]=atof(parmlist.GetParamPtr("SourceChGain")->GetValue(i));
                }
        }
}

//-----------------------------------------------------------------------------

void __fastcall TfMain::bConvertClick(TObject *Sender)
{


 bci2000data=new BCI2000DATA;

 Continue->Enabled= false;

 ret=bci2000data->Initialize(eSourceFile->Text.c_str(), 50000);
 if (ret != BCI2000ERR_NOERR)
    {
    Application->MessageBox("Error opening input file", "Error", MB_OK);
    delete bci2000data;
    return;
    }

 fp=fopen(eDestinationFile->Text.c_str(), "wb");
 if (!fp)
    {
    Application->MessageBox("Error opening output file", "Error", MB_OK);
    delete bci2000data;
    return;
    }

    CheckCalibrationFile();

 firstrun=bci2000data->GetFirstRunNumber();
 lastrun=bci2000data->GetLastRunNumber();

 frun->Text= firstrun;
 lrun->Text= lastrun;

 Continue->Enabled= true;

 }

 
 void __fastcall TfMain::ContinueClick(TObject *Sender)
{

 firstrun= atoi( frun->Text.c_str() );
 lastrun= atoi( lrun->Text.c_str() );
 
 channels=bci2000data->GetNumChannels();
 dummy=1;
 samplingrate=bci2000data->GetSampleFrequency();

 // write the header
 fwrite(&channels, 2, 1, fp);
 fwrite(&dummy, 2, 1, fp);
 fwrite(&samplingrate, 2, 1, fp);

 // go through each run
 Gauge->MinValue=0;
 Gauge->Progress=0;
 Gauge->MaxValue=lastrun;
 for (cur_run=firstrun; cur_run<=lastrun; cur_run++)
  {
  bci2000data->SetRun(cur_run);
  numsamples=bci2000data->GetNumSamples();

  // go through all samples in each run
  for (sample=0; sample<numsamples; sample++)
   {
   if (sample % 1000 == 0) Application->ProcessMessages();
   // go through each channel
   for (channel=0; channel<channels; channel++)
    {
    cur_value=bci2000data->ReadValue(channel, sample);
    val= (float)cur_value;
    val= val * ( gain[channel] / 0.003 ) + offset[channel];
    cur_value= (__int16) (val);
    fwrite(&cur_value, 2, 1, fp);
    }
   // write the state element
   bci2000data->ReadStateVector(sample);
   cur_state=ConvertState(bci2000data);
   fwrite(&cur_state, 2, 1, fp);
   }
   // now, write one sample (all channels + state)
  // with a state value of 0 (i.e., Inter-Run-Interval)
  dummy=0;
  for (channel=0; channel<=channels; channel++)
     fwrite(&dummy, 2, 1, fp);
  Gauge->Progress=cur_run;
  }

 fclose(fp);

 Gauge->Progress=0;
 Application->MessageBox("Conversion process finished successfully", "Message", MB_OK);
}
//---------------------------------------------------------------------------


// convert one state of BCI2000 into the state of the old system
__int16 TfMain::ConvertState(BCI2000DATA *bci2000data)
{
STATEVECTOR *statevector;
__int16     old_state;
short       ITI, targetcode, resultcode, feedback, restperiod;
static short cur_targetcode=-1;

 statevector=bci2000data->GetStateVectorPtr();
 ITI=statevector->GetStateValue("InterTrialInterval");
 targetcode=statevector->GetStateValue("TargetCode");
 resultcode=statevector->GetStateValue("ResultCode");
 feedback=statevector->GetStateValue("Feedback");
 restperiod= statevector->GetStateValue("RestPeriod");
 if (targetcode > 0) cur_targetcode=targetcode;

 // ITI
 if (ITI == 1)
    return(20);

 // there is an outcome
 if (resultcode > 0)
    {
    if (resultcode == cur_targetcode)
       return(18);
    else
       return(19);
    }

 // target on the screen
 if (targetcode > 0)
    {
    if (feedback == 0)                  // no cursor -> pre-trial pause
       return(2+targetcode-1);
    else
       return(10+targetcode-1);         // cursor on the screen
    }
  if( restperiod == 1 )
        return( 24 );  

 // should actually never get here   
 return(1);
}


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
                ParameterFile->Text=OpenParameter->FileName;
}
//---------------------------------------------------------------------------

