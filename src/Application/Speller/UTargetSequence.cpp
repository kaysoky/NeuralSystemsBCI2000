#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include <stdio.h>

#include "UTargetSequence.h"

#include "UBCIError.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)


TARGETSEQUENCE::TARGETSEQUENCE(PARAMLIST *plist, STATELIST *slist)
{

/*shidong starts*/

debug = false;
if(debug)f = fopen("MuSpellerDebug.txt", "w");

/*shidong starts*/
char    line[512];
int     i;

 targets=new TARGETLIST();
 tree=new TREE();
 dictionary=new DICTIONARY();

 strcpy(line, "Speller int EnablePrediction= 1 0 0 1 // Enable word prediction (0=no, 1=yes)");
 plist->AddParameter2List(line,strlen(line));
 strcpy(line, "Speller string DictionaryFile= odgenswords.voc 0 0 100 // Dictionary file for word prediction");
 plist->AddParameter2List(line,strlen(line));
 /*shidong starts*/                 
 strcpy(line, "Speller matrix TargetDefinitionMatrix = 41 { ID Type Display FontSizeFactor IconFile} //Target Definition Matrix");
 plist->AddParameter2List(line,strlen(line));
 strcpy(line, "Speller matrix TreeDefinitionMatrix= 39 39 {ParentID DisplayPosition TargetID   } -// Tree Definition Matrix");

 /*shidong ends*/
 plist->AddParameter2List(line,strlen(line));

 activetargets_historynum=0;
 text_historynum=0;
 for (i=0; i<MAX_TARGETHISTORY; i++)
  activetargets_history[i]=NULL;
}


TARGETSEQUENCE::~TARGETSEQUENCE()
{
 if (targets)           delete targets;
 if (tree)              delete tree;
 if (dictionary)        delete dictionary;
 /*shidong starts*/
if(debug)  fclose(f);
 /*shidong ends*/

 targets=NULL;
 tree=NULL;
 dictionary=NULL;

 DeleteHistory();
}


int TARGETSEQUENCE::Initialize(PARAMLIST *plist, int numT )
{
int ret;

 // delete the history of all previous active targets
 DeleteHistory();

 // Load the dictionary file
 try {
  if (atoi(plist->GetParamPtr("EnablePrediction")->GetValue()) == 0)
     prediction=false;
  else
     prediction=true;

  // load and create all potential targets
  /*shidong starts*/
  NUM_TARGETS = numT;
  ret=LoadPotentialTargets(   plist  );
  /*plist->GetParamPtr("TargetDefinitionMatrix")->GetNumColumns(),
  plist->GetParamPtr("TargetDefinitionMatrix")->GetNumRows(),
  plist->GetParamPtr("TreeDefinitionMatrix")->GetNumColumns(),
  plist->GetParamPtr("TreeDefinitionMatrix")->GetNumRows()   */

        if(debug) fprintf(f, "After LoadPotentialTargets, ret is %d.\n", ret);

  // possibly replace this with something better
  if (ret == 0)  Application->MessageBox("Could not load target definition matrix.", "Error", MB_OK);
  if (ret == -1) Application->MessageBox("Could not load tree definition matrix.", "Error", MB_OK);
  if (ret <= 0)  return(0);

        if(debug) fprintf(f, "After App->msgBox, ret is %d.\n", ret);

    /*shidong ends*/
 ret=dictionary->LoadDictionary(plist->GetParamPtr("DictionaryFile")->GetValue(), true);
        if(debug) fprintf(f, "After LoadDictionary, ret is %d, dic file is %s.\n", ret, plist->GetParamPtr("TargetDefinitionMatrix")->GetValue() );
  // if there was no error, add the dictionary to the potential targets
  if (ret == 1)
     AddDictionary2PotentialTargets();  /* */
} catch(...) {ret=0;}

 // possibly replace this with something better
 if (ret == 0) Application->MessageBox("Could not find dictionary file. Wrong directory ?", "Error", MB_OK);

 return(ret);
}


// delete the history of both the targets and the result text
void TARGETSEQUENCE::DeleteHistory()
{
int     i;

 // delete the history of targets
 for (i=0; i<MAX_TARGETHISTORY; i++)
  if (activetargets_history[i])
     {
     delete activetargets_history[i];
     activetargets_history[i]=NULL;
     }

 activetargets_historynum=0;
 text_historynum=0;
}


//int TARGETSEQUENCE::LoadPotentialTargets(const int targetRow, const int targetCol, const int treeRow, const int treeCol)
int TARGETSEQUENCE::LoadPotentialTargets(PARAMLIST *plist)
{
/*shidong starts */
 int    targetID, parentID, displaypos, ptr, targettype;
 TARGET *cur_target;
 


 // if we already have a list of potential targets, delete this list
 if (targets) delete targets;
 targets=new TARGETLIST();
 if(debug)fprintf(f, "Row # is %d.\n", plist->GetParamPtr("TargetDefinitionMatrix")->GetNumRows());
 if(debug)fprintf(f, "ID \tType \tDisplay\tFontSizeFactor\tIconFile\n");



/* */


 // parse the target definition matrix
 for (int i = 0; i < plist->GetParamPtr("TargetDefinitionMatrix")->GetNumRows(); i++)
 {
  targetID =
        AnsiString((const char*)plist->GetParamPtr("TargetDefinitionMatrix")->GetValue(i,0)).ToInt();
 cur_target = new TARGET(targetID);
 cur_target->targettype =
        AnsiString((const char*)plist->GetParamPtr("TargetDefinitionMatrix")->GetValue(i,1)).ToInt();
 cur_target->Caption = AnsiString((const char*)plist->GetParamPtr("TargetDefinitionMatrix")->GetValue(i,2));
 cur_target->FontSizeFactor = (float)(atof(plist->GetParamPtr("TargetDefinitionMatrix")->GetValue(i,3)));
  cur_target->IconFile =
 AnsiString((const char*)plist->GetParamPtr("TargetDefinitionMatrix")->GetValue(i,4));
 if(debug)fprintf(f, "%d\t", cur_target->targetID);
 if(debug)fprintf(f, "%d\t", cur_target->targettype);
 if(debug)fprintf(f, "%s\t", cur_target->Caption);
  if(debug)fprintf(f, "%f\n", cur_target->FontSizeFactor);
 if(debug)fprintf(f, "%s\n", cur_target->IconFile);


 targets->Add(cur_target);
 }//for




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
 // load the tree file to go with the list of targets
 //if (tree->LoadTree(treeRow, treeCol) == 0)
 if (tree->LoadTree(plist) == 0)
    return(-1);
                 
 return(1);
 /*shidong ends*/
}


int TARGETSEQUENCE::AddDictionary2PotentialTargets()
{
TARGET  *cur_target;
int     i, cur_targetID;

 cur_targetID=targets->GetMaxTargetID()+1;

 // go through the dictionary and add the words to the potential targets
 for (i=0; i<dictionary->GetNumWords(); i++)
  {
  cur_target=new TARGET(cur_targetID);
  cur_target->Caption=AnsiString(dictionary->GetWord(i)).Trim();
  cur_target->targettype=TARGETTYPE_WORD;
  targets->Add(cur_target);
  cur_targetID++;
  }

 return(1);
}


// get these targets, in case there is word prediction
TARGETLIST *TARGETSEQUENCE::GetActiveTargetsPrediction(int cur_parentID, BYTE backuppos, const char *cur_prefix)
{
TARGETLIST *new_list;
int     targetID, i, nummatching;
TARGET  *new_target, *target;
BYTE    offset;
bool    *populated;

 nummatching=dictionary->GetNumMatching(cur_prefix, false);

 // sanity check
 if ((!cur_prefix) || (!prediction) || (nummatching == 0) || (nummatching >= NUM_TARGETS))
    return(NULL);

 new_list=new TARGETLIST;                                               // the list of active targets
 new_list->parentID=cur_parentID;
 populated=new bool[NUM_TARGETS];
 for (i=0; i<NUM_TARGETS; i++)
  populated[i]=false;

 // if BACK-UP is on the top, then start populating farther down
 offset=0;
 /*shiodng starts*/
 //if (backuppos == 0) offset=NUM_TARGETS-nummatching;
 /*shiodng ends*/

 // set the mode of these targets to predictive mode
 new_list->predictionmode=MODE_PREDICTION;

 /*shidong starts*/
 /*
 // create the target for BACK UP
 targetID=TARGETID_BACKUP;
 target=targets->GetTargetPtr(targetID);
 new_target=target->CloneTarget();
 new_target->Color=clYellow;
 // new_target->TextColor=clGreen;
 new_target->TextColor=clBlack;
 new_target->parentID=cur_parentID;
 new_target->targetposition=backuppos;
 populated[new_target->targetposition]=true;            // mark that target as being occupied
 new_list->Add(new_target);

 /*shiodng ends*/
 // create the targets with the words in them
 for (i=0; i<nummatching; i++)
  {
  target=targets->GetTargetPtr(dictionary->GetMatchingWord(cur_prefix, false, i));
  new_target=target->CloneTarget();
  new_target->parentID=cur_parentID;
  new_target->Color=clYellow;
  // new_target->TextColor=clGreen;
        /*shidong starts*/
        new_target->targettype= target->targettype;
        /*shidong ends*/
  new_target->TextColor=clBlack;
  new_target->targetposition=offset+(BYTE)i;
  populated[new_target->targetposition]=true;           // mark that target as being occupied
  new_list->Add(new_target);
  }

 // populate the "missing" targets with blank targets
 for (i=0; i<NUM_TARGETS; i++)
  {
  // if that target population is not populated ...
  if (!populated[i])
     {
     // create a blank target
     targetID=TARGETID_BLANK;
     target=targets->GetTargetPtr(targetID);
     new_target=target->CloneTarget();
     new_target->parentID=cur_parentID;
     new_target->Color=clYellow;
     /*shidong starts*/
     new_target->targettype = target->targettype;
     /*shidong ends*/
     // new_target->TextColor=clGreen;
     new_target->TextColor=clBlack;
     new_target->targetposition=i;
     new_list->Add(new_target);
     }
  }

 delete [] populated;
 return(new_list);
}


// gets new targets, given a certain rootID
// i.e., the targetID of the last selected target
// parentID==TARGETID_ROOT on start
TARGETLIST *TARGETSEQUENCE::GetActiveTargets(int cur_parentID,  const char *cur_prefix)
{
// BYTE backuppos
TARGETLIST *new_list;
int     targetID, displaypos, count, nummatching;
TARGET  *new_target, *target;
BYTE    offset;
bool    fullypopulated;                 // specifies whether or not there is a target for every display position. if not, then we shuffle backup around in a funky fashion

/*shidong starts*/
int     backuppos;                      //position of the "Back-Up"



  
/*shidong ends*/


 // include prediction, if we specified a prefix and we enabled prediction, and
 // if the number of possible words with this prefix fits in the free targets
 if ((cur_prefix) && (prediction))
    {
    nummatching=dictionary->GetNumMatching(cur_prefix, false);
    if ((nummatching > 0) && (nummatching < NUM_TARGETS))
       return(GetActiveTargetsPrediction(cur_parentID, backuppos,  cur_prefix));
    }

 new_list=new TARGETLIST;                                               // the list of active targets
 new_list->parentID=cur_parentID;

 // set the mode of these targets to normal mode, i.e., no prediction
 new_list->predictionmode=MODE_NORMAL;

 // if BACK-UP is on the top, shift everything down by one
 offset=0;
 /*shidong starts*/
 //if (backuppos == 0) offset=1;
 /*shidong ends*/

 // in the tree, the first three displaypositions are reserved for the actual targets
 // the last displayposition is not in the tree, but will be dynamically assigned here for BACKUP
 // the logic is a little strange, please forgive

 // count the number of targets on the screen
 // if there is a target defined for each display position, no need to shuffle BACKUP around, etc.
 count=0;
 fullypopulated=false;
 for (displaypos=0; displaypos<NUM_TARGETS; displaypos++)
  {
  targetID=tree->DetermineTargetID(cur_parentID, displaypos);

  //if(debug) fprintf(f, "in loop, count is %d, cur_parentID is %d, displaypos is %d, and id is %d.\n", count,cur_parentID, displaypos, targetID);

  if (targetID != TARGETID_NOID) count++;
  }
 if(debug) fprintf(f,"count is %d. \n", count);
 if (count == NUM_TARGETS)
    {
    fullypopulated=true;
    offset=0;
    }
 if(debug) fprintf(f,"\nB4 reset count is %d. targetID is %d, fully is %d.\n", count, targetID, fullypopulated);

 // if there is at least one target (as defined by the tree) on the screen, populate the targets
 if (count > 0)
    {
    count=0;
    for (displaypos=0; displaypos<NUM_TARGETS; displaypos++)
     {
     // if every display position on the screen is filled with a target, then just get that target ID
     if (fullypopulated)
        {
        if(debug) fprintf(f,"The list is fully populated.\n");
        targetID=tree->DetermineTargetID(cur_parentID, displaypos);
        }
     else
        {
                if(debug) fprintf(f,"The list is NOT fully populated.\n");

                if ((displaypos == NUM_TARGETS-1) && (activetargets_historynum == 0))     // if we are at root and we haven't spelled anything (i.e., there is no history), then use a blank target
                {
                        targetID=TARGETID_BLANK;
                }
                else                                                                      // otherwise use the target determined by the tree or determine BACK UP
                {
                if (displaypos == NUM_TARGETS-1)
                {
                        // if we are at the last position, then we should have had NUM_TARGETS-1
                        // if not, then we might have selected a dummy target that does not lead to anything in the tree
                        if (count == NUM_TARGETS-1)
                        targetID=TARGETID_BACKUP;
                        else
                        targetID=TARGETID_NOID;
                }
                else
                        targetID=tree->DetermineTargetID(cur_parentID, displaypos);
                }
        }//else the list is not fully populated
//if(debug) fprintf(f,"B4 safety check, count is %d. targetID is %d.\n", count, targetID);        
        
     // safety check ...
     if (targetID == TARGETID_NOID) targetID=TARGETID_BLANK;
     target=targets->GetTargetPtr(targetID);
     if ((targetID >= 0) && (target))
        {
        // add to active targets
        new_target=target->CloneTarget();
        new_target->Color=clYellow;
        // new_target->TextColor=clGreen;
        new_target->TextColor=clBlack;
        new_target->parentID=cur_parentID;
        /*shidong starts*/
        new_target->targettype= target->targettype;
        /*shidong ends*/
        // if the current targetID is "BLANK" or "BACKUP" choose the defined displayposition
        // otherwise, choose the displayposition stored in the tree, offset by whether or not BACKUP is the first or last target
        /*shidong starts*/
        /*
        if ((targetID == TARGETID_BLANK) || (targetID == TARGETID_BACKUP))
           new_target->targetposition=backuppos;
        else */
           new_target->targetposition=(BYTE)displaypos+offset;
        if(debug) fprintf(f, "The new target's ID is %d, position is %d.\n", new_target->targetID, new_target->targetposition);

        /*shidong ends*/

        new_list->Add(new_target);
        count++;
        }
        if(debug) fprintf(f, "count is %d, and curTargetID is %d.\n", count, targetID);
        if(debug) fprintf(f, "list has %d targets, last target's ID is %d.\n\n", new_list->GetNumTargets(), new_target->targetID);
     } //for
    } //if

 // if, for some reason, the lists contain no target, then display the root targets
 // and delete the (empty) lists of current targets and in history
 if (new_list->GetNumTargets() == 0)
    {
    if(debug) fprintf(f, "This should never be called.\n");

    delete new_list;
    new_list=GetActiveTargets(TARGETID_ROOT, NULL);
    }
 if(debug) fprintf(f, "This Active Target List has %d targets.\n", new_list->GetNumTargets());
 return(new_list);
}


void TARGETSEQUENCE::PushTargetsOnHistory(TARGETLIST *activetargets)
{
TARGET  *target;
int     i, targetID;


  if(debug) fprintf(f, "In pushTargetHistory, The push Target List has %d targets.\n", activetargets->GetNumTargets());
  if(debug) fprintf(f, "In PushTargetsOnHistory, before push activetargets_historynum is  %d.\n", activetargets_historynum);

 // don't do anything if the list of active targets does not contain a single target
 if (activetargets->GetNumTargets() == 0)
    return;

 activetargets_history[activetargets_historynum]=new TARGETLIST;        // add the same thing to the history of previous target lists
 activetargets_history[activetargets_historynum]->predictionmode=activetargets->predictionmode;
 for (i=0; i<activetargets->GetNumTargets(); i++)
  {
  targetID=activetargets->GetTargetID(i);
  target=activetargets->GetTargetPtr(targetID, i);

  /*shidong starts*/
        if(debug) fprintf(f, "IN LOOP,  current dsiplaypos is %d, and the target's ID is %d.\n", i, target->targetID);
  
  TARGET *toAdd;
  toAdd = target->CloneTarget();
  toAdd->targettype = target->targettype;

  // add also to target history so that we can back up
  // only if, of course, we don't run outa space
  if (activetargets_historynum < MAX_TARGETHISTORY-1)
     activetargets_history[activetargets_historynum]->Add(toAdd);
  }        /*shidong ends*/
 activetargets_historynum++;
 if(debug) fprintf(f, "In PushTargetsOnHistory, after push activetargets_historynum is  %d.\n", activetargets_historynum);

}


void TARGETSEQUENCE::PushTextOnHistory(AnsiString text)
{
 if (text_historynum < MAX_TEXTHISTORY-1)
    {
    text_history[text_historynum]=text;
    text_historynum++;
    }
}


// retrieves previous targets
// in case we are already at the very start (in order to prevent crashing)
// just return the target list for the rootID
TARGETLIST *TARGETSEQUENCE::GetPreviousTargets()
{
TARGETLIST *ptr;

  if(debug) fprintf(f, "In GetPreviousTargets, activetargets_historynum is  %d.\n", activetargets_historynum);

 // we are at the start already
 if (activetargets_historynum == 0)
    return(GetActiveTargets(TARGETID_ROOT, NULL));

 // otherwise, return the previous list
 activetargets_historynum--;
 ptr=activetargets_history[activetargets_historynum];
 activetargets_history[activetargets_historynum]=NULL;          // 'release' this list of targets; active targets will be freed at some other place


  if(debug) fprintf(f, "In GetPreviousTargets, After decrease, ptr has %d targets.\n", ptr->GetNumTargets());

  if(debug) fprintf(f, "In GetPreviousTargets, After decrease, activetargets_historynum is  %d.\n", activetargets_historynum);

 return(ptr);
}


// retrieves previous text
// in case we are already at the very start (in order to prevent crashing)
// (we shouldn't be able to back up from the first step anyway)
// just return " "
AnsiString TARGETSEQUENCE::GetPreviousText()
{
 // we are at the start already
 if (text_historynum == 0)
    return("");

 // otherwise, return the previous text
 text_historynum--;
 return(text_history[text_historynum]);
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
int TARGETSEQUENCE::get_argument(int ptr, char *buf, const char *line, int maxlen) const
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
