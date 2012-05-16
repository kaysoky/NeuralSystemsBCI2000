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
# include "Windows.h"
# ifdef _MSC_VER
#  include <direct.h>
#  define S_ISREG(x) ( (x) & S_IFREG )
#  define S_ISDIR(x) ( (x) & S_IFDIR )
# else // _MSC_VER
#  include <dir.h>
# endif // _MSC_VER
#elif __APPLE__
# include <mach-o/dyld.h>
#endif // _WIN32
#include <sys/stat.h>
#include <cstdlib>
#include <cerrno>

using namespace std;
using namespace FileUtils;

static string sSeparators = SeparatorSet();
static string sOriginalWD = GetCWD();
static string sInstallationDirectory = ParentDirectory( ExecutablePath() );
static OSMutex sWorkingDirMutex;

const string&
FileUtils::InstallationDirectory()
{
  return sInstallationDirectory;
}

string
FileUtils::ExecutablePath()
{
  string path;
#if _WIN32
  char* pFileName = NULL;
  DWORD size = 1024; // Must be large enough to hold path in WinXP.
  DWORD result = 0;
  do {
    delete[] pFileName;
    pFileName = new char[size];
    result = ::GetModuleFileNameA( NULL, pFileName, size );
    size *= 2;
  } while( result != 0 && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER );
  if( result != 0 )
    path = pFileName;
  delete[] pFileName;
#elif _GNU_SOURCE
  path = CanonicalPath( sOriginalWD + DirSeparator + program_invocation_name );
#elif __APPLE__
  uint32_t size = 0;
  ::_NSGetExecutablePath( NULL, &size );
  char* pPath = new char[size];
  pPath[0] = '\0';
  ::_NSGetExecutablePath( pPath, &size );
  path = CanonicalPath( pPath );
  delete[] pPath;
#else
# error Don't know how to obtain the executable's path on this platform.
#endif
  return path;
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
  if( IsAbsolutePath( inPath ) )
    return inPath;
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
#elif _GNU_SOURCE
  char* pPath = ::canonicalize_file_name( path.c_str() );
  if( pPath )
  {
    result = pPath;
    ::free( pPath );
  }
#elif __APPLE__
  char* pPath = ::realpath( path.c_str(), NULL );
  if( pPath )
  {
    result = pPath;
    ::free( pPath );
  }
#else
# error Don't know how to canonicalize a path on the current target OS.
#endif
  if( isDir && !result.empty() && sSeparators.find( *result.rbegin() ) == string::npos )
    result += DirSeparator;
  return result;
}

string
FileUtils::ParentDirectory( const std::string& inPath )
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

bool
FileUtils::IsFile( const std::string& inPath )
{
  struct stat s;
  return !::stat( inPath.c_str(), &s ) && ( S_ISREG( s.st_mode ) );
}

bool
FileUtils::IsDirectory( const std::string& inPath )
{
  struct stat s;
  return !::stat( inPath.c_str(), &s ) && ( S_ISDIR( s.st_mode ) );
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

