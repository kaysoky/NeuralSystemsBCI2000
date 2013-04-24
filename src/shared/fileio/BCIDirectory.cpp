////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates BCI2000 directory and file naming
//   conventions.
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

#include "BCIDirectory.h" 

#ifdef _WIN32
# include "windows.h"
# ifdef _MSC_VER
#  include <direct.h>
#  include "dirent_win.h"
# else // _MSC_VER
#  include <dir.h>
#  include <dirent.h>
# endif // _MSC_VER
#else // _WIN32
# include <sys/stat.h>
# include <dirent.h>
#endif // _WIN32
#include <cstdlib>
#include <cerrno>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace FileUtils;

static const char* BCIFileExtension = ".dat";

BCIDirectory::BCIDirectory()
: mFileExtension( BCIFileExtension ),
  mSessionNumber( none ),
  mDesiredRunNumber( none ),
  mActualRunNumber( none )
{
}

BCIDirectory&
BCIDirectory::UpdateRunNumber()
{
  mActualRunNumber = GetLargestRun( DirectoryPath() ) + 1;
  if( mDesiredRunNumber != none && mDesiredRunNumber > mActualRunNumber )
    mActualRunNumber = mDesiredRunNumber;
  return *this;
}

string
BCIDirectory::DirectoryPath() const
{
  string result = mDataDirectory;
  if( ! IsAbsolutePath(result) )
    result = InstallationDirectory() + result;
  for (string::iterator i = result.begin(); i != result.end(); i++)
  {
    if( *i == '\\' || *i == '/') *i = DirSeparator;
  }
  if( result.length() > 0 && result[ result.length() - 1 ] != DirSeparator )
    result += DirSeparator;
  if( !mSubjectName.empty() )
  {
    result += mSubjectName;
    if( mSessionNumber != none )
    {
      ostringstream oss;
      oss << setfill( '0' ) << setw( 3 ) << mSessionNumber;
      result += oss.str();
    }
    result += DirSeparator;
  }
  return result;
}

string
BCIDirectory::FilePath() const
{
  return DirectoryPath() + ConstructFileName();
}


const BCIDirectory&
BCIDirectory::CreatePath() const
{
  string fullPath = DirectoryPath();
  if( !IsAbsolutePath( fullPath ) )
    fullPath = InstallationDirectory() + fullPath;
  MakeDirectory( fullPath, true );
  return *this;
}

string
BCIDirectory::ConstructFileBase() const
{
  ostringstream oss;
  oss << mFilePrefix << mSubjectName;
  if( mSessionNumber != none )
    oss << "S" << setfill( '0' ) << setw( 3 ) << mSessionNumber;
  if( mDesiredRunNumber != none )
    oss << "R";
  if( !oss.tellp() )
    oss << "Data";
  return oss.str();
}

string
BCIDirectory::ConstructFileName() const
{
  std::ostringstream oss;
  oss << ConstructFileBase();
  if( mDesiredRunNumber == none )
  {
    if( mActualRunNumber > 1 )
      oss << mActualRunNumber;
  }
  else if( mActualRunNumber != none )
    oss << setfill( '0' ) << setw( 2 ) << mActualRunNumber;
  return oss.str();
}

int
BCIDirectory::GetLargestRun( const string& inPath )
{
  int largestRun = 0;
  DIR* dir = ::opendir( inPath.c_str() );
  if( dir != NULL )
  {
    string base = ConstructFileBase();
    struct dirent* entry;
    while( NULL != ( entry = ::readdir( dir ) ) )
    {
      string fileName = entry->d_name;
      int curRun = ExtractRunNumber( entry->d_name, base );
      if( curRun > largestRun )
        largestRun = curRun;
    }
    ::closedir( dir );
  }
  return largestRun;
}

int
BCIDirectory::ExtractRunNumber( const string& inFileName, const string& inFileBase )
{
  int result = -1;
  if( !::stricmp( inFileName.substr( 0, inFileBase.length() ).c_str(), inFileBase.c_str() ) )
  {
    string sub = inFileName.substr( inFileBase.length() );
    if( sub.empty() || !::isdigit( sub[0] ) )
      result = 1;
    else
      result = ::atoi( sub.c_str() );
  }
  return result;
}
