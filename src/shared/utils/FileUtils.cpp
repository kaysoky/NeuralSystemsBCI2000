//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: File-system related utility functions.
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
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "FileUtils.h"
#include "OSMutex.h"
#include "StringUtils.h"

#ifdef _WIN32
# include "windows.h"
# ifdef _MSC_VER
#  include <direct.h>
//#  include "dirent_win.h"
# else // _MSC_VER
#  include <dir.h>
//#  include <dirent.h>
# endif // _MSC_VER
#else // _WIN32
//# include <dirent.h>
#endif // _WIN32
#include <sys/stat.h>
#include <cstdlib>
#include <cerrno>

using namespace std;
using namespace FileUtils;

static string sSeparators = SeparatorSet();
static string sOriginalWD = GetCWD();
static string sInstallationDirectory = UpDirectory( ExecutablePath() );
static OSMutex sWorkingDirMutex;

const string&
FileUtils::InstallationDirectory()
{
  return sInstallationDirectory;
}

string
FileUtils::ExecutablePath()
{
#if _WIN32
  static char* pFileName = NULL;
  if( pFileName == NULL )
  {
    DWORD size = 1024; // Must be large enough to hold path in WinXP.
    DWORD result = 0;
    do {
      delete[] pFileName;
      pFileName = new char[size];
      result = ::GetModuleFileNameA( NULL, pFileName, size );
      size *= 2;
    } while( result != 0 && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER );
    if( result == 0 )
    {
      delete[] pFileName;
      pFileName = NULL;
    }
  }
  return pFileName;
#else // _WIN32
  return CanonicalPath( sOriginalWD + DirSeparator + program_invocation_name );
#endif // _WIN32
}

string
FileUtils::GetCWD()
{
  OSMutex::Lock lock( sWorkingDirMutex );
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
  if( !result.empty() && sSeparators.find( *result.rbegin() ) == string::npos )
    result += DirSeparator;
  return result;
}

bool
FileUtils::ChDir( const string& inDir )
{
  OSMutex::Lock lock( sWorkingDirMutex );
  return !::chdir( inDir.c_str() );
}

string
FileUtils::AbsolutePath( const string& inPath )
{
  return GetCWD() + inPath;
}

string
FileUtils::CanonicalPath( const std::string& inPath )
{
  OSMutex::Lock lock( sWorkingDirMutex );
  string path = inPath;
  if( !path.empty() && sSeparators.find( *path.rbegin() ) != string::npos )
    path = path.substr( 0, path.length() - 1 );
  bool isDir = IsDirectory( inPath );
  string result;
#if _WIN32
  wchar_t* pFilePart;
  wstring wpath = StringUtils::ToWide( path );
  DWORD size = ::GetFullPathNameW( wpath.c_str(), 0, NULL, &pFilePart );
  if( size > 0 )
  {
    wchar_t* pBuf = new wchar_t[size];
    if( ::GetFullPathNameW( wpath.c_str(), size, pBuf, &pFilePart ) )
      result = StringUtils::ToNarrow( pBuf );
    delete[] pBuf;
  }
#else // _WIN32
  char* pPath = ::canonicalize_file_name( path.c_str() );
  if( pPath )
  {
    result = pPath;
    ::free( pPath );
  }
#endif // _WIN32
  if( isDir && !result.empty() && sSeparators.find( *result.rbegin() ) == string::npos )
    result += DirSeparator;
  return result;
}

string
FileUtils::UpDirectory( const std::string& inPath )
{
  if( IsFile( inPath ) )
    return FileUtils::ExtractDirectory( inPath );
  return inPath + DirSeparator + ".." + DirSeparator;
}

string
FileUtils::ExtractDirectory( const std::string& inPath )
{
  size_t pos = inPath.find_last_of( SeparatorSet() );
  if( pos == string::npos )
    return "";
  return inPath.substr( 0, pos + 1 );
}

string
FileUtils::ExtractFile( const std::string& inPath )
{
  size_t pos = inPath.find_last_of( SeparatorSet() );
  if( pos == string::npos )
    return inPath;
  return inPath.substr( pos + 1 );
}

string
FileUtils::ExtractBase( const std::string& inPath )
{
  string file = ExtractFile( inPath );
  size_t pos = file.find_last_of( "." );
  if( pos == string::npos )
    return file;
  return file.substr( 0, pos );
}

string
FileUtils::ExtractExtension( const std::string& inPath )
{
  string file = ExtractFile( inPath );
  size_t pos = file.find_last_of( "." );
  if( pos == string::npos )
    return "";
  return file.substr( pos );
}

namespace {
bool ExistsAs( const std::string& inPath, int inFlag )
{
  struct stat s;
  return !::stat( inPath.c_str(), &s ) && ( s.st_mode & inFlag );
}
} // namespace

bool
FileUtils::IsFile( const std::string& inPath )
{
  return ExistsAs( inPath, _S_IFREG );
}

bool
FileUtils::IsDirectory( const std::string& inPath )
{
  return ExistsAs( inPath, _S_IFDIR );
}

bool
FileUtils::IsAbsolutePath( const string& inPath )
{
#ifdef _WIN32
  return inPath.length() > 1 && ( inPath[ 1 ] == DriveSeparator || inPath[ 0 ] == DirSeparator && inPath[ 1 ] == DirSeparator );
#else
  return inPath.length() > 0 && inPath[ 0 ] == DirSeparator;
#endif
}

bool
FileUtils::MkDir( const string& inName )
{
#ifdef _WIN32
  return !::mkdir( inName.c_str() );
#else
  const int rwxr_xr_x = 0755;
  return !::mkdir( inName.c_str(), rwxr_xr_x );
#endif
}

