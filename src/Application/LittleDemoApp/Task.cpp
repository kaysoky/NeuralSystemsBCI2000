#include "PCHIncludes.h"
#pragma hdrstop

#include "Task.h"
#include "UState.h"
#include "UBCITime.h"

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
   "NeuralMusic int AcousticMode= 10 0 0 1 // Achin's Acoustic Mode :-)",
   // has to be in there
   "NeuralMusic int NumberTargets= 10 0 0 1 // not used",
 END_PARAMETER_DEFINITIONS

 BEGIN_STATE_DEFINITIONS
   "Pitch 8 0 0 0",
   "StimulusTime 16 17528 0 0",
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

void TTask::Initialize()
{
 AcousticMode = Parameter( "AcousticMode" );

 delete vis;
 vis= new GenericVisualization;
 vis->SetSourceID(SOURCEID_TASKLOG);
 vis->SendCfg2Operator(SOURCEID_TASKLOG, CFGID_WINDOWTITLE, "User Task Log");

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
 // update the display
 Application->ProcessMessages();

 // send the current pitch to the operator as well
 sprintf(memotext, "Current Pitch: %d\r", pitch);
 vis->SendMemo2Operator(memotext);

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
  State( "StimulusTime" )= BCITIME::GetBCItime_ms();
}


