#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include "UStatus.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
//---------------------------------------------------------------------------
#pragma package(smart_init)

using namespace std;

const STATUS STATUS::Fail( "Error", 498 );

STATUS::STATUS()
: code( 0 )
{
  status[ 0 ] = '\0';
}

STATUS::STATUS( const char* inStatus, int inCode )
: code( inCode )
{
  ::strncpy( status, inStatus, LENGTH_STATUSLINE );
  status[ LENGTH_STATUSLINE - 1 ] = '\0';
}

int STATUS::ParseStatus(const char *line, int length)
{
char    temp[4];

 if ((length > 4) && (length < LENGTH_STATUSLINE))
    {
    strncpy(temp, line, 3);
    temp[3]=0;
    code=atoi(temp);
    strncpy(status, line+4, length-4);
    status[length-4]=0;
    }
 else
    {
    code=0;
    status[0]=0;
    }

 return(ERRSTATUS_NOERR);
}


const char *STATUS::GetStatus()
{
 return(status);
}


int STATUS::GetCode()
{
 return(code);
}

void
STATUS::WriteToStream( ostream& os ) const
{
  ostringstream oss;
  oss << code << ": " << status;
  for( size_t p = oss.str().find( '}' ); p != string::npos; p = oss.str().find( '}', p ) )
    oss.str().replace( p, 1, "\\}" );
  os << oss.str();
}

istream&
STATUS::ReadBinary( istream& is )
{
  string buf;
  if( getline( is, buf, '\0' ) )
    if( ParseStatus( buf.data(), buf.length() ) != ERRSTATUS_NOERR )
      is.setstate( ios::failbit );
  return is;
}

ostream&
STATUS::WriteBinary( ostream& os ) const
{
  os << code << ": " << status;
  os.put( 0 );
  return os;
}


