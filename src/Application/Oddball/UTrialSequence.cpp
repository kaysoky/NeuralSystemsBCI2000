#include "PCHIncludes.h"
#pragma hdrstop

#include <stdio.h>

#include "UCoreComm.h"
#include "UParameter.h"
#include "UState.h"
#include "UTargetSequence.h"
#include "UTrialSequence.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


// **************************************************************************
// Function:   TRIALSEQUENCE
// Purpose:    This is the constructor of the TRIALSEQUENCE class
// Parameters: plist - pointer to the paramter list
//             slist - pointer to the state list
// Returns:    N/A
// **************************************************************************
TRIALSEQUENCE::TRIALSEQUENCE(PARAMLIST *plist, STATELIST *slist)
{
int     i;
char    line[512];

 strcpy(line,"Oddball int OnTime= 10 10 0 5000 // Visible duration of icon in units of SampleBlocks");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"Oddball int OffTime= 3 10 0 5000 // Invisible duration of icon in units of SampleBlocks");
 plist->AddParameter2List(line,strlen(line) );

 vis=NULL;

 slist->AddState2List("IconNumber 2 0 0 0\n");
 slist->AddState2List("IconVisible 1 0 0 0\n");
}


// **************************************************************************
// Function:   ~TRIALSEQUENCE
// Purpose:    This is the destructor of the TRIALSEQUENCE class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TRIALSEQUENCE::~TRIALSEQUENCE()
{
 if (vis) delete vis;

 vis=NULL;
}


// **************************************************************************
// Function:   Initialize
// Purpose:    Initialize is called after parameterization to initialize the trial sequence
// Parameters: plist        - pointer to the paramter list
//             new_svect    - current pointer to the state vector
//             new_corecomm - pointer to the communication object
//             new_userdisplay - pointer to the userdisplay (that contains the status bar, the cursor, and the currently active targets)
// Returns:    0 ... if there was a problem (e.g., a necessary parameter does not exist)
//             1 ... OK
// **************************************************************************
int TRIALSEQUENCE::Initialize( PARAMLIST *plist, STATEVECTOR *new_svect, CORECOMM *new_corecomm, USERDISPLAY *new_userdisplay)
{
int     ret;

 corecomm=new_corecomm;
 statevector=new_svect;
 userdisplay=new_userdisplay;

 // if (vis) delete vis;
 // vis= new GenericVisualization( plist, corecomm );
 // vis->SetSourceID(SOURCEID_SPELLERTRIALSEQ);
 // vis->SendCfg2Operator(SOURCEID_SPELLERTRIALSEQ, CFGID_WINDOWTITLE, "Speller Trial Sequence");

 ret=1;
 try
  {
  ontime=atoi(plist->GetParamPtr("OnTime")->GetValue());
  offtime=atoi(plist->GetParamPtr("OffTime")->GetValue());
  }
 catch( TooGeneralCatch& )
  {
  ret=0;
  ontime=10;
  offtime=3;
  }

 // reset the trial's sequence
 ResetTrialSequence();

 return(ret);
}


// **************************************************************************
// Function:   ResetTrialSequence
// Purpose:    Resets the trial's sequence to the beginning of the ITI
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TRIALSEQUENCE::ResetTrialSequence()
{
char    line[256];
int     i;

 cur_sequence=0;
 cur_on=false;

 selectedtarget=NULL;
 oldrunning=0;

 statevector->SetStateValue("IconNumber", 0);
 statevector->SetStateValue("IconVisible", 0);
}


// **************************************************************************
// Function:   SuspendTrial
// Purpose:    Turn off display when trial gets suspended
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TRIALSEQUENCE::SuspendTrial()
{

 userdisplay->HideActiveTargets();           // hide all active targets

 userdisplay->DisplayMessage("TIME OUT !!!");      // display the "TIME OUT" message
}


// **************************************************************************
// Function:   Process
// Purpose:    Processes the control signal provided by the task
// Parameters: controlsignal - pointer to the vector of control signals
// Returns:    pointer to the selected target (if one was selected), or NULL
// **************************************************************************
TARGET *TRIALSEQUENCE::Process(const std::vector<float>& controlsignal)
{
TARGET   *selected;
unsigned short running;

 selected=NULL;

 running=statevector->GetStateValue("Running");

 // when we suspend the system, show the "TIME OUT" message
 if ((running == 0) && (oldrunning == 1))
    {
    SuspendTrial();
    oldrunning=0;
    }

 // don't do anything if running is not 1
 if (running == 0) return(NULL);

 // when we (re)start the system, reset the trial's sequence
 if ((running == 1) && (oldrunning == 0))
    {
    ResetTrialSequence();
    userdisplay->HideMessage();                 // hide any message that's there
    }

 if (cur_on && (cur_sequence == ontime))
    {
    cur_on=false;
    cur_sequence=0;
    selected=userdisplay->activetargets->GetTargetPtr(userdisplay->activetargets->GetTargetID(0));
    userdisplay->HideActiveTargets();
    statevector->SetStateValue("IconVisible", 0);
    statevector->SetStateValue("IconNumber", 0);
    }

 if (!cur_on && (cur_sequence == offtime))
    {
    cur_on=true;
    cur_sequence=0;
    userdisplay->DisplayActiveTargets();           // display all active targets
    statevector->SetStateValue("IconVisible", 1);
    statevector->SetStateValue("IconNumber", userdisplay->activetargets->GetTargetID(0));
    }

 oldrunning=running;
 cur_sequence++;
 return(selected);
}

