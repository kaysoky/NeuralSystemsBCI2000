#include <vcl.h>
#pragma hdrstop

#include "UState.h"
#include "Task.h"
#include "UBCITime.h"


TTask::TTask(  PARAMLIST *plist, STATELIST *slist )
: vis( NULL ),
  svect( NULL ),
  AcousticMode( 0 ),
  form( NULL ),
  progressbar( NULL ),
  chart( NULL ),
  series( NULL )
{
 const char* params[] =
 {
   "NeuralMusic int AcousticMode= 10 0 0 1 // Achin's Acoustic Mode :-)",
   // has to be in there
   "NeuralMusic int NumberTargets= 10 0 0 1 // not used",
 };
 const size_t numParams = sizeof( params ) / sizeof( *params );
 for( size_t i = 0; i < numParams; ++i )
   plist->AddParameter2List( params[ i ] );

 slist->AddState2List("Pitch 8 0 0 0\n");
 slist->AddState2List("StimulusTime 16 17528 0 0\n");

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


void TTask::Initialize( PARAMLIST *plist, STATEVECTOR *new_svect, CORECOMM* corecomm )
{
 AcousticMode=atoi(plist->GetParamPtr("AcousticMode")->GetValue());

 svect=new_svect;

 delete vis;
 vis= new GenericVisualization( plist, corecomm );
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
  const std::vector< float >& signals = Input->GetChannel( 0 );
  int cur_pitch, cur_running;

  cur_pitch=svect->GetStateValue("Pitch");

  cur_running=svect->GetStateValue("Running");
  if (cur_running > 0)
    {
    // make music
    cur_pitch=MakeMusic( signals[ 0 ] );
    }

  svect->SetStateValue("Pitch", 23);
  // time stamp the data
  svect->SetStateValue("StimulusTime", BCITIME::GetBCItime_ms());
}


