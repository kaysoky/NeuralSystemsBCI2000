#include "PCHIncludes.h"
#pragma hdrstop

#include "Task.h"
#include "Localization.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dos.h>

RegisterFilter( TTask, 3 );

// **************************************************************************
// Function:   TASK
// Purpose:    This is the constructor for the TASK class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TTask::TTask()
: vis( NULL ),
  targetsequence( new TARGETSEQUENCE ),
  trialsequence( new TRIALSEQUENCE ),
  userdisplay( new USERDISPLAY ),
  cur_time( new BCITIME )
{

 BEGIN_PARAMETER_DEFINITIONS
   "Oddball int WinXpos= 5 0 0 5000 // "
       "User Window X location",
   "Oddball int WinYpos= 5 0 0 5000 // "
       "User Window Y location",
   "Oddball int WinWidth= 512 512 0 2000 // "
       "User Window Width",
   "Oddball int WinHeight= 512 512 0 2000 // "
       "User Window Height",
   "Oddball int TargetWidth= 5 0 0 100 // "
       "TargetWidth in percent of screen width",
   "Oddball int TargetHeight= 5 0 0 100 // "
       "TargetHeight in percent of screen height",
   "Oddball int TargetTextHeight= 10 0 0 100 // "
       "Height of target labels in percent of screen height",
   "Oddball string BackgroundColor= 0x00585858 0x00505050 0x00000000 0x00000000 // "
       "Background Color (color)",
   "Oddball int NumberTargets= 4 4 0 100 // "
       "Number of Targets ... NOT USED YET",
 END_PARAMETER_DEFINITIONS

 BEGIN_STATE_DEFINITIONS
   "StimulusTime 16 17528 0 0",
 END_STATE_DEFINITIONS

 LANGUAGES "German",
 BEGIN_LOCALIZED_STRINGS
   "Waiting to start ...",
           "Warte ...",
 END_LOCALIZED_STRINGS
}

//-----------------------------------------------------------------------------


// **************************************************************************
// Function:   ~TASK
// Purpose:    This is the destructor for the TASK class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TTask::~TTask()
{
 delete vis;
 delete targetsequence;
 delete trialsequence;
 delete userdisplay;
 delete cur_time;
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
void TTask::Initialize()
{
TColor  BackgroundColor;
char    memotext[256];
int     ret;

 delete vis;
 vis= new GenericVisualization(SOURCEID::TASKLOG);
 vis->Send(CFGID::WINDOWTITLE, "User Task Log");

 Wx=  Parameter( "WinXpos" );
 Wy=  Parameter( "WinYpos" );
 Wxl= Parameter( "WinWidth" );
 Wyl= Parameter( "WinHeight" );

 BackgroundColor=( TColor )strtol( Parameter( "BackgroundColor" ), NULL, 16 );
 userdisplay->TargetWidth=Parameter( "TargetWidth" );
 userdisplay->TargetHeight=Parameter( "TargetHeight" );
 userdisplay->TargetTextHeight=Parameter( "TargetTextHeight" );

 // initialize the between-trial sequence
 targetsequence->Initialize();

 // initialize the within-trial sequence
 trialsequence->Initialize(userdisplay);

 // set the window position, size, and background color
 userdisplay->SetWindowSize(Wy, Wx, Wxl, Wyl, BackgroundColor);
 // get the active targets as a subset of all the potential targets
 if (userdisplay->activetargets) delete userdisplay->activetargets;
 userdisplay->activetargets=targetsequence->GetActiveTargets();       // get the targets that are on the screen first
 // set the initial position/sizes of the current targets, status bar, cursor
 userdisplay->InitializeActiveTargetPosition();
 // userdisplay->DisplayCursor();
 userdisplay->DisplayMessage( ( char* )LocalizableString( "Waiting to start ..." ) );

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
 // get new targets
 userdisplay->activetargets=targetsequence->GetActiveTargets();
 // show the new targets
 userdisplay->InitializeActiveTargetPosition();
}


// **************************************************************************
// Function:   Process
// Purpose:    Processes the control signal sent by the frame work
// Parameters: signals - pointer to the vector of controlsignals (1st element = up/down, 2nd element = left/right)
// Returns:    N/A
// **************************************************************************
void TTask::Process( const GenericSignal* Input,
                           GenericSignal* Output )
{
 const std::vector< float >& signals = Input->GetChannel( 0 );
 TARGET  *selected;

 // use the current control signal to proceed within the trial sequence
 selected=trialsequence->Process(signals);

 // only if a target has been selected,
 // get the next active targets as a subset of all the potential targets
 // and as a result of the selected target
 if (selected) HandleSelected(selected);

 // write the current time, i.e., the "StimulusTime" into the state vector
 State( "StimulusTime" ) = cur_time->GetBCItime_ms();
}

