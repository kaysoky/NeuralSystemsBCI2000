#include "PCHIncludes.h"
#pragma hdrstop

#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dos.h>

#include "UState.h"
#include "BCIDirectry.h"
#include "UBCIError.h"
#include "Localization.h"
#include "MeasurementUnits.h"

#include "Task.h"

using namespace std;

RegisterFilter( TTask, 3 );

// **************************************************************************
// Function:   TASK
// Purpose:    This is the constructor for the TASK class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TTask::TTask()
: mVis( SOURCEID::TASKLOG ),
  trialsequence( new TRIALSEQUENCE ),
  userdisplay( new USERDISPLAY ),
  cur_time( new BCITIME ),
  bcitime( new BCITIME ),
  logfile( NULL ),
  // this keeps track of the current run number and will be set to 0 only here
  cur_runnr( 0 ),
  f( NULL )
{
 BEGIN_PARAMETER_DEFINITIONS
  "P3Speller int WinXpos= 5 0 0 5000 // "
      "User Window X location",
  "P3Speller int WinYpos= 5 0 0 5000 // "
      "User Window Y location",
  "P3Speller int WinWidth= 512 512 0 2000 // "
      "User Window Width",
  "P3Speller int WinHeight= 512 512 0 2000 // "
      "User Window Height",
  "P3Speller int TargetWidth= 5 0 0 100 // "
      "TargetWidth in percent of screen width",
  "P3Speller int TargetHeight= 5 0 0 100 // "
      "TargetHeight in percent of screen height",
  "P3Speller int TargetTextHeight= 10 0 0 100 // "
      "Height of target labels in percent of screen height",
  "P3Speller int StatusBarSize= 10 0 0 100 // "
      "Size of status bar in percent of screen height",
  "P3Speller int StatusBarTextHeight= 8 0 0 100 // "
      "Size of status bar text in percent of screen height",
  "P3Speller string BackgroundColor= 0x00FFFFFF 0x00505050 0x00000000 0x00000000 // "
      "Background Color (color)",
  "P3Speller int NumberOfSequences= 15 15 0 0 // "
      "Number of sets of 12 intensifications",
  "P3Speller int PostSetInterval= 60 60 0 10000 // "
      "Duration after set of n intensifications in units of SampleBlocks",
  "P3Speller int PreSetInterval= 60 60 0 10000 // "
      "Duration before set of n intensifications in units of SampleBlocks",
  "P3Speller string TextResult = %20 %20 %20 %20 //"
      "User spell result",
  "P3Speller int P3TestMode = 0 0 0 1 //"
      "P3TestMode (0=no, 1=yes) (boolean)",

 END_PARAMETER_DEFINITIONS

 BEGIN_STATE_DEFINITIONS
  "SelectedTarget 7 0 0 0",
  "SelectedRow 3 0 0 0",
  "SelectedColumn 3 0 0 0",
  "PhaseInSequence 2 0 0 0",
  "StimulusTime 16 17528 0 0",
 END_STATE_DEFINITIONS

 LANGUAGES "German",
 BEGIN_LOCALIZED_STRINGS
  "TIME OUT !!!",
           "Zeit abgelaufen!",
  "Waiting to start ...",
           "Warte ...",
 END_LOCALIZED_STRINGS

 /*shidong starts*/
 textresult = "";
 debug = false;
 if(debug) f = fopen ("TaskDebug.txt", "w");
 
 /*shidong ends*/
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
  delete trialsequence;
  // if (userdisplay) delete userdisplay;
  delete cur_time;
  delete bcitime;
  if( logfile ) fclose(logfile);
  if( f ) fclose(f);
  userdisplay=NULL;
}

// **************************************************************************
// Function:   current_directory
// Purpose:    Gets the current directory, including drive letter
// Parameters: path ... pointer to buffer of length MAXPATH
// Returns:    pointer to buffer that includes current directory
// **************************************************************************
char *current_directory(char *path)
{
  strcpy(path, "X:\\");      /* fill string with form of response: X:\ */
  path[0] = 'A' + getdisk();    /* replace X with current drive letter */
  getcurdir(0, path+3);  /* fill rest of string with current directory */
  return(path);
}


void TTask::Preflight( const SignalProperties&, SignalProperties& ) const
{
}//Preflight



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
char    memotext[256], cur_dir[MAXPATH];
int     ret, numerpsamples, sampleblocksize;


/*shidong starts*/
  PreflightCondition( Parameter("NumMatrixColumns")*Parameter("NumMatrixRows") ==  Parameter("TargetDefinitionMatrix")->GetNumRows() );
  textresult = ( const char* )Parameter("TextResult");
  if(debug) fprintf(f, "In Initialize, its length is %d, andtextresult is %s.\n", textresult.Length(), textresult);
  P3TestMode = Parameter("P3TestMode");

/*shidong ends*/

  mVis.Send( CFGID::WINDOWTITLE, "User Task Log" );

  Wx=  Parameter( "WinXpos" );
  Wy=  Parameter( "WinYpos" );
  Wxl= Parameter( "WinWidth" );
  Wyl= Parameter( "WinHeight" );

  BackgroundColor=(TColor)strtol(Parameter("BackgroundColor"), NULL, 16);
  userdisplay->TargetWidth=Parameter("TargetWidth");
  userdisplay->TargetHeight=Parameter("TargetHeight");
  userdisplay->TargetTextHeight=Parameter("TargetTextHeight");
  userdisplay->StatusBarSize=Parameter("StatusBarSize");
  userdisplay->StatusBarTextHeight=Parameter("StatusBarTextHeight");
  numberofsequences=Parameter("NumberOfSequences");
  postsetinterval=MeasurementUnits::ReadAsTime(Parameter("PostSetInterval"));
  presetinterval=MeasurementUnits::ReadAsTime(Parameter("PreSetInterval"));
  numerpsamples=Parameter("NumSamplesInERP");
  sampleblocksize=Parameter("SampleBlockSize");

  // we have to make sure that we wait long enough after a set of n intensifications
  // to get all the responses
  if (postsetinterval*sampleblocksize <= numerpsamples)
    bciout << "PostSetInterval shorter than time derived by NumERPSamples (we have to wait long enough to get the final response)"
           << endl;

  // open an output file for the task log
  current_directory(cur_dir);    // store current directory
  string FName = BCIDirectory()
   .SubjectDirectory( Parameter( "FileInitials" ) )
   .SubjectName( Parameter( "SubjectName" ) )
   .SessionNumber( Parameter( "SubjectSession" ) )
   .CreatePath()
   .FilePath()
   + ".log";
  if (logfile) fclose(logfile);
    logfile= fopen(FName.c_str(), "a+");
  if( !logfile )
    bcierr << "Could not open " << FName << " for writing" << std::endl;
  ChDir(AnsiString(cur_dir));    // restore current directory

  State( "PhaseInSequence" ) = 0;

  // set the window position, size, and background color
  userdisplay->SetWindowSize(Wy, Wx, Wxl, Wyl, BackgroundColor);

  // initialize the within-trial sequence
  trialsequence->Initialize(userdisplay);

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
 for (i=0; i<trialsequence->NUM_STIMULI; i++)
  {
  responsecount[i]=0;
  response[i]=0;
  }


 postsequence=false;
 presequence=false;
 if (presetinterval > 0) presequence=true;
 presequencecount=0;
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
 /*shidong starts*/
// for (i=0; i<6; i++)
 for (i=0; i<trialsequence->NumMatrixColumns; i++)
  {
        //if(debug)fprintf(f, "responsecount[%d] is %d.\n", i, responsecount[i]);
        //if(debug)fprintf(f, "response[%d] is %d.\n", i, response[i]);
  if (responsecount[i] > 0)
     if (response[i]/(float)responsecount[i] > maxval)
        {
        maxval=response[i]/(float)responsecount[i];
        pickedcol=i;
        }
  }

        //if(debug)fprintf(f, "From 0 ~ 6 columns, picked column is %d. maxval is %f.\n", pickedcol+1, maxval);

 // get the row with the highest classification result
 maxval=-9999999999999999;
 //for (i=6; i<12; i++)
 for (i=trialsequence->NumMatrixColumns; i< (trialsequence->NumMatrixColumns+trialsequence->NumMatrixRows); i++)
  {
        //if(debug)fprintf(f, "responsecount[%d] is %d.\n", i, responsecount[i]);
        //if(debug)fprintf(f, "response[%d] is %d.\n", i, response[i]);
  if (responsecount[i] > 0)
     if (response[i]/(float)responsecount[i] > maxval)
        {
        maxval=response[i]/(float)responsecount[i];
        pickedrow=i-trialsequence->NumMatrixColumns;
        //if(debug)fprintf(f, "In row, maxval is %d and pickedrow is %d and i is %d.\n", maxval, pickedrow, i);
        }
  }

        //if(debug)fprintf(f, "From 6 ~ 12 rows, picked row is %d. maxval is %f.\n", pickedrow+1, maxval);
 /*shidong ends*/

  // sanity check
 if ((pickedrow == -1) || (pickedcol == -1))
    return("Error");

 // from row and column, determine the targetID

 pickedtargetID=pickedrow*(trialsequence->NumMatrixColumns)+pickedcol+1;
 /*shidong starts*/
 /*DEBUG 
 pickedtargetID = random(36);
 pickedtargetID ++;     //avoid id == 0
*/ /*shidong ends*/

 State( "SelectedTarget" ) = pickedtargetID;
 State( "SelectedRow" ) = pickedrow + 1;
 State( "SelectedColumn" ) = pickedcol + 1;
 pickedtargetptr=userdisplay->activetargets->GetTargetPtr(pickedtargetID);
 if (!pickedtargetptr)
    return("Did not find target");

 // finally, return the selected character
 /*shidong starts
// return(pickedtargetptr->Caption);
char    memotext[256];
sprintf(memotext, "selected targetID is %d, Display is %s, Result is %s.\r",
        pickedtargetID, pickedtargetptr->CharDisplayInMatrix, pickedtargetptr->CharDisplayInResult);

  mVis.Send( memotext ); */
//if(debug)fprintf(f, "pickedtargetID is %d, pickedrow is %d, pickedcol is %d, result is %s.\n", pickedtargetID, pickedrow+1, pickedcol+1, pickedtargetptr->CharDisplayInResult);

        if (P3TestMode == 1)            //if testmode
        {
                TARGET *temp;
                AnsiString toRet ="";
                for(int i=0; i<userdisplay->activetargets->GetNumTargets(); i++)
                {
                      temp = userdisplay->activetargets->GetTargetPtr(i+1);
                      if (temp->IsClickedOn())
                      {         //get the last target that is clicked
                                toRet = temp->CharDisplayInResult;
                      }
                }
                if(toRet == "")
                {
                        return(pickedtargetptr->CharDisplayInResult);
                }
                else
                {
                        return toRet;
                }
        }

        return(pickedtargetptr->CharDisplayInResult);

 /*shidong ends*/
}


// **************************************************************************
// Function:   ProcessPreSequence
// Purpose:    We just count how long we are in the pre-sequence period    
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TTask::ProcessPreSequence()
{
 // don't process anything if we are in the pre-sequence period
 if (presequence)
    {
    State( "PhaseInSequence" ) = 1;
    if (presequencecount == 0)
       {
       // display the matrix, etc.
       trialsequence->GetReadyForTrial();
       }
    if (presequencecount >= presetinterval)
       {
       presequence=false;
       presequencecount=0;
       }
    presequencecount++;
    }
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
    State( "PhaseInSequence" ) = 3;
    if ((postsequencecount > postsetinterval) && (!postpostsequence))
       {
       // determine predicted character
       predchar=DeterminePredictedCharacter();          // given these responses, determine which character we have picked
       /*shidong starts*/
        

       if (predchar == "<END>")                         //check for user "end" input
       {
         State("Running")=0;
        //return;        
       }
       else if (predchar == "<BS>")           //check for backspace
       {
        trialsequence->char2spellidx -= 1;
        if (trialsequence->char2spellidx < 1)           //if 1st predchar is <BS>
                trialsequence->char2spellidx = 1;
        //delete one character                
       //userdisplay->statusbar->resulttext.Delete(userdisplay->statusbar->resulttext.Length(),1);
        textresult.Delete(textresult.Length(), 1);
       }
       else
       {
        /*sprintf(memotext, "Adding  char2spellidx %d.\r", trialsequence->char2spellidx);
        mVis.Send(memotext);  */
        trialsequence->char2spellidx += 1;
         //userdisplay->statusbar->resulttext += predchar;
        textresult += predchar;
       }

       if(!trialsequence->onlinemode)  //if offline
       { 
        userdisplay->statusbar->resulttext = textresult;
       }
       else             //if online
       {
        int textIndex = textresult.LastDelimiter(" ");          //get index of last space
        userdisplay->statusbar->resulttext = textresult.SubString(textIndex+1, textresult.Length());
        userdisplay->statusbar->goaltext = textresult.SubString(0, textIndex);
        if (userdisplay->statusbar->resulttext.Length()==0)    //check for null
                userdisplay->statusbar->resulttext  = "";
        if (userdisplay->statusbar->goaltext.Length()==0)       //check for null        
                userdisplay->statusbar->goaltext  = "";  
       }

       /*shidong ends*/

       trialsequence->SetUserDisplayTexts();
       userdisplay->DisplayStatusBar();

       // write the results in the log file
       if (logfile)
          {
          fprintf(logfile, "This is the end of this sequence: %d total intensifications\r\n", cur_sequence);
          fprintf(logfile, "Responses for each stimulus:\r\n");
          for (i=0; i<trialsequence->NUM_STIMULI; i++)
          fprintf(logfile, "Response %02d: %.2f\r\n", i+1, response[i]/(float)responsecount[i]);
          fprintf(logfile, "Predicted character: %s\r\n", predchar.c_str());
          }

       // send the results to the operator log
       sprintf(memotext, "This is the end of this sequence: %d total intensifications\r", cur_sequence);
       mVis.Send( memotext );

       sprintf(memotext, "Predicted character: %s\r", predchar.c_str());
       mVis.Send( memotext );
       /*shidong starts  
       sprintf(memotext, "The cur_stimuluscode is %d.\r", trialsequence->c);
       mVis.Send(memotext);  */
       /*shidong ends*/

       // if we are in offline mode, suspend the run if we had spelled enough characters (otherwise, continue)
       // if we are in online mode, just reset the task sequence and continue
       if (!trialsequence->onlinemode)
          {
          // we want the postsequence just one cycle longer so that the final
          // classification result gets reflected in the file
          if (trialsequence->char2spellidx > trialsequence->TextToSpell.Length())
             postpostsequence=true;
          else
             ResetTaskSequence();
          }
       else
          ResetTaskSequence();

       // always end the postsequence, except when we tuck on one extra cycle at the end of the offline mode
       if (!postpostsequence) postsequence=false;
       }
    else
       {
       // turn it off one cycle afterwards; now, the classification of the final character is reflected in the file
       if (postpostsequence)
          {
          postpostsequence=false;
          postsequence=false;
          State( "Running" ) = 0;
          running=0;
          /*shidong starts*/
          //Resting();
          /*shidong ends*/
          trialsequence->SuspendTrial();
          }
       }
    postsequencecount++;
    }
}


// **************************************************************************
// Function:   ProcessSigProcResults
// Purpose:    This function does statistics on the results from SigProc
// Parameters: signals - pointer to the vector of controlsignals (1st element = up/down, 2nd element = left/right)
// Returns:    N/A
// **************************************************************************
void TTask::ProcessSigProcResults( const std::vector<float>& signals )
{
unsigned short cur_stimuluscoderes, cur_stimulustyperes;

 // did we get a resulting classification from Signal Processing ?
 cur_stimuluscoderes=State("StimulusCodeRes");
 cur_stimulustyperes=State("StimulusTypeRes");
 // we got one if StimulusCodeRes > 0
 if (cur_stimuluscoderes > 0)
    {
    responsecount[cur_stimuluscoderes-1]++;
    response[cur_stimuluscoderes-1] += (float)signals[0];    // use the first control signal as classification result
    /*shidong debug starts  
    response[cur_stimuluscoderes-1] +=  (float)rand();
   */ //if(debug) fprintf(f, "StimulusCodeRes:\t %d, \t response[%d]:\t  %f.\n", cur_stimuluscoderes, cur_stimuluscoderes-1, response[cur_stimuluscoderes-1]);
   /*shidong debug ends*/

    }
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
char    memotext[256];
int     ret;

 running=State("Running");
 // don't do anything if running is not 1
 if ((running == 0) && (oldrunning == 1))
    {
    /*shidong starts*/
    //Resting();
    trialsequence->SuspendTrial();
    State("PhaseInSequence" ) = 0;
    /*shidong ends*/
    }
 if (running == 0)
 /*shiodng starts*/
 {
        //Resting();
        return;
 }
 /*shidong ends*/
 // has the system been restarted ?
 if ((running == 1) && (oldrunning == 0))
    {
    State( "SelectedTarget" ) = 0;
    State( "SelectedRow" ) = 0;
    State( "SelectedColumn" ) = 0;
    postpostsequence=false;
    cur_runnr++;
    ResetTaskSequence();
    mVis.Send( "******************************" );
    if (trialsequence->onlinemode)
       sprintf(memotext, "Start of run %d in online mode\r", cur_runnr);
    else
       sprintf(memotext, "Start of run %d in offline mode\r", cur_runnr);
    mVis.Send( memotext );
    if (logfile) fprintf(logfile, "******************************\r\n%s\n", memotext);
    trialsequence->char2spellidx=1;
    /*shidong starts*/
   // userdisplay->statusbar->resulttext="";
   // trialsequence->SetUserDisplayTexts();
   //
    if (!trialsequence->onlinemode)      //offline mode
    {
        trialsequence->chartospell=trialsequence->TextToSpell.SubString(trialsequence->char2spellidx, 1);
        userdisplay->statusbar->goaltext=trialsequence->TextToSpell+" ("+trialsequence->chartospell+")";
        if(textresult.Length()>0)       //if textresult is not empty
        {
                textresult = "";
                
        }
        userdisplay->statusbar->resulttext="";
    }
     else // if we are in online mode
    {
         if(debug) fprintf(f, "In OnlineMode.\n");  

        if (textresult.Length()>0)
        {
                if(debug) fprintf(f, "textresult has something, it's %s, its length is %d.\n", textresult, textresult.Length());
                if(textresult=="(null)")
                {
                        textresult = "";
                        userdisplay->statusbar->resulttext="";
                        userdisplay->statusbar->goaltext="";
                        if(debug) fprintf(f, "in\n");
                }
                int textIndex = textresult.LastDelimiter(" ");          //get index of last space
                userdisplay->statusbar->resulttext = textresult.SubString(textIndex+1, textresult.Length());
                userdisplay->statusbar->goaltext = textresult.SubString(0, textIndex);
                if (userdisplay->statusbar->resulttext.Length()==0)    //check for null
                        userdisplay->statusbar->resulttext  = "";
                if (userdisplay->statusbar->goaltext.Length()==0)       //check for null
                        userdisplay->statusbar->goaltext  = ""; 
        }
        else
        {
                if(debug) fprintf(f, "textresult has nothing, it's %s, its length is %d.\n", textresult, textresult.Length());
                textresult = "";
                userdisplay->statusbar->resulttext="";
                userdisplay->statusbar->goaltext="";
                if(debug) fprintf(f, "setting goal and result text to \"\".\n");
        }
    }
        /*shidong ends*/

    }

 // if we have a period before the sequence, we have to process it
 ProcessPreSequence();

 // do statistics on the results from signal processing
 ProcessSigProcResults(signals);

 // we have to wait a little after a sequence ended (before we turn off the task)
 // to let the final ERP results to come in
 ProcessPostSequence();

 // skip processing the trial if we are in the pre- or post sequence period (i.e., matrix visible, but no flashing)
 if ((running == 0) || (postsequence) || (presequence)) goto skipprocess;

 // use the current control signal to proceed within the trial sequence
 ret=trialsequence->Process(signals);
 State( "PhaseInSequence" ) = 2;

 // whenever the trialsequence returns 1, a trial (i.e., one intensification), is over
 if (ret == 1)
    {
    cur_sequence++;
    // did we have enough total intensifications ?
    // then end this run
    if (cur_sequence >= numberofsequences*trialsequence->NUM_STIMULI)
       {
       postsequence=true;
       postsequencecount=0;
       }
    }

 skipprocess:
 //Resting();
 // write the current time, i.e., the "StimulusTime" into the state vector
 State( "StimulusTime" ) = cur_time->GetBCItime_ms();
 oldrunning=running;
}

void TTask::StopRun()
{
 /*shidong starts */
 if(debug) fprintf(f, "in stoprun().\n");
 if(trialsequence->onlinemode) //if online
 {
//         char memo[256] ;
  //      sprintf(memo, "%s", textresult);
        Parameter("TextResult") = textresult.c_str();
 }
 else   //if offline
 {
        
 }
 /*shidong ends*/
}
