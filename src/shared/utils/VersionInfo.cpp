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
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "VersionInfo.h"
#include <sstream>
#include <cstring>

using namespace std;

const char* VersionInfo::sNames[] =
{
 "Version",
 "Source Revision",
 "Source Date",
 "Build Date",
};
const size_t VersionInfo::sNumNames = sizeof( sNames ) / sizeof( *sNames );

const std::string&
VersionInfo::operator[]( size_t inIdx )
{
  static string emptyString;
  const string* result = &emptyString;
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
VersionInfo::WriteToStream( ostream& os ) const
{
  for( const_iterator i = begin(); i != end(); ++i )
    os << "$" << i->first << ": " << i->second << " $ ";
  return os;
}


