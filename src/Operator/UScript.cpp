#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------


#include "..\shared\defines.h"
#include <stdio.h>

#include "UScript.h"
#include "UOperatorUtils.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


int SCRIPT::Initialize(PARAMLIST *my_paramlist, STATELIST *my_statelist, SYSLOG *my_syslog, TCustomWinSocket *my_socket)
{
 paramlist=my_paramlist;
 statelist=my_statelist;
 syslog=my_syslog;
 socket=my_socket;
 cur_line=0;

 return(0);
}


int SCRIPT::ExecuteScript(char *my_filename)
{
char    buf[256];
FILE    *fp;

 strcpy(filename, my_filename);

 // don't even try to do anything, if there is no filename defined
 if ((filename == NULL) || (filename[0] == '\0') || (filename[0] == '\r') || (filename[0] == '\n'))
    return(1);

 fp=fopen(filename, "rb");
 if (!fp)
    {
    sprintf(buf, "Could not open script file %s ...", filename);
    syslog->AddSysLogEntry(buf);
    return(0);
    }

 // walk through the scriptfile
 cur_line=1;
 while (!feof(fp))
  {
  fgets(buf, 256, fp);
  ExecuteCommand(buf);          // execute this particular command
  cur_line++;
  }

 if (fp) fclose(fp);

 return(1);
}


// **************************************************************************
// Function:   ExecuteLine
// Purpose:    executes one particular line in a scriptfile
// Parameters: line - pointer to the string containing the command
// Returns:    1 ... OK
//             0 ... Problem
// **************************************************************************
int SCRIPT::ExecuteCommand(char *line)
{
char    token[256], buf[256], value[256];
int     idx;
bool    ok;

 // don't even try to do anything, if there is nothing in the line
 if ((line == NULL) || (line[0] == '\0') || (line[0] == '\r') || (line[0] == '\n'))
    return(1);

 ok=false;

 idx=0;
 idx=get_argument(idx, token, line, 256);
 // command is LOAD
 if (stricmp(token, "LOAD") == 0)
    {
    ok=true;
    idx=get_argument(idx, token, line, 256);
    if (stricmp(token, "PARAMETERFILE") == 0)
       {
       idx=get_argument(idx, token, line, 256);
       if (!paramlist->LoadParameterList(token))
          {
          sprintf(buf, "%s: Error in scriptfile on line %d: Couldn't load parameter file %s", filename, cur_line, token);
          syslog->AddSysLogEntry(buf);
          }
       else
          {
          sprintf(buf, "%s: Successfully loaded parameter file %s", filename, token);
          syslog->AddSysLogEntry(buf);
          return(1);
          }
       }
    else
       ok=false;
    }

 // command is SET
 if (stricmp(token, "SET") == 0)
    {
    ok=true;
    idx=get_argument(idx, token, line, 256);
    if (stricmp(token, "STATE") == 0)
       {
       idx=get_argument(idx, token, line, 256);
       idx=get_argument(idx, value, line, 256);
       if (OperatorUtils::UpdateState(statelist, token, (unsigned short)atoi(value), socket) == ERR_NOERR)
          sprintf(buf, "%s: Set state %s to %s ... OK", filename, token, value);
       else
          sprintf(buf, "%s: Set state %s to %s ... Error", filename, token, value);
       syslog->AddSysLogEntry(buf);
       return(1);
       }
    else
       ok=false;
    }

 // command is INSERT
 if (stricmp(token, "INSERT") == 0)
    {
    ok=true;
    idx=get_argument(idx, token, line, 256);
    if (stricmp(token, "STATE") == 0)
       {
       while ((line[idx] == '\0') || (line[idx] == ' '))
        idx++;
       statelist->AddState2List(&line[idx]);
       idx=get_argument(idx, token, line, 256);
       sprintf(buf, "%s: Added state %s to list", filename, token);
       syslog->AddSysLogEntry(buf);
       return(1);
       }
    else
       ok=false;
    }

 // command is SYSTEM
 if (stricmp(token, "SYSTEM") == 0)
    {
    ok=true;
    idx=get_argument(idx, token, line, 256);
    if (system(token) == 0)
       {
       sprintf(buf, "%s: Successfully started %s", filename, token);
       syslog->AddSysLogEntry(buf);
       return(1);
       }
    else
       {
       sprintf(buf, "%s: Could not execute %s", filename, token);
       syslog->AddSysLogEntry(buf);
       return(0);
       }
    }

 // is it any command we don't know ?
 if (!ok)
    {
    sprintf(buf, "%s: Error in scriptfile on line %d: Syntax Error", filename, cur_line);
    syslog->AddSysLogEntry(buf);
    return(0);
    }

 return(1);
}


// **************************************************************************
// Function:   get_argument
// Purpose:    parses the parameter line in a buffer
//             it returns the next token that is being delimited by either
//             a ' ' or '='
// Parameters: ptr - index into the line of where to start
//             buf - destination buffer for the token
//             line - the whole line
//             maxlen - maximum length of the line
// Returns:    the index into the line where the returned token ends
// **************************************************************************
int SCRIPT::get_argument(int ptr, char *buf, char *line, int maxlen)
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



