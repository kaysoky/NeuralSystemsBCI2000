#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------


#include <stdio.h>
#include <string.h>

#include "UParameter.h"
#include "UState.h"
#include "UTrialSequence.h"
#include "UTaskUtil.h"
#include "UBCIError.h"
#include "Localization.h"
#include "MeasurementUnits.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


// **************************************************************************
// Function:   TRIALSEQUENCE
// Purpose:    This is the constructor of the TRIALSEQUENCE class
// Parameters: plist - pointer to the paramter list
//             slist - pointer to the state list
// Returns:    N/A
// **************************************************************************
TRIALSEQUENCE::TRIALSEQUENCE()
: vis( NULL ),
  targets( new TARGETLIST )
{

 /*shidong starts*/
 f = NULL;
 debug = false;
 if(debug) f = fopen("debug2.txt", "w");

 BEGIN_PARAMETER_DEFINITIONS
   "P3Speller matrix TargetDefinitionMatrix= "
      "36 "
      "{Display Enter Display%20Size Icon%20File Sound%20File} "
      "A A 1 % % "  "B B 1 % % "  "C C 1 % % "  "D D 1 % % "  "E E 1 % % "  "F F 1 % % "
      "G G 1 % % "  "H H 1 % % "  "I I 1 % % "  "J J 1 % % "  "K K 1 % % "  "L L 1 % % "
      "M M 1 % % "  "N N 1 % % "  "O O 1 % % "  "P P 1 % % "  "Q Q 1 % % "  "R R 1 % % "
      "S S 1 % % "  "T T 1 % % "  "U U 1 % % "  "V V 1 % % "  "W W 1 % % "  "X X 1 % % "
      "Y Y 1 % % "  "Z Z 1 % % "  "1 1 1 % % "  "2 2 1 % % "  "3 3 1 % % "  "4 4 1 % % "
      "5 5 1 % % "  "6 6 1 % % "  "7 7 1 % % "  "8 8 1 % % "  "9 9 1 % % "  "_ _ 1 % % "
      "% % % // Target Definition Matrix",
   "P3Speller int NumMatrixColumns= 6 "
      "6 0 6 // Display Matrix's Column Number",
   "P3Speller int NumMatrixRows= 6 "
      "6 0 6 // Display Matrix's Row Number",
 /*shidong ends*/

  "P3Speller int OnTime= 4 "
    "10 0 5000 // Duration of intensification in units of SampleBlocks",
  "P3Speller int OffTime= 1 "
    "10 0 5000 // Interval between intensification in units of SampleBlocks",
  "P3Speller int OnlineMode= 0 "
    "0 0 1 // Online mode (0=no, 1=yes) (boolean)",
  "P3Speller int ResultDisplay= 1 "
    "0 0 1 // Display results (0=no, 1=yes) (boolean)",
  "P3Speller string TextColor= 0x00000000 "
    "0x00505050 0x00000000 0x00000000 // Text Color (color)",
  "P3Speller string TextColorIntensified= 0x000000FF "
    "0x00505050 0x00000000 0x00000000 // Intensified Text Color (color)",
  "P3Speller string TextToSpell= P "
    "P A Z // Character or string to spell in offline mode",
    /* VK added for icon highlighting */
  "P3Speller int IconHighlight= 1 "
    "1 0 2 // Icon highlight method 0: GRAYSCALE, 1: INVERT, 2: DARKEN (enumeration)",
  "P3Speller int HighlightFactor= 2 "
    "2 1 5 // Scale factor to reduce icon pixel values",
 END_PARAMETER_DEFINITIONS

 BEGIN_STATE_DEFINITIONS
  "StimulusCode 5 0 0 0",
  "StimulusType 3 0 0 0",
  "Flashing 1 0 0 0",
 END_STATE_DEFINITIONS
}


// **************************************************************************
// Function:   ~TRIALSEQUENCE
// Purpose:    This is the destructor of the TRIALSEQUENCE class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TRIALSEQUENCE::~TRIALSEQUENCE()
{
 delete vis;
 delete targets;
 /*shidong starts*/
 if( f )
   fclose( f );        
 /*shidong ends*/
}
// **************************************************************************
// Function:   Preflight
// Purpose:    Added to check for presence of icon and wave files
// **************************************************************************

// VK Adding to verify existence of files
void TRIALSEQUENCE::Preflight(const SignalProperties& inputProperties,
                        SignalProperties& outputProperties ) const
{
  int ret, row, col;
  AnsiString iFileName, sFileName;
  TImage *temp_icon;
  TWavePlayer testPlayer;
  bool soundFlag = false;

  row = Parameter("TargetDefinitionMatrix")->GetNumRows();
  col =  Parameter("TargetDefinitionMatrix")->GetNumColumns();
  if (col != 5)
    bciout << "P3Speller: Target Definition Matrix should have 5 columns!" ;
    
  // parse Target Definition Matrix for icon and sound file names.
  for (int i = 0; i<row; i++)
  {
    iFileName = "";
    sFileName = "";
    iFileName = AnsiString((const char*)Parameter("TargetDefinitionMatrix", i, 3));
    sFileName = AnsiString((const char*)Parameter("TargetDefinitionMatrix", i, 4));
    if(iFileName != "" && iFileName != " ")
    {
      temp_icon = new TImage(static_cast<TComponent*>(NULL));
      try
      {
      temp_icon->Picture->LoadFromFile(iFileName);
      }
      catch(...)
      {
        bcierr << "P3Speller: Could not open icon file - "
               << iFileName.c_str() << std::endl;
      }
      delete temp_icon;
    }
    if(sFileName != "" && sFileName != " ")
    {
      if(!soundFlag)
        soundFlag = true;
      TWavePlayer::Error err = testPlayer.AttachFile( sFileName.c_str() );
      if( err == TWavePlayer::fileOpeningError )
        bcierr << "P3Speller: Could not open sound file - "
               << sFileName.c_str() << std::endl;
      else if( err != TWavePlayer::noError )
        bcierr << "P3Speller: Some general error prevents wave audio playback"
               << std::endl;
    }
  }
  if(soundFlag)       //  need to play audio, check for sound card
  {
    if(waveOutGetNumDevs() <= 0)
      bcierr << "P3Speller: Sound Card not found " << std::endl;
  }

  if (Parameter("TextWindowEnabled") == 1)
  {
    if(Parameter("OnlineMode") == 0)
      bcierr << "P3Speller: Cannot open text window in offline mode!"  << std::endl;

    if(AnsiString((const char*)Parameter("TextWindowFilePath")) == "")
      bcierr << "P3Speller: Please enter path for saving text window files!"  << std::endl;
  }
  outputProperties = inputProperties;
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
int TRIALSEQUENCE::Initialize(USERDISPLAY *new_userdisplay)
{
int     ret;

 // load and create all potential targets
 /*shidong starts*/
 ret=LoadPotentialTargets(Parameter("TargetDefinitionMatrix")->GetNumColumns(), Parameter("TargetDefinitionMatrix")->GetNumRows());
 NumMatrixColumns = Parameter("NumMatrixColumns");
 NumMatrixRows = Parameter("NumMatrixRows");
 NUM_STIMULI = NumMatrixColumns + NumMatrixRows;
 /*shidong ends*/
 if (ret == 0)
    {
    bcierr << "P3 Speller: Could not open target definition file" << std::endl;
    return(0);
    }

 userdisplay=new_userdisplay;

 // if (vis) delete vis;
 // vis= new GenericVisualization( plist, corecomm );
 // vis->SetSourceID(SOURCEID_SPELLERTRIALSEQ);
 // vis->SendCfg2Operator(SOURCEID_SPELLERTRIALSEQ, CFGID_WINDOWTITLE, "Speller Trial Sequence");

 ret=1;
 ontime = MeasurementUnits::ReadAsTime( Parameter( "OnTime" ) );
 offtime = MeasurementUnits::ReadAsTime( Parameter( "OffTime" ) );
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
 userdisplay->DisplayMessage( LocalizableString( "Waiting to start ..." ) );

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
cur_target->IconHighlightFactor=2;       // VK
cur_target->IconHighlightMethod="";
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
        // VK: add icon file + sound file
        cur_target->IconFile = AnsiString((const char*)Parameter("TargetDefinitionMatrix", i, 3));
        cur_target->SoundFile = AnsiString((const char*)Parameter("TargetDefinitionMatrix", i, 4));

        // VK: Parameters for testing icon highlighting
        if (Parameter("IconHighlight") == 0)
          cur_target->IconHighlightMethod = AnsiString("GRAYSCALE");
        else if (Parameter("IconHighlight") == 1)
          cur_target->IconHighlightMethod = AnsiString("INVERT");
        else if (Parameter("IconHighlight") == 2)
          cur_target->IconHighlightMethod = AnsiString("DARKEN");
        cur_target->IconHighlightFactor = (float)Parameter("HighlightFactor");

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

 State("StimulusCode")=0;
 State("StimulusType")=0;
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

 // VK  - enable this statement if text window is to be closed on suspend
 // userdisplay->DisableTextWindow();

 State("StimulusCode")=0;
 State("StimulusType")=0;
 State("Flashing")=0;

 userdisplay->DisplayMessage( LocalizableString( "TIME OUT !!!" ) ); // display the "TIME OUT" message
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
        //VK
        cur_target->HighlightIcon(intensify);
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
     //VK
     cur_target->HighlightIcon(intensify);     
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
int TRIALSEQUENCE::Process(const GenericSignal* controlsignal)
{
unsigned short running, within;
int     ret;
    /*shidong starts*/
    if (debug) fprintf( f, "The  cur_stimuluscode is %d.\t", cur_stimuluscode);
    /*shidong ends*/
 running=State("Running");

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
    State("StimulusType")=0;
    State("Flashing")=0;
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
       State("StimulusType")=0;
    else
       State("StimulusType")=within;
    State("Flashing")=1;
    }

 State("StimulusCode")=cur_stimuluscode;

 oldrunning=running;
 cur_trialsequence++;
 return(ret);
}

