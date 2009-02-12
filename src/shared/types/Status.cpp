////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for module status messages.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Status.h"

#include <string>
#include <sstream>

using namespace std;

const Status Status::Fail( "Error", fail );

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
  if( Code() == debugMessage )
    result = debug;
  else if( ( Code() >= firstWarningCode ) && ( Code() <= lastWarningCode ) )
    result = warning;
  else if( ( Code() >= firstErrorCode ) && ( Code() <= lastErrorCode ) )
    result = error;
  else switch( Code() )
  {
    case sourceInitialized:
    case sigprocInitialized:
    case appInitialized:
      result = initialized;
      break;
    case sourceRunning:
    case sigprocRunning:
    case appRunning:
      result = running;
      break;
    case sourceSuspended:
    case sigprocSuspended:
    case appSuspended:
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


