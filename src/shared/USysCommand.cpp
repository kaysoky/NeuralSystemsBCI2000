/******************************************************************************
 * Program:   BCI2000                                                         *
 * Module:    USysCommand.cpp                                                 *
 * Comment:   This unit provides support for system commands                  *
 * Version:   0.15                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.15 - 03/29/2001 - first start                                           *
 ******************************************************************************/

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "USysCommand.h"

//---------------------------------------------------------------------------

// **************************************************************************
// Function:   SYSCMD
// Purpose:    The constructor for the SYSCMD object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SYSCMD::SYSCMD()
{
}


// **************************************************************************
// Function:   ~SYSCMD
// Purpose:    The destructor for the SYSCMD object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SYSCMD::~SYSCMD()
{
}


// **************************************************************************
// Function:   get_argument
// Purpose:    parses the parameter line that is being sent in the core
//             communication, or as stored in any BCI2000 .prm file
//             it returns the next token that is being delimited by either
//             a ' ' or '='
// Parameters: ptr - index into the line of where to start
//             buf - destination buffer for the token
//             line - the whole line
//             maxlen - maximum length of the line
// Returns:    the index into the line where the returned token ends
// **************************************************************************
int SYSCMD::get_argument(int ptr, char *buf, char *line, int maxlen)
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
// Function:   GetSysCmd
// Purpose:    Returns a parameter line in ASCII format
//             Tis parameter line is constructed, based upon the current
//             values in the PARAM object
// Parameters: N/A
// Returns:    a pointer to the parameter line
// **************************************************************************
char *SYSCMD::GetSysCmd()
{
 return(buffer);
}


// **************************************************************************
// Function:   ParseSysCmd
// Purpose:    This routine is called by coremessage->ParseMessage()
//             it parses the provided ASCII SysCmd line and copies its content
//             into the SYSCMD object
// Parameters: line - pointer to the ASCII system command line
//             length - length of this syscmd line
// Returns:    ERRSYSCMD_NOERR
// **************************************************************************
int SYSCMD::ParseSysCmd(char *new_line, int length)
{
 strncpy(buffer, new_line, length);

 return(ERRSYSCMD_NOERR);
}

