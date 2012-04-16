////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents a collection of version information
//   that can read itself from a stream to accommodate the markup needs of
//   versioning software.
//   The recognized syntax is:
//   $Keyword1: value1 $ $Keyword2: value2 $ ...
//   There may not be white space between an opening $ and a keyword.
//   Colons following keywords are mandatory.
//   Any characters between closing and opening $ are ignored.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "VersionInfo.h"
#include "Version.h"
#include <sstream>
#include <cstring>

using namespace std;

const string VersionInfo::sEmptyString;
const VersionInfo VersionInfo::Current( BCI2000_VERSION );

const char* VersionInfo::sNames[] =
{
 "Version",
 "Source Revision",
 "Source Date",
 "Build Date",
};
const size_t VersionInfo::sNumNames = sizeof( sNames ) / sizeof( *sNames );

VersionInfo::VersionInfo( const string& inString )
{
  if( !inString.empty() )
  {
    istringstream iss( inString );
    ReadFromStream( iss );
  }
}

const std::string&
VersionInfo::operator[]( size_t inIdx ) const
{
  const string* result = &sEmptyString;
  if( inIdx < sNumNames )
  {
    const_iterator i = this->find( sNames[ inIdx ] );
    if( i != this->end() )
      result = &i->second;
  }
  return *result;
}

istream&
VersionInfo::ReadFromStream( istream& is )
{
  this->clear();
  struct
  {
    const char* keyword;
    int         replaceBy;
  } substitutions[] =
  {
    { "Version",     VersionID },
    { "version",     VersionID },
    { "versionID",   VersionID },

    { "Source Revision", Revision },
    { "rev",         Revision },
    { "revision",    Revision },

    { "Source Date", SourceDate },
    { "date",        SourceDate },

    { "Build Date",  BuildDate },
    { "build",       BuildDate },
    { "built",       BuildDate },
    { "builddate",   BuildDate },
  };

  string line;
  while( getline( is, line, '$' ) && getline( is, line, '$' ) )
  {
    istringstream linestream( line );
    string keyword, value;
    if( getline( linestream, keyword, ':' ) && getline( linestream >> ws, value ) )
    {
      for( size_t i = 0; i < sizeof( substitutions ) / sizeof( *substitutions ); ++i )
        if( !::stricmp( keyword.c_str(), substitutions[ i ].keyword ) )
          keyword = sNames[ substitutions[ i ].replaceBy ];
      while( !value.empty() && iswspace( *value.rbegin() ) )
        value = value.substr( 0, value.length() - 1 );
      if( !value.empty() )
        VersionInfoBase::operator[]( keyword ) = value;
    }
  }
  return is;
}

ostream&
VersionInfo::WriteToStream( ostream& os, bool pretty ) const
{
  if( pretty )
  {
    for( size_t i = 0; i < sizeof( sNames ) / sizeof( *sNames ); ++i )
    {
      const_iterator j = find( sNames[i] );
      if( j != end() && !j->second.empty() )
        os << j->first << ": " << j->second << "\n";
    }
  }
  else
  {
    for( const_iterator i = begin(); i != end(); ++i )
      os << "$" << i->first << ": " << i->second << " $ ";
  }
  return os;
}


