#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------


#include <stdio.h>

#include "UParameter.h"
#include "UState.h"
#include "UTrialSequence.h"
#include "UTaskUtil.h"
#include "UBCIError.h"
#include "Localization.h"

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

 targets=new TARGETLIST();

 /*shidong starts*/
 
 debug = false;
 if(debug) f = fopen("debug2.txt", "w");
 strcpy(line, "P3Speller matrix TargetDefinitionMatrix= 36 {Display Enter Display%20Size } A A 1 B B 1 C C 1 D D 1 E E 1 F F 1 G G 1 H H 1 I I 1 J J 1 K K 1 L L 1 M M 1 N N 1 O O 1 P P 1 Q Q 1 R R 1 S S 1 T T 1 U U 1 V B 1 W W 1 X X 1 Y Y 1 Z Z 1 1 1 1 2 2 1 3 3 1 4 4 1 5 5 1 6 6 1 7 7 1 8 8 1 9 9 1 _ _ 1 0 0 100 // Target Definition Matrix");
 plist->AddParameter2List(line,strlen(line));
 strcpy(line,"P3Speller int NumMatrixColumns= 6 6 0 6 // Display Matrix's Column Number");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"P3Speller int NumMatrixRows= 6 6 0 6 // Display Matrix's Row Number");
 plist->AddParameter2List(line,strlen(line) );

 /*shidong ends*/

 strcpy(line,"P3Speller int OnTime= 4 10 0 5000 // Duration of intensification in units of SampleBlocks");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"P3Speller int OffTime= 1 10 0 5000 // Interval between intensification in units of SampleBlocks");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"P3Speller int OnlineMode= 0 0 0 1 // Online mode (0=no, 1=yes)");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"P3Speller int ResultDisplay= 1 0 0 1 // Display results (0=no, 1=yes)");
 plist->AddParameter2List(line,strlen(line) );
 strcpy(line,"P3Speller string TextColor= 0x00000000 0x00505050 0x00000000 0x00000000 // Text Color in hex (0x00BBGGRR)");
 plist->AddParameter2List(line,strlen(line));
 strcpy(line,"P3Speller string TextColorIntensified= 0x000000FF 0x00505050 0x00000000 0x00000000 // Text Color in hex (0x00BBGGRR)");
 plist->AddParameter2List(line,strlen(line));
 strcpy(line,"P3Speller string TextToSpell= P P A Z // Character or string to spell in offline mode");
 plist->AddParameter2List(line,strlen(line));

 vis=NULL;

 slist->AddState2List("StimulusCode 5 0 0 0\n");
 slist->AddState2List("StimulusType 3 0 0 0\n");
 slist->AddState2List("Flashing 1 0 0 0\n");
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
 if (targets) delete targets;
 targets=NULL;
 /*shidong starts*/
 fclose(f);        
 /*shidong ends*/
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
int TRIALSEQUENCE::Initialize( PARAMLIST *plist, STATEVECTOR *new_svect, USERDISPLAY *new_userdisplay)
{
int     ret;

 // load and create all potential targets
 /*shidong starts*/
 ret=LoadPotentialTargets(plist->GetParamPtr("TargetDefinitionMatrix")->GetNumColumns(), plist->GetParamPtr("TargetDefinitionMatrix")->GetNumRows());
 NumMatrixColumns = Parameter("NumMatrixColumns");
 NumMatrixRows = Parameter("NumMatrixRows");
 NUM_STIMULI = NumMatrixColumns + NumMatrixRows;
 /*shidong ends*/
 if (ret == 0)
    {
    bcierr << "P3 Speller: Could not open target definition file" << std::endl;
    return(0);
    }

 statevector=new_svect;
 userdisplay=new_userdisplay;

 // if (vis) delete vis;
 // vis= new GenericVisualization( plist, corecomm );
 // vis->SetSourceID(SOURCEID_SPELLERTRIALSEQ);
 // vis->SendCfg2Operator(SOURCEID_SPELLERTRIALSEQ, CFGID_WINDOWTITLE, "Speller Trial Sequence");

 ret=1;
 ontime = Parameter( "OnTime" );
 offtime = Parameter( "OffTime" );
 TextColor=(TColor)strtol( Parameter( "TextColor" ), NULL, 16 );
 TextColorIntensified=(TColor)strtol( Parameter( "TextColorIntensified" ), NULL, 16 );
 TextToSpell = ( const char* )Parameter( "TextToSpell" );
 onlinemode = ( int )Parameter( "OnlineMode" );

 // get the active targets as a subset of all the potential targets
 if (userdisplay->activetargets) delete userdisplay->activetargets;
 userdisplay->activetargets=GetActiveTargets();       // get the targets that are on the screen first

 /*shidong starts*/
        //tell the userdisplay the number of rows and columns
        userdisplay->displayCol = NumMatrixColumns;
        userdisplay->displayRow = NumMatrixRows;
 /*shidong ends*/

 // set the initial position/sizes of the current targets, status bar, cursor
 userdisplay->InitializeActiveTargetPosition();
 bool cur_resulttextvisible;
 if (( int )Parameter( "ResultDisplay" ) == 1)
    cur_resulttextvisible=true;
 else
    cur_resulttextvisible=false;
 userdisplay->InitializeStatusBarPosition(cur_resulttextvisible);
 // userdisplay->DisplayCursor();
 userdisplay->DisplayMessage( const_cast<char*>( LocalizableString( "Waiting to start ..." ) ) );

 oldrunning=0;

 // reset the trial's sequence
 ResetTrialSequence();

 return(ret);
}


int TRIALSEQUENCE::LoadPotentialTargets(const int col, const int row)
{
/*shidong starts*/
TARGET  *cur_target;
int     targetID, targettype;

// if we already have a list of potential targets, delete this list
if (targets) delete targets;
targets = new TARGETLIST();

cur_target = new TARGET(0);    //assign targetID
cur_target->Caption = "";
cur_target->IconFile = "";
cur_target->CharDisplayInMatrix="";
cur_target->CharDisplayInResult="";
cur_target->FontSizeFactor=1;
cur_target->targettype=TARGETTYPE_NOTYPE;
targets->Add(cur_target);

for (int i = 0; i<row; i++)     //parse each row of the TargetDefinitionMatrix
{
        targetID = i+1;
        cur_target = new TARGET(targetID);    //assign targetID
        cur_target->Caption =  AnsiString((const char*)Parameter("TargetDefinitionMatrix", i, 0)) ;
       //assign caption to character display in column 1

        cur_target->IconFile = "";
        cur_target->CharDisplayInMatrix=AnsiString((const char*)Parameter("TargetDefinitionMatrix", i, 0));
        cur_target->CharDisplayInResult=AnsiString((const char*)Parameter("TargetDefinitionMatrix", i, 1));        //assign display result in column 2
        cur_target->FontSizeFactor=((float)Parameter("TargetDefinitionMatrix", i, 2));//assign font size factor in column 3
        targettype=TARGETTYPE_NOTYPE;
        if ((targetID == TARGETID_BLANK) || (targetID == TARGETID_BACKUP) || (targetID == TARGETID_ROOT))
                targettype=TARGETTYPE_CONTROL;
        if ((targetID >= TARGETID_A) && (targetID <= TARGETID__))
                targettype=TARGETTYPE_CHARACTER;
        if ((targetID >= TARGETID_ABCDEFGHI) && (targetID <= TARGETID_YZ_))
                targettype=TARGETTYPE_CHARACTERS;
        cur_target->targettype=targettype;
        targets->Add(cur_target);
}//for

        return(1);


/*shidong ends*/
/*
char    buf[256], line[256];
FILE    *fp;
int     targetID, parentID, displaypos, ptr, targettype;
TARGET  *cur_target;

 // if we already have a list of potential targets, delete this list
 if (targets) delete targets;
 targets=new TARGETLIST();

 // read the target definition file
 fp=fopen(targetdeffilename, "rb");
 if (!fp) return(0);


 while (!feof(fp))
  {
  fgets(line, 255, fp);
  if (strlen(line) > 2)
     {
     ptr=0;
     // first column ... target code
     ptr=get_argument(ptr, buf, line, 255);
     targetID=atoi(buf);
     cur_target=new TARGET(targetID);
     // second column ... caption
     ptr=get_argument(ptr, buf, line, 255);
     cur_target->Caption=AnsiString(buf).Trim();
     // third column ... icon
     ptr=get_argument(ptr, buf, line, 255);
     cur_target->IconFile=AnsiString(buf).Trim();
     targettype=TARGETTYPE_NOTYPE;
     if ((targetID == TARGETID_BLANK) || (targetID == TARGETID_BACKUP) || (targetID == TARGETID_ROOT))
        targettype=TARGETTYPE_CONTROL;
     if ((targetID >= TARGETID_A) && (targetID <= TARGETID__))
        targettype=TARGETTYPE_CHARACTER;
     if ((targetID >= TARGETID_ABCDEFGHI) && (targetID <= TARGETID_YZ_))
        targettype=TARGETTYPE_CHARACTERS;
     cur_target->targettype=targettype;
     targets->Add(cur_target);
     }
  }
 fclose(fp);
*/


}//LoadPotentialTargets

// gets new targets, given a certain rootID
// i.e., the targetID of the last selected target
// parentID==TARGETID_ROOT on start
TARGETLIST *TRIALSEQUENCE::GetActiveTargets()
{
TARGETLIST      *new_list;
TARGET          *target, *new_target;;
int             targetID;

 new_list=new TARGETLIST;                                               // the list of active targets
 new_list->parentID=0;
 /*shidong starts*/
 for (targetID=1; targetID<=NumMatrixColumns * NumMatrixRows; targetID++)
 /*shidong ends*/
  {
  target=targets->GetTargetPtr(targetID);
  if ((targetID >= 0) && (target))
     {
     // add to active targets
     new_target=target->CloneTarget();
     new_target->Color=clYellow;
     new_target->TextColor=TextColor;
     new_target->parentID=0;
     new_target->targetposition=targetID;
     new_target->targetID=targetID;
     new_list->Add(new_target);
     }
   }

 return(new_list);
}


// **************************************************************************
// Function:   get_argument
// Purpose:    parses a line
//             it returns the next token that is being delimited by a ";"
// Parameters: ptr - index into the line of where to start
//             buf - destination buffer for the token
//             line - the whole line
//             maxlen - maximum length of the line
// Returns:    the index into the line where the returned token ends
// **************************************************************************
int TRIALSEQUENCE::get_argument(int ptr, char *buf, const char *line, int maxlen) const
{
 // skip one preceding semicolon, if there is any
 if ((line[ptr] == ';') && (ptr < maxlen))
    ptr++;

 // go through the string, until we either hit a semicolon, or are at the end
 while ((line[ptr] != ';') && (line[ptr] != '\n') && (line[ptr] != '\r') && (ptr < maxlen))
  {
  *buf=line[ptr];
  ptr++;
  buf++;
  }

 *buf=0;
 return(ptr);
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

 cur_trialsequence=0;
 cur_on=false;

 selectedtarget=NULL;
 cur_stimuluscode=0;

 // initialize block randomized numbers
 InitializeBlockRandomizedNumber();

 statevector->SetStateValue("StimulusCode", 0);
 statevector->SetStateValue("StimulusType", 0);
}


// **************************************************************************
// Function:   GetRandomStimulusCode
// Purpose:    Resets the trial's sequence to the beginning of the ITI
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
int TRIALSEQUENCE::GetRandomStimulusCode()
{
int     num;

 num=GetBlockRandomizedNumber(NUM_STIMULI);

return(num);
}


// **************************************************************************
// Function:   SuspendTrial
// Purpose:    Turn off display when trial gets suspended
//             also, turn off all the states so that they are not 'carried over' to the next run
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TRIALSEQUENCE::SuspendTrial()
{
 userdisplay->HideActiveTargets();           // hide all active targets
 userdisplay->HideStatusBar();               // hide the status bar

 statevector->SetStateValue("StimulusCode", 0);
 statevector->SetStateValue("StimulusType", 0);
 statevector->SetStateValue("Flashing", 0);

 userdisplay->DisplayMessage( const_cast<char*>( LocalizableString( "TIME OUT !!!" ) ) ); // display the "TIME OUT" message
}


// this (dis)intensifies a row/column and returns
// 1 if the character to spell is within this row/column
short TRIALSEQUENCE::IntensifyTargets(int stimuluscode, bool intensify)
{
TARGET  *cur_target, *chartospellptr;
int     i, cur_row, cur_targetID, chartospellID;
short   thisisit;

 // find the ID of the character that we currently want to spell
 chartospellptr=userdisplay->activetargets->GetTargetPtr(chartospell);
 if (chartospellptr)
    chartospellID=chartospellptr->targetID;
 else
    chartospellID=0;
 thisisit=0;

 // do we want to flash a column (stimuluscode 1..NumMatrixColumns) ?
 /*shidong starts*/
 if ((stimuluscode > 0) && (stimuluscode <= NumMatrixColumns))
    {
    for (i=0; i<userdisplay->activetargets->GetNumTargets(); i++)
     {
     /*shidong starts*/
     //if (i%6+1 == stimuluscode)
     if (i%NumMatrixColumns + 1 == stimuluscode)
     /*shidong ends*/
        {
        cur_target=userdisplay->activetargets->GetTargetPtr(i+1);
        if (intensify)
           cur_target->SetTextColor(TextColorIntensified);
        else
           cur_target->SetTextColor(TextColor);
        if ((i+1 == chartospellID) && (intensify))
           thisisit=1;
        }
     }
    }

 // do we want to flash a row (stimuluscode NumMatrixColumns..NUM_STIMULI) ?
 if ((stimuluscode > NumMatrixColumns) && (stimuluscode <= NUM_STIMULI ))
    {
    /*shidong starts*/
    for (i=0; i<NumMatrixColumns; i++)
     {
//     cur_row=stimuluscode-6;
  //   cur_targetID=(cur_row-1)*6+i+1;
        cur_row=stimuluscode-NumMatrixColumns;
        cur_targetID=(cur_row-1)*NumMatrixColumns+i+1;
     cur_target=userdisplay->activetargets->GetTargetPtr(cur_targetID);
     if (intensify)
        cur_target->SetTextColor(TextColorIntensified);
     else
        cur_target->SetTextColor(TextColor);
     if ((cur_targetID == chartospellID) && (intensify))
        thisisit=1;
     }
     /*shidong ends*/
    }
 /*shidong ends*/
 return(thisisit);
}


// **************************************************************************
// Function:   GetReadyForTrial
// Purpose:    Show matrix, etc.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TRIALSEQUENCE::GetReadyForTrial()
{
 userdisplay->HideMessage();                 // hide any message that's there
 userdisplay->DisplayStatusBar();
 userdisplay->DisplayActiveTargets();           // display all active targets
}


// **************************************************************************
// Function:   SetUserDisplayTexts
// Purpose:    Set goal and result text on userdisplay
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void TRIALSEQUENCE::SetUserDisplayTexts()
{
 // if we are in offline mode, then ...
 if (!onlinemode)
    {
    chartospell=TextToSpell.SubString(char2spellidx, 1);
    userdisplay->statusbar->goaltext=TextToSpell+" ("+chartospell+")";
    }
 else // if we are in online mode
    {
        //userdisplay->statusbar->goaltext="";
    }
}



// **************************************************************************
// Function:   Process
// Purpose:    Processes the control signal provided by the task
// Parameters: controlsignal - pointer to the vector of control signals
// Returns:    pointer to the selected target (if one was selected), or NULL
// **************************************************************************
int TRIALSEQUENCE::Process(const std::vector<float>& controlsignal)
{
unsigned short running, within;
int     ret;
    /*shidong starts*/
    if (debug) fprintf( f, "The  cur_stimuluscode is %d.\t", cur_stimuluscode);
    /*shidong ends*/
 running=statevector->GetStateValue("Running");

 // when we suspend the system, show the "TIME OUT" message
 if ((running == 0) && (oldrunning == 1))
    {
    SuspendTrial();
    oldrunning=0;
    }

 // when we just started with this character,
 // then show the matrix and reset the trial sequence
 if ((running == 1) && (oldrunning == 0))
    {
    ResetTrialSequence();
    GetReadyForTrial();
    }

 // are we at the end of the intensification period ?
 // if yes, turn off the row/column
 if (cur_on && (cur_trialsequence == ontime))
    {
    cur_on=false;
    cur_trialsequence=0;
    IntensifyTargets(cur_stimuluscode, false);
    cur_stimuluscode=0;
    statevector->SetStateValue("StimulusType", 0);
    statevector->SetStateValue("Flashing", 0);
    ret=1;
    }

 // are we at the end of the "dis-intensification" period ?
 // if yes, intensify the next row/column
 if (!cur_on && (cur_trialsequence == offtime))
    {
    cur_on=true;
    cur_trialsequence=0;
    cur_stimuluscode=GetRandomStimulusCode();
    /*shidong starts*/
    if (debug) fprintf(f,  "The  cur_stimuluscode is %d.\n", cur_stimuluscode);
    /*shidong ends*/
    within=IntensifyTargets(cur_stimuluscode, true);
    // in the online mode, we do not know whether this is standard or oddball
    if (onlinemode)
       statevector->SetStateValue("StimulusType", 0);
    else
       statevector->SetStateValue("StimulusType", within);
    statevector->SetStateValue("Flashing", 1);
    }

 statevector->SetStateValue("StimulusCode", cur_stimuluscode);

 oldrunning=running;
 cur_trialsequence++;
 return(ret);
}

