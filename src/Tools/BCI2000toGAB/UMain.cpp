//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <vector>
#include <limits>

#include "UBCI2000DATA.h"
#include "UParameter.h"

#include "UMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CGAUGES"
#pragma resource "*.dfm"

using namespace std;

template<typename From, typename To> bool convert_num( const From& f, To& t )
{
  bool overflow = false;
  if( numeric_limits<To>::is_integer ) // asymmetry in numeric_limits::min()
    overflow = ( numeric_limits<To>::max() < f ) || ( f < numeric_limits<To>::min() );
  else
    overflow = ( numeric_limits<To>::max() < f ) || ( f < -numeric_limits<To>::max() );
  t = To( f );
  return overflow;
}

TfMain *fMain;


//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
        : TForm(Owner),
          gabTargets( NULL ),
          autoMode( false )
{
  if( Application->OnIdle == NULL )
    Application->OnIdle = DoStartupProcessing;
  Caption = Caption + "("__DATE__")";
}
//---------------------------------------------------------------------------

void __fastcall TfMain::DoStartupProcessing( TObject*, bool& )
{
  Application->OnIdle = NULL;
  if( ParamCount() > 0 )
  {
    autoMode = true;
    eSourceFile->Text = ParamStr( 1 );
    if( ParamCount() > 1 )
    {
      eDestinationFile->Text = ParamStr( 2 );
      if( ParamCount() > 2 )
      {
        ParameterFile->Enabled = true;
        ParameterFile->Text = ParamStr( 3 );
      }
    }
    if( bConvert->Enabled )
      bConvert->Click();
    if( Continue->Enabled )
      Continue->Click();
    Application->Terminate();
  }
}

void __fastcall TfMain::CheckCalibrationFile( void )
{
  int i;
  int n_chans;
  PARAMLIST parmlist;
  const PARAMLIST* paramlistPtr = &parmlist;

  if( ParameterFile->Enabled && ParameterFile->Text.Length() > 0 )
  {
    if( parmlist.LoadParameterList( ParameterFile->Text.c_str() ) )
    {
      n_chans= parmlist.GetParamPtr("SourceChOffset")->GetNumValues();
      offset.clear();
      offset.resize( n_chans, 0 );
      gain.clear();
      gain.resize( n_chans, 0 );
      for(i=0;i<n_chans;i++)
      {
        offset[i]=atoi(parmlist.GetParamPtr("SourceChOffset")->GetValue(i));
        gain[i]=atof(parmlist.GetParamPtr("SourceChGain")->GetValue(i));
      }
    }
    else
      Application->MessageBox( ( AnsiString( "Could not open calibration file \"" )
                                    + ParameterFile->Text + "\"." ).c_str(),
                                "Error", MB_OK | MB_ICONERROR );
  }
  else
    paramlistPtr = bci2000data->GetParamListPtr();
    
  static const short ud[] = { 11, 15 },
                     lr[] = { 13, 17 },
                     udlr[] = { 10, 12, 14, 16 };
  const PARAM* param = paramlistPtr->GetParamPtr( "NumberTargets" );
  int numberTargets = ( param ? atoi( param->GetValue() ) : 0 );
  param = paramlistPtr->GetParamPtr( "TargetOrientation" );
  int targetOrientation = ( param ? atoi( param->GetValue() ) : 0 ),
      numberTargetsMax = 0;
  switch( targetOrientation )
  {
    case 0:
    case 3: // Both
      gabTargets = udlr;
      numberTargetsMax = sizeof( udlr ) / sizeof( *udlr );
      break;
    case 1: // Vertical
      gabTargets = ud;
      numberTargetsMax = sizeof( ud ) / sizeof( *ud );
      break;
    case 2: // Horizontal
      gabTargets = lr;
      numberTargetsMax = sizeof( lr ) / sizeof( *lr );
      break;
  }
  if( numberTargets > numberTargetsMax )
  {
    Application->MessageBox(
                 "Target type heuristics failed. "
                 "Target conversion is likely to be unusable.",
                 "Warning", MB_OK | MB_ICONWARNING );
    gabTargets = NULL;
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
#if 0
 if( channels > 64 )
   channels = 64;
#endif
 dummy=1;
 samplingrate=bci2000data->GetSampleFrequency();

 bool overflowOccurred = false;
 
 // write the header
 fwrite(&channels, 2, 1, fp);
 fwrite(&dummy, 2, 1, fp);
 fwrite(&samplingrate, 2, 1, fp);

 // go through each run
 Gauge->MinValue=0;
 Gauge->Progress=0;
 Gauge->MaxValue=Gauge->Width;// lastrun;
 for (cur_run=firstrun; cur_run<=lastrun; cur_run++)
  {
  bci2000data->SetRun(cur_run);
  numsamples=bci2000data->GetNumSamples();

  // go through all samples in each run
  for (sample=0; sample<numsamples; sample++)
   {
   if (sample % 1000 == 0)
   {
     Gauge->Progress = ( ( cur_run - 1 ) * Gauge->MaxValue ) / lastrun
                      + ( sample * Gauge->MaxValue ) / ( numsamples * lastrun );
     Application->ProcessMessages();
   }
   // go through each channel
   if( CalibFromFile() )
   {
     for (channel=0; channel<channels; channel++)
     {
       cur_value=bci2000data->ReadValue(channel, sample);
       val= (float)cur_value;
       val= gain[channel] * ( val - offset[channel] ) / 0.003;
       overflowOccurred |= convert_num( val, cur_value );
       fwrite(&cur_value, 2, 1, fp);
     }
   }
   else
     for( channel = 0; channel < channels; ++channel )
     {
       val = bci2000data->Value( channel, sample ) / 0.003;
       overflowOccurred |= convert_num( val, cur_value );
       fwrite( &cur_value, sizeof( cur_value ), 1, fp );
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

 if( overflowOccurred )
   Application->MessageBox(
               "Numeric overflow occurred during conversion.\n"
               "Conversion process finished.",
               "Warning", MB_OK | MB_ICONWARNING );
 else if( !autoMode )
   Application->MessageBox(
               "Conversion process finished successfully",
               "Message", MB_OK | MB_ICONASTERISK );
 Gauge->Progress=0;
}
//---------------------------------------------------------------------------


// convert one state of BCI2000 into the state of the old system
__int16 TfMain::ConvertState(BCI2000DATA *bci2000data)
{
const STATEVECTOR *statevector;
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
 if (targetcode > 0 )
 {
   if( gabTargets != NULL )
     return gabTargets[ targetcode - 1 ];
   else
   {
      if (feedback == 0)                  // no cursor -> pre-trial pause
         return(2+targetcode-1);
      else
         return(10+targetcode-1);         // cursor on the screen
   }
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
        {
          ParameterFile->Enabled = true;
          ParameterFile->Text=OpenParameter->FileName;
        }
}
//---------------------------------------------------------------------------

