////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates BCI2000 directory and file naming
//   conventions.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#include <stdlib.h>
#include <errno.h>
#include <sstream>
#include <iomanip>

using namespace std;

static const char* BCIFileExtension = ".dat";

string BCIDirectory::sInstallationDirectory = BCIDirectory::GetCWD();

const string&
BCIDirectory::InstallationDirectory()
{
  return sInstallationDirectory;
}

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
  mActualRunNumber = mDesiredRunNumber;
  if( mDesiredRunNumber != none )
  {
    int largestRunNumber = GetLargestRun( DirectoryPath(), mFileExtension ) + 1;
    if( largestRunNumber > mDesiredRunNumber )
      mActualRunNumber = largestRunNumber;
  }
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
  result += mSubjectName;
  if( mSessionNumber != none )
  {
    ostringstream oss;
    oss << setfill( '0' ) << setw( 3 ) << mSessionNumber;
    result += oss.str();
  }
  result += DirSeparator;
  return result;
}

string
BCIDirectory::AbsolutePath( const std::string& inPath )
{
  string result;
  if( !IsAbsolutePath( inPath ) )
  {
    result = InstallationDirectory();
    if( result.empty() || *result.rbegin() != DirSeparator )
      result += DirSeparator;
    result += inPath;
  }
  else
  {
    result = inPath;
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
  string wd = GetCWD();
  ChangeForceDir( DirectoryPath() );
  ::chdir( wd.c_str() );
  return *this;
}


int
BCIDirectory::ChangeForceDir( const string& inPath )
{
  string fullPath = inPath;
  if( !IsAbsolutePath( fullPath ) )
    fullPath = InstallationDirectory() + fullPath;
  if( fullPath.length() < 1 || fullPath[ fullPath.length() - 1 ] != DirSeparator )
    fullPath += DirSeparator;
  // Changing directory is necessary to verify that the directory exists and is accessible.
  int err = ::chdir( fullPath.c_str() );
  if( err )
  {
    if( errno == EACCES )
      return err;
    else
    {
      size_t p = fullPath.rfind( DirSeparator, fullPath.length() - 2 );
      if( p == string::npos )
        return err;
      err = ChangeForceDir( fullPath.substr( 0, p ) );
      if( err )
        return err;
      err = MkDir( fullPath );
      if( err )
        return err;
      err = ::chdir( fullPath.c_str() );
    }
  }
  return err;
}


string
BCIDirectory::ConstructFileName() const
{
  ostringstream oss;
  oss << mSubjectName;
  if( mSessionNumber != none )
  {
     oss << "S" << setfill( '0' ) << setw( 3 ) << mSessionNumber;
     if( mActualRunNumber != none )
       oss << "R" << setfill( '0' ) << setw( 2 ) << mActualRunNumber;
  }
  return oss.str();
}


int
BCIDirectory::GetLargestRun( const string& inPath, const string& inExtension )
{
  int largestRun = 0;
  DIR* dir = ::opendir( inPath.c_str() );
  if( dir != NULL )
  {
    struct dirent* entry;
    while( NULL != ( entry = ::readdir( dir ) ) )
    {
      string fileName = entry->d_name;
      if( fileName.length() >= inExtension.length()
          && fileName.substr( fileName.length() - inExtension.length() ) == inExtension )
      {
        int curRun = ExtractRunNumber( entry->d_name );
        if( curRun > largestRun )
          largestRun = curRun;
      }
    }
    ::closedir( dir );
  }
  return largestRun;
}


int
BCIDirectory::ExtractRunNumber( const string& inFileName )
{
  int result = 0;
  size_t runBegin = inFileName.find_last_of( "Rr" ),
         numDigits = 0;
  if( runBegin != string::npos )
  {
    ++runBegin;
    while( ::isdigit( inFileName[ runBegin + numDigits ] ) )
      ++numDigits;
    result = ::atoi( inFileName.substr( runBegin, numDigits ).c_str() );
  }
  return result;
}

string
BCIDirectory::GetCWD()
{
  string result;
  int bufSize = 512;
  char* buf = NULL;
  const char* cwd = NULL;
  do
  {
    delete[] buf;
    buf = new char[ bufSize ];
    cwd = ::getcwd( buf, bufSize );
    bufSize += bufSize;
  } while( cwd == NULL && ( errno == ERANGE || errno == ENOMEM ) );
  if( cwd != NULL )
    result = cwd;
  delete[] buf;
  if( !result.empty() && result[ result.length() - 1 ] != DirSeparator )
    result += DirSeparator;
  return result;
}

int
BCIDirectory::MkDir( const string& inName )
{
#ifdef _WIN32
  return ::mkdir( inName.c_str() );
#else
  const int rwxr_xr_x = 0755;
  return ::mkdir( inName.c_str(), rwxr_xr_x );
#endif
}

bool
BCIDirectory::IsAbsolutePath( const string& inPath )
{
#ifdef _WIN32
  return inPath.length() > 1 && ( inPath[ 1 ] == DriveSeparator || inPath[ 0 ] == DirSeparator && inPath[ 1 ] == DirSeparator );
#else
  return inPath.length() > 0 && inPath[ 0 ] == DirSeparator;
#endif
}

