#include <vcl.h>
#pragma hdrstop

#include "Task.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dos.h>


// **************************************************************************
// Function:   TASK
// Purpose:    This is the constructor for the TASK class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TTask::TTask(  PARAMLIST *plist, STATELIST *slist )
{
char line[512];

 corecomm=NULL;
 vis=NULL;

 trialsequence=new TRIALSEQUENCE(plist, slist);
 userdisplay=new USERDISPLAY;
 cur_time=new BCITIME;

 strcpy(line,"P3Speller int WinXpos= 5 0 0 5000 // User Window X location");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"P3Speller int WinYpos= 5 0 0 5000 // User Window Y location");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"P3Speller int WinWidth= 512 512 0 2000 // User Window Width");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"P3Speller int WinHeight= 512 512 0 2000 // User Window Height");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"P3Speller int TargetWidth= 5 0 0 100 // TargetWidth in percent of screen width");
 plist->AddParameter2List(line,strlen(line));
 strcpy(line,"P3Speller int TargetHeight= 5 0 0 100 // TargetHeight in percent of screen height");
 plist->AddParameter2List(line,strlen(line));
 strcpy(line,"P3Speller int TargetTextHeight= 10 0 0 100 // Height of target labels in percent of screen height");
 plist->AddParameter2List(line,strlen(line));
 strcpy(line, "P3Speller int StatusBarSize= 10 0 0 100 // Size of status bar in percent of screen height");
 plist->AddParameter2List(line,strlen(line));
 strcpy(line, "P3Speller int StatusBarTextHeight= 8 0 0 100 // Size of status bar text in percent of screen height");
 plist->AddParameter2List(line,strlen(line));

 strcpy(line,"P3Speller string BackgroundColor= 0x00FFFFFF 0x00505050 0x00000000 0x00000000 // Background Color in hex (0x00BBGGRR)");
 plist->AddParameter2List(line,strlen(line));
 strcpy(line, "P3Speller int NumberOfSequences= 15 15 0 100 // Number of sets of 12 intensifications");
 plist->AddParameter2List(line,strlen(line));

 slist->AddState2List("StimulusTime 16 17528 0 0\n");
 bcitime=new BCITIME();
}

//-----------------------------------------------------------------------------


// **************************************************************************
// Function:   ~TASK
// Purpose:    This is the destructor for the TASK class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TTask::~TTask( void )
{
 if (vis)               delete vis;
 if (trialsequence)     delete trialsequence;
 if (userdisplay)       delete userdisplay;
 if (cur_time)          delete cur_time;
 if (bcitime)           delete bcitime;

 vis=NULL;
 cur_time=NULL;
 trialsequence=NULL;
 userdisplay=NULL;
 bcitime=NULL;
}


// **************************************************************************
// Function:   Initialize
// Purpose:    Initializes the task, e.g., resets the trial sequence, etc.
// Parameters: plist        - pointer to the parameter list
//             new_svect    - pointer to the state vector
//             new_corecomm - pointer to the communcation object that communicates with the operator
//             applic       - pointer to the current application
// Returns:    N/A
// **************************************************************************
void TTask::Initialize( PARAMLIST *plist, STATEVECTOR *new_svect, CORECOMM *new_corecomm, TApplication *applic)
{
TColor  BackgroundColor;
char    memotext[256];
int     ret;

 corecomm=new_corecomm;

 if (vis) delete vis;
 vis= new GenericVisualization( plist, corecomm );
 vis->SetSourceID(SOURCEID_TASKLOG);
 vis->SendCfg2Operator(SOURCEID_TASKLOG, CFGID_WINDOWTITLE, "User Task Log");

 try
  {
  Wx=  atoi(plist->GetParamPtr("WinXpos")->GetValue());
  Wy=  atoi(plist->GetParamPtr("WinYpos")->GetValue());
  Wxl= atoi(plist->GetParamPtr("WinWidth")->GetValue());
  Wyl= atoi(plist->GetParamPtr("WinHeight")->GetValue());

  BackgroundColor=(TColor)strtol(plist->GetParamPtr("BackgroundColor")->GetValue(), NULL, 16);
  userdisplay->TargetWidth=atof(plist->GetParamPtr("TargetWidth")->GetValue());
  userdisplay->TargetHeight=atof(plist->GetParamPtr("TargetHeight")->GetValue());
  userdisplay->TargetTextHeight=atof(plist->GetParamPtr("TargetTextHeight")->GetValue());
  userdisplay->StatusBarSize=atof(plist->GetParamPtr("StatusBarSize")->GetValue());
  userdisplay->StatusBarTextHeight=atof(plist->GetParamPtr("StatusBarTextHeight")->GetValue());
  numberofsequences=atoi(plist->GetParamPtr("NumberOfSequences")->GetValue());
  }
 catch(...)
  {
  BackgroundColor=clDkGray;
  userdisplay->TargetWidth=5;
  userdisplay->TargetTextHeight=10;
  Wx=5;
  Wy=5;
  Wxl=512;
  Wyl=512;
  }

 statevector=new_svect;

 // set the window position, size, and background color
 userdisplay->SetWindowSize(Wy, Wx, Wxl, Wyl, BackgroundColor);

 // initialize the within-trial sequence
 trialsequence->Initialize(plist, statevector, new_corecomm, userdisplay);

 // show the user window
 userdisplay->form->Show();

 cur_sequence=0;
 oldrunning=0;
}


void TTask::ResetTaskSequence()
{
int     i;

 cur_sequence=0;       // this counts the total number of intensifications

 // reset the statistics for the results of the different stimuli
 for (i=0; i<NUM_STIMULI; i++)
  {
  responsecount[i]=0;
  response[i]=0;
  }
  
 postsequence=false;
}


// **************************************************************************
// Function:   ProcessPostSequence
// Purpose:    This function determines when, after a sequence finished, it has to turn off the task
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TTask::ProcessPostSequence()
{
unsigned short     cur_time;
char    memotext[256];
int     i;

 if (postsequence)
    {
    // let's wait one second
    cur_time=bcitime->GetBCItime_ms();
    if (bcitime->TimeDiff(postseqtime, cur_time) > 1500)
       {
       statevector->SetStateValue("Running", 0);
       running=0;
       trialsequence->SuspendTrial();
       sprintf(memotext, "This is the end of this run: %d total intensifications\r", cur_sequence);
       vis->SendMemo2Operator(memotext);
       sprintf(memotext, "Responses for each stimulus:\r");
       vis->SendMemo2Operator(memotext);
       for (i=0; i<NUM_STIMULI; i++)
        {
        sprintf(memotext, "Response %02d: %.2f\r", i+1, response[i]/(float)responsecount[i]);
        vis->SendMemo2Operator(memotext);
        }
       postsequence=false;
       }
    }
}


// **************************************************************************
// Function:   ProcessSigProcResults
// Purpose:    This function does statistics on the results from SigProc
// Parameters: signals - pointer to the vector of controlsignals (1st element = up/down, 2nd element = left/right)
// Returns:    N/A
// **************************************************************************
void TTask::ProcessSigProcResults( short *signals )
{
unsigned short cur_stimuluscoderes, cur_stimulustyperes;

 // did we get a resulting classification from Signal Processing ?
 cur_stimuluscoderes=statevector->GetStateValue("StimulusCodeRes");
 cur_stimulustyperes=statevector->GetStateValue("StimulusTypeRes");
 // we got one if StimulusCodeRes > 0
 if (cur_stimuluscoderes > 0)
    {
    responsecount[cur_stimuluscoderes-1]++;
    response[cur_stimuluscoderes-1] += (float)signals[0];    // use the first control signal as classification result
    }
}


// **************************************************************************
// Function:   Process
// Purpose:    Processes the control signal sent by the frame work
// Parameters: signals - pointer to the vector of controlsignals (1st element = up/down, 2nd element = left/right)
// Returns:    N/A
// **************************************************************************
void TTask::Process( short *signals )
{
char    memotext[256];
int     ret;

 running=statevector->GetStateValue("Running");
 // don't do anything if running is not 1
 if (running == 0) return;
 // has the system been restarted ?
 if ((running == 1) && (oldrunning == 0))
    {
    ResetTaskSequence();
    sprintf(memotext, "Start of this run\r");
    vis->SendMemo2Operator(memotext);
    }

 // do statistics on the results from signal processing
 ProcessSigProcResults(signals);

 // we have to wait a little after a sequence ended (before we turn off the task)
 // to let the final ERP results to come in
 ProcessPostSequence();

 // skip processing the trial if we are in the process of turning the task off
 if ((running == 0) || (postsequence)) goto skipprocess;

 // use the current control signal to proceed within the trial sequence
 ret=trialsequence->Process(signals);

 // whenever the trialsequence returns 1, a trial (i.e., one intensification), is over
 if (ret == 1)
    {
    cur_sequence++;
    // did we have enough total intensifications ?
    // then end this run
    if (cur_sequence >= numberofsequences*NUM_STIMULI)
       {
       postsequence=true;
       postseqtime=bcitime->GetBCItime_ms();
       }
    }

 skipprocess:

 // write the current time, i.e., the "StimulusTime" into the state vector
 statevector->SetStateValue("StimulusTime", cur_time->GetBCItime_ms());
 oldrunning=running;
}

