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

#include "PCHIncludes.h"
#pragma hdrstop

#include "USysCommand.h"

#include <string.h>
#include <string>
#include <iostream>

//---------------------------------------------------------------------------
#pragma package(smart_init)

using namespace std;

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

#if 0
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
#endif

// **************************************************************************
// Function:   GetSysCmd
// Purpose:    Returns a parameter line in ASCII format
//             Tis parameter line is constructed, based upon the current
//             values in the PARAM object
// Parameters: N/A
// Returns:    a pointer to the parameter line
// **************************************************************************
const char *SYSCMD::GetSysCmd()
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
int SYSCMD::ParseSysCmd(const char *new_line, int length)
{
 strncpy(buffer, new_line, length);
 buffer[ LENGTH_SYSCMD - 1 ] = '\0';
 return(ERRSYSCMD_NOERR);
}

void
SYSCMD::WriteToStream( ostream& os ) const
{
  const char* p = buffer;
  while( *p != '\0' )
  {
    if( *p == '}' )
      os.put( '\\' );
    os.put( *p );
    ++p;
  }
}

istream&
SYSCMD::ReadBinary( istream& is )
{
  string buf;
  if( getline( is, buf, '\0' ) )
    if( ParseSysCmd( buf.data(), buf.length() ) != ERRSYSCMD_NOERR )
      is.setstate( ios::failbit );
  return is;
}

ostream&
SYSCMD::WriteBinary( ostream& os ) const
{
  os << buffer;
  os.put( 0 );
  return os;
}


