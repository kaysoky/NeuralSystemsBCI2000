/*************************************************************************
Task.cpp is the source code for the Right Justified Boxes task
*************************************************************************/

#include "Task.h"
#include "BCIDirectry.h"

#include <vcl.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dos.h>


TTask::TTask(  PARAMLIST *plist, STATELIST *slist )
{
 char line[512];

 vis= NULL;

 strcpy(line,"NeuralMusic int AcousticMode= 10 0 0 1 // Achin's Acoustic Mode :-)");
 plist->AddParameter2List(line,strlen(line));
 // has to be in there
 strcpy(line,"NeuralMusic int NumberTargets= 10 0 0 1 // not used");
 plist->AddParameter2List(line,strlen(line));

 slist->AddState2List("Pitch 8 0 0 0\n");
 slist->AddState2List("StimulusTime 16 17528 0 0\n");

 form=new TForm(Application);
 form->Caption="Neural Music Test";

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
}

//-----------------------------------------------------------------------------

TTask::~TTask( void )
{
 if( vis ) delete vis;
 vis= NULL;

 if (progressbar) delete progressbar;
 form->Close();
}


void TTask::Initialize( PARAMLIST *plist, STATEVECTOR *new_svect, CORECOMM *new_corecomm, TApplication *applic)
{
STATELIST       *slist;

 Applic= applic;
 corecomm=new_corecomm;

 AcousticMode=atoi(plist->GetParamPtr("AcousticMode")->GetValue());

 svect=new_svect;
 slist=svect->GetStateListPtr();

 if (vis) delete vis;
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

 progressbar->Position=pitch;
 // update the display
 Application->ProcessMessages();

 // send the current pitch to the operator as well
 sprintf(memotext, "Current Pitch: %d\r", pitch);
 vis->SendMemo2Operator(memotext);

 return(pitch);
}


void TTask::Process( short *signals )
{
int     cur_pitch, cur_running;

 cur_pitch=svect->GetStateValue("Pitch");

 cur_running=svect->GetStateValue("Running");
 if (cur_running > 0)
    {
    // make music
    cur_pitch=MakeMusic(signals[0]);
    }

 svect->SetStateValue("Pitch", 23);
 // time stamp the data
 bcitime=new BCITIME;
 svect->SetStateValue("StimulusTime", bcitime->GetBCItime_ms());
 delete bcitime;
}


