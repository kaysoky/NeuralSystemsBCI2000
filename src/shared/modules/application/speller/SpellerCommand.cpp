////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents a speller command (given in <>) which
//   can read and write itself from/to a stream.
//   For plain strings without command codes, the Code property is empty, and
//   the Value property contains the string.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SpellerCommand.h"
#include <sstream>

using namespace std;

SpellerCommand::SpellerCommand( const std::string& inString )
{
  istringstream iss( inString );
  if( !( iss >> *this ) )
  {
    mCode = "";
    mValue = "";
  }
}

// Formatted IO
std::istream&
SpellerCommand::ReadFromStream( std::istream& is )
{
  mValue = "";
  mCode = "";
  while( is.peek() != EOF && is.peek() != cOpenChar )
    mValue += is.get();
  if( is && mValue.empty() )
  {
    is.ignore();
    getline( is, mCode, cCloseChar );
    string::iterator i = mCode.begin();
    while( ::isalpha( *i ) )
    {
      *i = ::toupper( *i );
      ++i;
    }
    mValue = string( i, mCode.end() );
    mCode = string( mCode.begin(), i );
  }
  return is;
}

std::ostream&
SpellerCommand::WriteToStream( std::ostream& os ) const
{
  if( mCode.empty() )
    os << mValue;
  else
   os << cOpenChar << mCode << ' ' << mValue << cCloseChar;
  return os;
}

