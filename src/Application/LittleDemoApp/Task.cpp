#include "PCHIncludes.h"
#pragma hdrstop

#include "Task.h"
#include "UState.h"
#include "UBCITime.h"

#include <assert>

RegisterFilter( TTask, 3 );

TTask::TTask()
: vis( NULL ),
  AcousticMode( 0 ),
  form( NULL ),
  progressbar( NULL ),
  chart( NULL ),
  series( NULL )
{
 BEGIN_PARAMETER_DEFINITIONS
   "NeuralMusic int AcousticMode= 1 "
     "0 0 2 // Achin's Acoustic Mode :-) 0: no sound, 1: MIDI, 2: WAV (enumeration)",
   "NeuralMusic matrix Sounds= "
     "{ MIDI WAV } " // row labels

     "{     ultra%20low     low              medium            high              ultra%20high } " // column labels
/* MIDI */ "45              52               59                66                73 "
/* WAV  */ "sounds\\pig.wav sounds\\frog.wav sounds\\train.wav sounds\\uh-uh.wav sounds\\whistle.wav "

     " // sounds to be played for different ranges of the feedback signal",
   // has to be in there
   "NeuralMusic int NumberTargets= 10 "
     "0 0 10 // not used",
 END_PARAMETER_DEFINITIONS

 BEGIN_STATE_DEFINITIONS
   "Pitch 8 0 0 0",
   "StimulusTime 16 17528 0 0",
   "TargetCode 8 0 0 0",
 END_STATE_DEFINITIONS

 form=new TForm( ( TComponent* )NULL );
 form->Caption="Neural Music Test";
 form->Height=300;

 progressbar=new TProgressBar(form);
 progressbar->Parent=form;
 progressbar->Visible=true;
 progressbar->Enabled=true;
 progressbar->Position=0;
 progressbar->Min=0;
 progressbar->Max=255;
 progressbar->Top=10;
 progressbar->Left=10;
 progressbar->Width=200;
 progressbar->Height=20;

 chart=new TChart(form);
 chart->Visible=false;
 chart->Parent=form;
 chart->Left=0;
 chart->Top=40;
 chart->Width=form->ClientWidth;
 chart->Height=form->ClientHeight-40;
 chart->Title->Visible=false;
 chart->Legend->Visible=false;
 chart->AllowPanning=pmVertical;
 chart->AllowZoom=false;
 chart->View3D=false;
 chart->Anchors << akLeft << akTop << akRight << akBottom;
 chart->Visible=true;

 series=new TLineSeries(chart);
 series->ParentChart=chart;
}

//-----------------------------------------------------------------------------

TTask::~TTask( void )
{
 delete vis;
 delete form;
}

void TTask::Preflight( const SignalProperties& inputProperties,
                             SignalProperties& outputProperties ) const
{
  switch( ( int )Parameter( "AcousticMode" ) )
  {
    case 0: // none
      break;
    case 1: // Midi
      // We believe our MIDI player handles any input.
      break;
    case 2: // WAV
      {
        std::string applicationPath = ::ExtractFilePath( ::ParamStr( 0 ) ).c_str();
        bool genError = false;
        TWavePlayer testPlayer;
        for( size_t i = 0; !genError && i < Parameter( "Sounds" )->GetNumValuesDimension2(); ++i )
        {
          std::string fileName = applicationPath + ( const char* )Parameter( "Sounds", "WAV", i );
          TWavePlayer::Error err = testPlayer.AttachFile( fileName.c_str() );
          if( err == TWavePlayer::fileOpeningError )
              bcierr << "Could not open \""
                     << fileName
                     << "\" as a wave sound file"
                     << std::endl;
          else if( err != TWavePlayer::noError )
          {
              bcierr << "Some general error prevents wave audio playback"
                     << std::endl;
              genError = true;
          }
        }
      }
      break;
    default:
      PreflightCondition( false );
  }
  PreflightCondition( inputProperties >= SignalProperties( 1, 1, SignalType::int16 ) );
  outputProperties = SignalProperties( 0, 0 );
}

void TTask::Initialize()
{
 AcousticMode = Parameter( "AcousticMode" );
 switch( AcousticMode )
 {
   case 0: // none
     break;
   case 1: // MIDI
     break;
   case 2: // WAV
     {
      std::string applicationPath = ::ExtractFilePath( ::ParamStr( 0 ) ).c_str();
      for( size_t i = 0; i < Parameter( "Sounds" )->GetNumValuesDimension2(); ++i )
      {
        std::string fileName = applicationPath + ( const char* )Parameter( "Sounds", "WAV", i );
        wavePlayers[ Parameter( "Sounds" )->LabelsDimension2()[ i ] ].AttachFile( fileName.c_str() );
      }
     }
     break;
   default:
     assert( false );
 }

 delete vis;
 vis= new GenericVisualization(SOURCEID::TASKLOG);
 vis->SendCfg2Operator(SOURCEID::TASKLOG, CFGID::WINDOWTITLE, "User Task Log");

 form->Show();
}


int TTask::MakeMusic(short controlsignal)
{
char    memotext[256];
int     pitch;

 // pitch=(controlsignal+32767)/256;
 pitch=abs(controlsignal % 256);

 // set the position on the progress bar according to the current control signal
 progressbar->Position=pitch;
 // add the current control signal value to the chart
 series->AddY((double)controlsignal, "", (TColor)clTeeColor);

 // send the current pitch to the operator as well
 sprintf(memotext, "Current Pitch: %d\r", pitch);
 vis->SendMemo2Operator(memotext);

 const char* pitchLabel = "";

 if( pitch < 51 )
   pitchLabel = "ultra low";
 else if( pitch < 102 )
   pitchLabel = "low";
 else if( pitch < 153 )
   pitchLabel = "medium";
 else if( pitch < 204 )
   pitchLabel = "high";
 else
   pitchLabel = "ultra high";

 switch( AcousticMode )
 {
   case 0: // none
     break;
   case 1: // MIDI
     midiPlayer.Play( Parameter( "Sounds", "MIDI", pitchLabel ) );
     break;
   case 2: // WAV
     if( !wavePlayers[ pitchLabel ].IsPlaying() )
       wavePlayers[ pitchLabel ].Play();
     break;
   default:
     assert( false );
 }

 return(pitch);
}


void TTask::Process( const GenericSignal* Input, GenericSignal* )
{
  const GenericSignal& InputSignal = *Input;
  int cur_pitch, cur_running;

  cur_pitch = State( "Pitch" );

  cur_running = State( "Running" );
  if (cur_running > 0)
    {
    // make music
    cur_pitch=MakeMusic( InputSignal( 0, 0 ) );
    }

  State( "Pitch" ) = cur_pitch;
  // time stamp the data
  State( "StimulusTime" ) = BCITIME::GetBCItime_ms();
}

void TTask::Halt()
{
  midiPlayer.StopSequence();
  for( WavePlayerContainer::iterator i = wavePlayers.begin(); i != wavePlayers.end(); ++i )
    i->second.Stop();
}
