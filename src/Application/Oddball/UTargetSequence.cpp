//---------------------------------------------------------------------------

#include "PCHIncludes.h"
#pragma hdrstop

#include <stdio.h>

#include "UTargetSequence.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


TARGETSEQUENCE::TARGETSEQUENCE()
: targets( new TARGETLIST() )
{
  BEGIN_PARAMETER_DEFINITIONS
   "Oddball string TargetDefinitionFile= targets.cfg % % % "
     "// Target definition file (inputfile)",
   "Oddball int OddballProbability= 10 10 0 100 "
     "// Probability for oddball icon",
  END_PARAMETER_DEFINITIONS
}


TARGETSEQUENCE::~TARGETSEQUENCE()
{
 if (targets)           delete targets;
 targets=NULL;
}


int TARGETSEQUENCE::Initialize()
{
int ret;

 // load and create all potential targets
 ret=LoadPotentialTargets(Parameter("TargetDefinitionFile"));
 probability=Parameter("OddballProbability");

 return(ret);
}


int TARGETSEQUENCE::LoadPotentialTargets(const char *targetdeffilename)
{
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

 return(1);
}


// gets new targets, given a certain rootID
// i.e., the targetID of the last selected target
// parentID==TARGETID_ROOT on start
TARGETLIST *TARGETSEQUENCE::GetActiveTargets()
{
TARGETLIST      *new_list;
TARGET          *target, *new_target;;
int             targetID;

 new_list=new TARGETLIST;                                               // the list of active targets
 new_list->parentID=0;

 if (rand() % 100 > probability)
    targetID=1;
 else
    targetID=2;

 target=targets->GetTargetPtr(targetID);
 if ((targetID >= 0) && (target))
    {
    // add to active targets
    new_target=target->CloneTarget();
    new_target->Color=clYellow;
    // new_target->TextColor=clGreen;
    new_target->TextColor=clBlack;
    new_target->parentID=0;
    new_target->targetposition=0;
    new_list->Add(new_target);
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

