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

const SYSCMD SYSCMD::EndOfState( "EndOfState" );
const SYSCMD SYSCMD::EndOfParameter( "EndOfParameter" );
const SYSCMD SYSCMD::Start( "Start" );
const SYSCMD SYSCMD::Reset( "Reset" );

// **************************************************************************
// Function:   SYSCMD
// Purpose:    The constructor for the SYSCMD object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
SYSCMD::SYSCMD()
{
  buffer[ 0 ] = '\0';
}

SYSCMD::SYSCMD( const char* cmd )
{
  ::strncpy( buffer, cmd, LENGTH_SYSCMD );
  buffer[ LENGTH_SYSCMD - 1 ] = '\0';
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
 if( length >= LENGTH_SYSCMD )
   length = LENGTH_SYSCMD - 1;
 strncpy(buffer, new_line, length + 1);
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

bool
SYSCMD::operator<( const SYSCMD& s ) const
{
  return ::strncmp( buffer, s.buffer, LENGTH_SYSCMD ) < 0;
}

bool
SYSCMD::operator==( const SYSCMD& s ) const
{
  return ::strncmp( buffer, s.buffer, LENGTH_SYSCMD ) == 0;
}