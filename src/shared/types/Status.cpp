////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for module status messages.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Status.h"

#include <string>
#include <sstream>

using namespace std;

const Status Status::Fail( "Error", 498 );

Status::Status()
: mCode( 0 )
{
}

Status::Status( const string& inMessage, int inCode )
: mMessage( inMessage ),
  mCode( inCode )
{
}

Status::ContentType
Status::Content() const
{
  ContentType result = unknown;
  if( ( Code() >= 300 ) && ( Code() < 400 ) )
    result = warning;
  else if( ( Code() >= 400 ) && ( Code() < 500 ) )
    result = error;
  else switch( Code() )
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

ostream&
Status::WriteToStream( ostream& os ) const
{
  ostringstream oss;
  oss << mCode << ": " << mMessage;
  for( size_t p = oss.str().find( '}' ); p != string::npos; p = oss.str().find( '}', p ) )
    oss.str().replace( p, 1, "\\}" );
  return os << oss.str();
}

istream&
Status::ReadBinary( istream& is )
{
  is >> mCode;
  is.get();
  is >> ws;
  getline( is, mMessage, '\0' );
  return is;
}

ostream&
Status::WriteBinary( ostream& os ) const
{
  os << mCode << ": " << mMessage;
  os.put( 0 );
  return os;
}


