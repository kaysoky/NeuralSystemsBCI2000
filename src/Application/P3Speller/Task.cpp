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

 strcpy(line, "P3Speller int NumberTargets= 4 4 0 100 // Number of Targets ... NOT USED YET");
 plist->AddParameter2List(line,strlen(line));

 slist->AddState2List("StimulusTime 16 17528 0 0\n");
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

 vis=NULL;
 cur_time=NULL;
 trialsequence=NULL;
 userdisplay=NULL;
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
}


// **************************************************************************
// Function:   HandleSelected
// Purpose:    If the user actually selected a character, decide what to do
//             e.g., bring up old targets and possibly delete spelled character when selected target was BACK UP
//                   when selection was an actual character, add this character to the spelled word, etc.
// Parameters: selected - pointer to the selected target
// Returns:    N/A
// **************************************************************************
void TTask::HandleSelected(TARGET *selected)
{
 // in our case, we do not get new targets
 // since the targets are our alphabet

 // get new targets
 // userdisplay->activetargets=targetsequence->GetActiveTargets();
 // show the new targets
 // userdisplay->InitializeActiveTargetPosition();
}


// **************************************************************************
// Function:   Process
// Purpose:    Processes the control signal sent by the frame work
// Parameters: signals - pointer to the vector of controlsignals (1st element = up/down, 2nd element = left/right)
// Returns:    N/A
// **************************************************************************
void TTask::Process( short *signals )
{
TARGET  *selected;

 // use the current control signal to proceed within the trial sequence
 selected=trialsequence->Process(signals);

 // only if a target has been selected,
 // get the next active targets as a subset of all the potential targets
 // and as a result of the selected target
 if (selected) HandleSelected(selected);

 // write the current time, i.e., the "StimulusTime" into the state vector
 statevector->SetStateValue("StimulusTime", cur_time->GetBCItime_ms());
}

