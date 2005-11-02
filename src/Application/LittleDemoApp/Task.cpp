#include "PCHIncludes.h"
#pragma hdrstop

#include "Task.h"

#include <vcl.h>
#include <ComCtrls.hpp>

#include "UState.h"
#include "UBCITime.h"

using namespace std;

RegisterFilter( TTask, 3 );

TTask::TTask()
: mAcousticMode( amNone ),
  mpForm( NULL ),
  mpProgressbar( NULL ),
  mpChart( NULL ),
  mpSeries( NULL ),
  mVis( SOURCEID::TASKLOG )
{
  BEGIN_PARAMETER_DEFINITIONS
    "NeuralMusic int WinXpos= 400 0 0 0 "
      "// Window X location",
    "NeuralMusic int WinYpos= 5 0 0 0 "
      "// Window Y location",
    "NeuralMusic int WinWidth= 512 0 0 0 "
      "// Window Width",
    "NeuralMusic int WinHeight= 300 0 0 0 "
      "// Window Height",

    "NeuralMusic matrix NestedMatrices= 2 3 "
      " singleValue11        { matrix 2 2 11 12 21 22 } "
      " { matrix 1 2 11 12 } singleValue22 "
      " singleValue31        singleValue32 "
      "// A nested matrix example",

    "NeuralMusic int AcousticMode= 1 "
      "0 0 2 // Achin's Acoustic Mode :-) 0: no sound, 1: MIDI, 2: WAV (enumeration)",

    "NeuralMusic matrix Sounds= "
      "{ MIDI WAV } " // row labels
      "{     ultra%20low     low              medium            high              ultra%20high } " // column labels
/* MIDI */  "45              52               59                66                73 "
/* WAV  */  "sounds\\pig.wav sounds\\frog.wav sounds\\train.wav sounds\\uh-uh.wav sounds\\whistle.wav "

      " // sounds to be played for different ranges of the feedback signal",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "Pitch 8 0 0 0",
    "StimulusTime 16 17528 0 0",
    "TargetCode 8 0 0 0",
    "ResultCode 8 0 0 0",
    "Feedback 1 0 0 0",
    "IntertrialInterval 1 0 0 0",
  END_STATE_DEFINITIONS

  mpForm = new TForm( ( TComponent* )NULL );
  mpForm->Caption = "Neural Music Test";

  mpProgressbar = new TProgressBar( mpForm ); // Let the form delete the progressbar on its own destruction.
  mpProgressbar->Parent = mpForm;
  mpProgressbar->Visible = true;
  mpProgressbar->Enabled = true;
  mpProgressbar->Position = 0;
  mpProgressbar->Min = 0;
  mpProgressbar->Max = 255;
  mpProgressbar->Top = 10;
  mpProgressbar->Left = 10;
  mpProgressbar->Width = 200;
  mpProgressbar->Height = 20;
  mpProgressbar->Anchors << akLeft << akTop << akRight << akBottom;

  mpChart = new TChart( mpForm ); // Let the form delete the chart on its own destruction.
  mpChart->Visible = false;
  mpChart->Parent = mpForm;
  mpChart->Left = 0;
  mpChart->Top = 40;
  mpChart->Width = mpForm->ClientWidth;
  mpChart->Height = mpForm->ClientHeight-40;
  mpChart->Title->Visible = false;
  mpChart->Legend->Visible = false;
  mpChart->AllowPanning = pmVertical;
  mpChart->AllowZoom = false;
  mpChart->View3D = false;
  mpChart->Anchors << akLeft << akTop << akRight << akBottom;
  mpChart->Visible = true;

  mpSeries = new TLineSeries( mpChart ); // Let the chart delete the series on its own destruction.
  mpSeries->ParentChart = mpChart;
}


TTask::~TTask( void )
{
  delete mpForm;
}


void
TTask::Preflight( const SignalProperties& inputProperties,
                        SignalProperties& outputProperties ) const
{
  if( Parameter( "AcousticMode" ) == amWave )
  {
    string applicationPath = ::ExtractFilePath( ::ParamStr( 0 ) ).c_str();
    bool genError = false;
    TWavePlayer testPlayer;
    for( size_t i = 0; !genError && i < Parameter( "Sounds" )->GetNumColumns(); ++i )
    {
      string fileName = applicationPath + string( Parameter( "Sounds", "WAV", i ) );
      TWavePlayer::Error err = testPlayer.AttachFile( fileName.c_str() );
      if( err == TWavePlayer::fileOpeningError )
          bcierr << "Could not open \""
                 << fileName
                 << "\" as a wave sound file"
                 << endl;
      else if( err != TWavePlayer::noError )
      {
          bcierr << "Some general error prevents wave audio playback"
                 << endl;
          genError = true;
      }
    }
  }

  Parameter( "NestedMatrix" );

  PreflightCondition( inputProperties >= SignalProperties( 1, 1, SignalType::int16 ) );
  outputProperties = inputProperties;
}

void
TTask::Initialize()
{
  mAcousticMode = Parameter( "AcousticMode" );
  if( mAcousticMode == amWave )
  {
    string applicationPath = ::ExtractFilePath( ::ParamStr( 0 ) ).c_str();
    for( size_t i = 0; i < Parameter( "Sounds" )->GetNumColumns(); ++i )
    {
      string fileName = applicationPath + string( Parameter( "Sounds", "WAV", i ) );
      mWavePlayers[ i ].AttachFile( fileName.c_str() );
    }
  }

  mVis.Send( CFGID::WINDOWTITLE, "User Task Log" );

  mVis << "All elements of the 'NestedMatrices' parameter:\n";
  ParamRef NestedMatrices = Parameter( "NestedMatrices" );
  int numRows = NestedMatrices->GetNumRows(),
      numCols = NestedMatrices->GetNumColumns();
  for( int row = 0; row < numRows; ++row )
    for( int col = 0; col < numCols; ++col )
    {
      int numSubRows = NestedMatrices( row, col )->GetNumRows(),
          numSubCols = NestedMatrices( row, col )->GetNumColumns();
      if( numSubRows == 1 && numSubCols == 1 )
        mVis << NestedMatrices( row, col );
      else
        for( int subRow = 0; subRow < numSubRows; ++subRow )
          for( int subCol = 0; subCol < numSubCols; ++subCol )
            mVis << NestedMatrices( row, col )( subRow, subCol ) << ' ';
      mVis << endl;
    }

  mpForm->Left = Parameter( "WinXPos" );
  mpForm->Top = Parameter( "WinYPos" );
  mpForm->Width = Parameter( "WinWidth" );
  mpForm->Height = Parameter( "WinHeight" );
  mpForm->Show();
}


int
TTask::MakeMusic( short Controlsignal )
{
  int pitch = ::abs( Controlsignal % 256 );

  // set the position on the progress bar according to the current control signal
  mpProgressbar->Position = pitch;
  // add the current control signal value to the chart
  mpSeries->AddY( Controlsignal, "", TColor( clTeeColor ) );

  // send the current pitch to the operator as well
  ostringstream oss;
  oss << "Current Pitch: " << pitch << '\n';
  mVis.Send( oss.str() );

  int pitchIndex = ultra_high;

  if( pitch < 51 )
    pitchIndex = ultra_low;
  else if( pitch < 102 )
    pitchIndex = low;
  else if( pitch < 153 )
    pitchIndex = medium;
  else if( pitch < 204 )
    pitchIndex = high;

  switch( mAcousticMode )
  {
    case amNone:
      break;
    case amMidi:
      mMidiPlayer.Play( Parameter( "Sounds", "MIDI", pitchIndex ) );
      break;
    case 2: // WAV
      if( !mWavePlayers[ pitchIndex ].IsPlaying() )
        mWavePlayers[ pitchIndex ].Play();
      break;
    default:
      bcierr << "Unknown acoustic mode " << mAcousticMode << endl;
  }
  return pitch;
}


void
TTask::Process( const GenericSignal* Input, GenericSignal* Output )
{
  if( State( "Running" ) )
  {
    // make music
    State( "Pitch" ) = MakeMusic( ( *Input )( 0, 0 ) );
  }
  // time stamp the data
  State( "StimulusTime" ) = BCITIME::GetBCItime_ms();
  *Output = *Input;
}

void TTask::Halt()
{
  mMidiPlayer.StopSequence();
  for( WavePlayerContainer::iterator i = mWavePlayers.begin(); i != mWavePlayers.end(); ++i )
    i->Stop();
}
