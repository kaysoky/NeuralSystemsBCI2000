/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop

#include "UStatus.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

const STATUS STATUS::Fail( "Error", 498 );

STATUS::STATUS()
: mCode( 0 )
{
  mStatus[ 0 ] = '\0';
}

STATUS::STATUS( const char* inStatus, int inCode )
: mCode( inCode )
{
  ::strncpy( mStatus, inStatus, LENGTH_STATUSLINE );
  mStatus[ LENGTH_STATUSLINE - 1 ] = '\0';
}

int
STATUS::ParseStatus( const char* line, int length )
{
char    temp[4];

 if ((length > 4) && (length < LENGTH_STATUSLINE))
    {
    strncpy(temp, line, 3);
    temp[3]=0;
    mCode=atoi(temp);
    strncpy(mStatus, line+4, length-4);
    mStatus[length-4]=0;
    }
 else
    {
    mCode=0;
    mStatus[0]=0;
    }

 return(ERRSTATUS_NOERR);
}


const char*
STATUS::GetStatus() const
{
  return mStatus;
}


int
STATUS::GetCode() const
{
  return mCode;
}

STATUS::ContentType
STATUS::Content() const
{
  ContentType result = unknown;
  if( ( GetCode() >= 300 ) && ( GetCode() < 400 ) )
    result = warning;
  else if( ( GetCode() >= 400 ) && ( GetCode() < 500 ) )
    result = error;
  else switch( GetCode() )
  {
    case 200:
    case 201:
    case 202:
      result = initialized;
      break;
    case 203:
    case 205:
    case 207:
      result = running;
      break;
    case 204:
    case 206:
    case 208:
      result = suspended;
      break;
  }
  return result;
}

void
STATUS::WriteToStream( ostream& os ) const
{
  ostringstream oss;
  oss << mCode << ": " << mStatus;
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
  os << mCode << ": " << mStatus;
  os.put( 0 );
  return os;
}






