//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "UTarget.h"
#include "UTree.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


// **************************************************************************
// Function:   TREE
// Purpose:    This is the constructor for the TREE class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TREE::TREE()
{
 treesize=0;
}


// **************************************************************************
// Function:   ~TREE
// Purpose:    This is the destructor for the TREE class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
TREE::~TREE()
{
}


// **************************************************************************
// Function:   get_argument
// Purpose:    parses the parameter line
//             it returns the next token that is being delimited by either
//             a ' ' or '='
// Parameters: ptr - index into the line of where to start
//             buf - destination buffer for the token
//             line - the whole line
//             maxlen - maximum length of the line
// Returns:    the index into the line where the returned token ends
// **************************************************************************
int TREE::get_argument(int ptr, char *buf, char *line, int maxlen)
{
 // skip trailing spaces, if any
 while ((line[ptr] == '=') || (line[ptr] == ' ') && (ptr < maxlen))
  ptr++;

 // go through the string, until we either hit a space, a '=', or are at the end
 while ((line[ptr] != '=') && (line[ptr] != ' ') && (line[ptr] != '\n') && (line[ptr] != '\r') && (ptr < maxlen))
  {
  *buf=line[ptr];
  ptr++;
  buf++;
  }

 *buf=0;
 return(ptr);
}


// **************************************************************************
// Function:   LoadTree
// Purpose:    This function loads a tree from a specified file
// Parameters: filename - pointer to the filename
// Returns:    0 - error (e.g., file not found)
//             1 - no error
// **************************************************************************
int TREE::LoadTree(char *filename)
{
char    line[256], buf[256];
FILE    *fp;
int     ptr;

 // read the tree definition file
 fp=fopen(filename, "rb");
 if (!fp) return(0);

 while (!feof(fp))
  {
  fgets(line, 255, fp);
  if (strlen(line) > 2)
     {
     ptr=0;
     // first column ... parent ID
     ptr=get_argument(ptr, buf, line, 255);
     parentID[treesize]=atoi(buf);
     // second column ... display position
     ptr=get_argument(ptr, buf, line, 255);
     displaypos[treesize]=atoi(buf)-1;
     // third column ... target ID
     ptr=get_argument(ptr, buf, line, 255);
     targetID[treesize]=atoi(buf);
     treesize++;
     }
  }
 fclose(fp);

 return(1);
}


// **************************************************************************
// Function:   DetermineTargetID
// Purpose:    Returns target ID for a given parent ID and display position
// Parameters: cur_parentID   - target ID of the parent of the subtree
//             cur_displaypos - screen position (or, branch number) of the child of cur_parentID (0..NUM_TARGETS-1)
// Returns:    TARGETID_NOID - there is no child for that branch and parent
//             all other - target ID of the target that is a child of ID cur_parentID and has the specified screen position
// **************************************************************************
int TREE::DetermineTargetID(int cur_parentID, int cur_displaypos)
{
int     i, ret;

 ret=TARGETID_NOID;
 for (i=0; i<treesize; i++)
  if (parentID[i] == cur_parentID)
     if (displaypos[i] == cur_displaypos)
        {
        ret=targetID[i];
        break;
        }

 return(ret);
}


// **************************************************************************
// Function:   DoesLeadTo
// Purpose:    determines whether the tree (given the current parent ID)
//             can lead to a specific target_ID
// Parameters: cur_parentID   - target ID of the parent of the subtree
//             cur_targetID   - ID of the sought after target
// Returns:    false  - the subtree specified by cur_parentID does not contain a child with ID cur_targetID
//             true   - ... does contain ...
// **************************************************************************
bool TREE::DoesLeadTo(int cur_parentID, int cur_targetID)
{
int     branch, targetID, count;

 count=0;
 for (branch=0; branch<MAX_BRANCHES; branch++)
  {
  targetID=DetermineTargetID(cur_parentID, branch);
  if (targetID == cur_targetID)
     return(true);
  else
     {
     if ((targetID != TARGETID_NOID) && (targetID != TARGETID_BACKUP))
        {
        if (DoesLeadTo(targetID, cur_targetID))
           count++;
        }
     }
  }

 // does any of the paths contain the correct targetID ?
 if (count > 0) return(true);

 return(false);
}



