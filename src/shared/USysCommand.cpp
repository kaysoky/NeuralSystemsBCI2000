#include "PCHIncludes.h"
#pragma hdrstop

#include "USysCommand.h"

#include <string>
#include <iostream>

using namespace std;

const SysCommand SysCommand::EndOfState( "EndOfState" );
const SysCommand SysCommand::EndOfParameter( "EndOfParameter" );
const SysCommand SysCommand::Start( "Start" );
const SysCommand SysCommand::Reset( "Reset" );
const SysCommand SysCommand::Suspend( "Suspend" );


void
SysCommand::WriteToStream( ostream& os ) const
{
  string::const_iterator p = mBuffer.begin();
  while( p != mBuffer.end() )
  {
    if( *p == '}' )
      os.put( '\\' );
    os.put( *p++ );
  }
}


istream&
SysCommand::ReadBinary( istream& is )
{
  return ::getline( is, mBuffer, '\0' );
}


ostream&
SysCommand::WriteBinary( ostream& os ) const
{
  os.write( mBuffer.data(), mBuffer.size() );
  os.put( '\0' );
  return os;
}


bool
SysCommand::operator<( const SysCommand& s ) const
{
  return mBuffer < s.mBuffer;
}


bool
SysCommand::operator==( const SysCommand& s ) const
{
  return mBuffer == s.mBuffer;
}
