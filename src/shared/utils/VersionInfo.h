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
  VersionInfo( const std::string& inString = "" );
  const std::string& operator[]( const std::string& s )
    { return VersionInfoBase::operator[]( s ); }
  const std::string& operator[]( size_t ) const;
  std::istream& ReadFromStream( std::istream& is );
  std::ostream& WriteToStream( std::ostream& os, bool pretty = false ) const;

  static const VersionInfo Current;

 private:
  static const std::string sEmptyString;
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
