#include <vcl.h>
#pragma hdrstop

#include "BCIDirectry.h"

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
 strcpy(line, "P3Speller int InterSetInterval= 1500 1500 0 10000 // Time between sets of n intensifications");
 plist->AddParameter2List(line,strlen(line));

 slist->AddState2List("StimulusTime 16 17528 0 0\n");
 bcitime=new BCITIME();
 logfile=NULL;
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
 if (logfile)           fclose(logfile);

 vis=NULL;
 cur_time=NULL;
 trialsequence=NULL;
 userdisplay=NULL;
 bcitime=NULL;
 logfile=NULL;
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
AnsiString      FInit, SSes, SName;
TColor  BackgroundColor;
char    memotext[256], FName[256];
int     ret;
BCIDtry *bcidtry;

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
  intersetinterval=atoi(plist->GetParamPtr("InterSetInterval")->GetValue());
  FInit= AnsiString (plist->GetParamPtr("FileInitials")->GetValue());
  SSes = AnsiString (plist->GetParamPtr("SubjectSession")->GetValue());
  SName= AnsiString (plist->GetParamPtr("SubjectName")->GetValue());
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
  intersetinterval=1500;
  }

 // open an output file for the task log
 bcidtry= new BCIDtry();
 bcidtry->SetDir( FInit.c_str() );
 bcidtry->ProcPath();
 bcidtry->SetName( SName.c_str() );
 bcidtry->SetSession( SSes.c_str() );
 strcpy(FName, bcidtry->ProcSubDir() );
 strcat(FName, "\\");
 strcat(FName, (SName + "S" + SSes + ".log").c_str() );         // CAT vs CPY
 if (logfile) fclose(logfile);
 logfile= fopen(FName, "a+");
 delete bcidtry;

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
// Function:   DeterminePredictedCharacter
// Purpose:    This function determines which character we have selected
//             by looking at the maximum value for all rows and the max. value for all columns
// Parameters: N/A
// Returns:    the predicted character (i.e., the caption of the predicted target)
// **************************************************************************
AnsiString TTask::DeterminePredictedCharacter()
{
int     i, pickedrow, pickedcol, pickedtargetID;
TARGET  *pickedtargetptr;
float   maxval;

 pickedrow=-1;
 pickedcol=-1;

 // get the column with the highest classification result
 maxval=-9999999999999999;
 for (i=0; i<6; i++)
  {
  if (responsecount[i] > 0)
     if (response[i]/(float)responsecount[i] > maxval)
        {
        maxval=response[i]/(float)responsecount[i];
        pickedcol=i;
        }
  }

 // get the row with the highest classification result
 maxval=-9999999999999999;
 for (i=6; i<12; i++)
  {
  if (responsecount[i] > 0)
     if (response[i]/(float)responsecount[i] > maxval)
        {
        maxval=response[i]/(float)responsecount[i];
        pickedrow=i-6;
        }
  }

  // sanity check
 if ((pickedrow == -1) || (pickedcol == -1))
    return("Error");

 // from row and column, determine the targetID
 pickedtargetID=pickedrow*6+pickedcol+1;
 pickedtargetptr=userdisplay->activetargets->GetTargetPtr(pickedtargetID);
 if (!pickedtargetptr)
    return("Did not find target");

 // finally, return the selected character
 return(pickedtargetptr->Caption);
}


// **************************************************************************
// Function:   ProcessPostSequence
// Purpose:    This function determines when, after a sequence finished, it has to turn off the task
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TTask::ProcessPostSequence()
{
unsigned short  cur_time;
AnsiString      predchar;
char    memotext[256];
int     i;

 if (postsequence)
    {
    // let's wait one second
    cur_time=bcitime->GetBCItime_ms();
    if (bcitime->TimeDiff(postseqtime, cur_time) > intersetinterval)
       {
       // determine predicted character
       predchar=DeterminePredictedCharacter();          // given these responses, determine which character we have picked
       userdisplay->statusbar->resulttext += predchar;
       userdisplay->DisplayStatusBar();

       // write the results in the log file
       fprintf(logfile, "This is the end of this sequence: %d total intensifications\r\n", cur_sequence);
       fprintf(logfile, "Responses for each stimulus:\r\n");
       for (i=0; i<NUM_STIMULI; i++)
        fprintf(logfile, "Response %02d: %.2f\r\n", i+1, response[i]/(float)responsecount[i]);
       fprintf(logfile, "Predicted character: %s\r\n", predchar.c_str());

       // send the results to the operator log
       sprintf(memotext, "This is the end of this sequence: %d total intensifications\r", cur_sequence);
       vis->SendMemo2Operator(memotext);
       sprintf(memotext, "Predicted character: %s\r", predchar.c_str());
       vis->SendMemo2Operator(memotext);

       // if we are in offline mode, suspend the run
       // otherwise, just reset the task sequence and continue
       if (!trialsequence->onlinemode)
          {
          statevector->SetStateValue("Running", 0);
          running=0;
          trialsequence->SuspendTrial();
          }
       else
          ResetTaskSequence();

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
 if ((running == 0) && (oldrunning == 1))
    trialsequence->SuspendTrial();
 if (running == 0) return;
 // has the system been restarted ?
 if ((running == 1) && (oldrunning == 0))
    {
    ResetTaskSequence();
    if (trialsequence->onlinemode)
       sprintf(memotext, "Start of this run in online mode\r");
    else
       sprintf(memotext, "Start of this run in offline mode\r");
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

