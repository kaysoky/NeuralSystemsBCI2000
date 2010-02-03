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
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef VERSION_INFO_H
#define VERSION_INFO_H

#include <iostream>
#include <string>
#include <map>

typedef std::map<std::string, std::string> VersionInfoBase;

class VersionInfo : public VersionInfoBase
{
 public:
  enum StandardKeys
  {
    VersionID = 0,
    Revision,
    SourceDate,
    BuildDate,
  };
  const std::string& operator[]( const std::string& s )
    { return VersionInfoBase::operator[]( s ); }
  const std::string& operator[]( size_t );
  std::istream& ReadFromStream( std::istream& is );
  std::ostream& WriteToStream( std::ostream& os ) const;

 private:
  static const char*  sNames[];
  static const size_t sNumNames;
};

inline
std::istream& operator>>( std::istream& is, VersionInfo& v )
{ return v.ReadFromStream( is ); }

inline
std::ostream& operator<<( std::ostream& os, const VersionInfo& v )
{ return v.WriteToStream( os ); }

#endif // VERSION_INFO_H
