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

#define VERSION "0.4"

using namespace std;

TfMain *fMain;

template<typename From, typename To> float convert_num( const From& f, To& t )
{
  bool overflow = false;
  if( numeric_limits<To>::max() < f )
  {
    overflow = true;
    t = numeric_limits<To>::max();
  }
  else if( numeric_limits<To>::is_integer && f < numeric_limits<To>::min() )
  {
    overflow = true;
    t = numeric_limits<To>::min();
  }
  else if( !numeric_limits<To>::is_integer && f < -numeric_limits<To>::max() )
  {
    overflow = true;
    t = -numeric_limits<To>::max();
  }
  else
    t = To( f );
  float ratio = 0.0;
  if( overflow )
    ratio = float( f ) / ( float( t ) + numeric_limits<float>::epsilon() );
  return ratio;
}

void TfMain::UserMessage( const AnsiString& msg, TfMain::msgtypes type ) const
{
  bool actuallyShowIt = !autoMode;
  const char* msgtype = "Message";
  long msgflags = MB_OK;
  switch( type )
  {
    case Warning:
      msgflags |= MB_ICONWARNING;
      msgtype = "Warning";
      break;
    case Error:
      msgflags |= MB_ICONERROR;
      msgtype = "Error";
      actuallyShowIt = true;
      break;
  }
  if( actuallyShowIt )
    Application->MessageBox( msg.c_str(), msgtype, msgflags );
}

//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
        : TForm(Owner),
          gabTargets( NULL ),
          autoMode( false ),
          bci2000data( NULL )
{
  Caption = Caption + " " VERSION " ("__DATE__")";
  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;
  mSourceFiles->WordWrap = false;
  
  if( Application->OnIdle == NULL )
    Application->OnIdle = DoStartupProcessing;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::DoStartupProcessing( TObject*, bool& )
{
  Application->OnIdle = NULL;

  const char optionChar = '-';
  if( ParamCount() > 0 && ParamStr( 1 )[ 1 ] != optionChar )
  {
    autoMode = true;
    mSourceFiles->Lines->Clear();
    int i = 1;
    while( i <= ParamCount() && ParamStr( i )[ 1 ] != optionChar )
      mSourceFiles->Lines->Append( ParamStr( i++ ) );
    while( i <= ParamCount() && ParamStr( i )[ 1 ] == optionChar )
    {
      if( ParamStr( i ).Length() >= 2 )
      {
        switch( ParamStr( i )[ 2 ] )
        {
          case 'o':
          case 'O':
            if( ParamCount() > i && ParamStr( i + 1 )[ 1 ] != optionChar )
              eDestinationFile->Text = ParamStr( ++i );
            else
              eDestinationFile->Text = ParamStr( i ).c_str() + 2;
            break;
          case 'p':
          case 'P':
            ParameterFile->Enabled = true;
            if( ParamCount() > i && ParamStr( i + 1 )[ 1 ] != optionChar )
             ParameterFile->Text = ParamStr( ++i );
            else
              ParameterFile->Text = ParamStr( i ).c_str() + 2;
            break;
        }
      }
      ++i;
    }
    if( bConvert->Enabled )
      bConvert->Click();
    Application->Terminate();
  }
}

void __fastcall TfMain::CheckCalibrationFile( void )
{
  int i;
  int n_chans;
  PARAMLIST paramlist;

  if( ParameterFile->Enabled && ParameterFile->Text.Length() > 0 )
  {
    if( paramlist.LoadParameterList( ParameterFile->Text.c_str() ) )
    {
      n_chans= paramlist.GetParamPtr("SourceChOffset")->GetNumValues();
      offset.clear();
      offset.resize( n_chans, 0 );
      gain.clear();
      gain.resize( n_chans, 0 );
      for(i=0;i<n_chans;i++)
      {
        offset[i]=atoi(paramlist.GetParamPtr("SourceChOffset")->GetValue(i));
        gain[i]=atof(paramlist.GetParamPtr("SourceChGain")->GetValue(i));
      }
    }
    else
      UserMessage( "Could not open parameter file \""
                   + ParameterFile->Text + "\".", Error );
  }

  static const short ud[] = { 11, 15 },
                     lr[] = { 13, 17 },
                     udlr[] = { 10, 12, 14, 16 };
  const PARAM* param = paramlist.GetParamPtr( "NumberTargets" );
  if( param == NULL )
    param = bci2000data->GetParamListPtr()->GetParamPtr( "NumberTargets" );
  int numberTargets = ( param ? atoi( param->GetValue() ) : 0 );

  param = paramlist.GetParamPtr( "TargetOrientation" );
  if( param == NULL )
    param = bci2000data->GetParamListPtr()->GetParamPtr( "TargetOrientation" );
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
    UserMessage( "Target type heuristics failed. "
                 "Target conversion is likely to be unusable.",
                 Warning );
    gabTargets = NULL;
  }
}

//-----------------------------------------------------------------------------

void __fastcall TfMain::bConvertClick(TObject *Sender)
{
 offset.clear();
 gain.clear();

 bool errorOccurred = false;
 for( int i = 0; i < mSourceFiles->Lines->Count; ++i )
 {
   BCI2000DATA reader;
   if( reader.Initialize( mSourceFiles->Lines->Strings[ i ].c_str(), 50000 )
        != BCI2000ERR_NOERR )
   {
      errorOccurred = true;
      UserMessage( "Error opening input file \""
                   + mSourceFiles->Lines->Strings[ i ] + "\"",
                   Error );
   }
 }
 if( errorOccurred )
   return;

 delete bci2000data;
 bci2000data=new BCI2000DATA;
 ret=bci2000data->Initialize(mSourceFiles->Lines->Strings[ 0 ].c_str(), 50000);
 if (ret != BCI2000ERR_NOERR)
    {
    UserMessage( "Error opening input file", Error );
    delete bci2000data;
    return;
    }

 fp=fopen(eDestinationFile->Text.c_str(), "wb");
 if (!fp)
    {
    UserMessage( "Error opening output file", Error );
    delete bci2000data;
    return;
    }

 CheckCalibrationFile();

 firstrun = 1;
 lastrun = mSourceFiles->Lines->Count;

 channels=bci2000data->GetNumChannels();
 dummy=1;
 samplingrate=bci2000data->GetSampleFrequency();

 float maxRatio = 0.0;

 // write the header
 fwrite(&channels, 2, 1, fp);
 fwrite(&dummy, 2, 1, fp);
 fwrite(&samplingrate, 2, 1, fp);

 // go through each run
 Gauge->MinValue=0;
 Gauge->Progress=0;
 Gauge->MaxValue=Gauge->Width;
 for (cur_run=firstrun; cur_run<=lastrun; cur_run++)
  {
  delete bci2000data;
  bci2000data = new BCI2000DATA;
  bci2000data->Initialize( mSourceFiles->Lines->Strings[ cur_run - 1 ].c_str(), 50000 );
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
       float ratio = convert_num( val, cur_value );
       if( maxRatio < ratio )
         maxRatio = ratio;
       fwrite(&cur_value, 2, 1, fp);
     }
   }
   else
     for( channel = 0; channel < channels; ++channel )
     {
       val = bci2000data->Value( channel, sample ) / 0.003;
       float ratio = convert_num( val, cur_value );
       if( maxRatio < ratio )
         maxRatio = ratio;
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
  Gauge->Progress=( cur_run * Gauge->MaxValue ) / lastrun;
  }

 fclose(fp);

 if( maxRatio > 1.0 )
   UserMessage( "Numeric overflow occurred during conversion.\n"
                "Input data exceeded the numeric range by a ratio of "
                + FloatToStr( maxRatio ) + ".\n"
                "Conversion process finished.",
                Warning );
 else
   UserMessage( "Conversion process finished successfully",
                Message );
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
 OpenDialog->Options << ofAllowMultiSelect;
 if( OpenDialog->Execute() )
    mSourceFiles->Lines = OpenDialog->Files;
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

