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

#if _WIN32
# include <Windows.h>
# define S_ISLNK(x) ( false )
# if _MSC_VER
#  include <direct.h>
#  define S_ISREG(x) ( (x) & S_IFREG )
#  define S_ISDIR(x) ( (x) & S_IFDIR )
# else // _MSC_VER
#  include <dir.h>
# endif // _MSC_VER
#elif __APPLE__
# include <mach-o/dyld.h>
#endif // _WIN32

#if _MSC_VER
# include "../fileio/dirent_win.h"
#else // _MSC_VER
# include <dirent.h>
#endif // _MSC_VER

#ifndef _WIN32
# include <cstdio>
# include <unistd.h>
#endif // _WIN32

#include <sys/stat.h>
#include <cstdlib>
#include <cerrno>
#include <sstream>
#include <algorithm>

#include "StringUtils.h"
#include "OSMutex.h"
#include "FileUtils.h"
#include "BCIException.h"

using namespace std;
using namespace FileUtils;

static const string& Separators()
{
  static string separators = SeparatorSet();
  return separators;
}
static const string& sSeparators_ = Separators(); // Force initialization at load time

static OSMutex& WorkingDirMutex()
{
  static OSMutex instance;
  return instance;
}
static const OSMutex& sWorkingDirMutex_ = WorkingDirMutex();

static const string& OriginalWD()
{
  static string originalWD = WorkingDirectory();
  return originalWD;
}
static const string& sOriginalWD_ = OriginalWD();

string
FileUtils::EnsureSeparator( const string& inDir )
{
  if( inDir.empty() || Separators().find( *inDir.rbegin() ) != string::npos )
    return inDir;
  return inDir + DirSeparator;
}

string
FileUtils::ClearSeparator( const string& inDir )
{
  if( inDir.empty() || Separators().find( *inDir.rbegin() ) == string::npos )
    return inDir;
  return inDir.substr( 0, inDir.length() - 1 );
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
#else // _WIN32
# if _GNU_SOURCE
  path = program_invocation_name;
# elif __APPLE__
  uint32_t size = 0;
  ::_NSGetExecutablePath( NULL, &size );
  char buf[size];
  buf[0] = '\0';
  ::_NSGetExecutablePath( buf, &size );
  path = buf;
# else
#  error Don´t know how to obtain the executable´s path on this platform.
# endif
  if( !IsAbsolutePath( path ) )
    path = OriginalWD() + path;
  path = CanonicalPath( path );
#endif // _WIN32
  if( path.empty() )
    throw std_runtime_error( "Could not obtain executable path" );
  return path;
}

string
FileUtils::ApplicationTitle()
{
  string baseName = ExtractBase( ExecutablePath() ),
         title;
  bool wasSpace = true,
       wasUpper = false;
  for( string::const_iterator i = baseName.begin(); i != baseName.end(); ++i )
  {
    if( ::isspace( *i ) )
    {
      wasSpace = true;
      wasUpper = false;
    }
    else
    {
      if( ::isupper( *i ) && !wasSpace && !wasUpper )
        title += " ";
      if( ::isupper( *i ) )
        wasUpper = true;
      else
        wasUpper = false;
      wasSpace = false;
    }
    title += *i;
  }
  return title;
}

const string&
FileUtils::InstallationDirectoryS()
{
  static string installationDirectory = ParentDirectoryS( ExecutablePath() );
  return installationDirectory;
}

string
FileUtils::WorkingDirectoryS()
{
  OSMutex::Lock lock( WorkingDirMutex() );
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
  if( !result.empty() && Separators().find( *result.rbegin() ) != string::npos )
    result = result.substr( result.length() - 1 );
  return result;
}

bool
FileUtils::ChangeDirectory( const string& inDir )
{
  OSMutex::Lock lock( WorkingDirMutex() );
  return !::chdir( inDir.c_str() );
}

string
FileUtils::AbsolutePath( const string& inPath )
{
  if( IsAbsolutePath( inPath ) )
    return inPath;
  return WorkingDirectory() + inPath;
}

string
FileUtils::CanonicalPath( const std::string& inPath )
{
  OSMutex::Lock lock( WorkingDirMutex() );
  string path = inPath,
         sep;
  if( !path.empty() && Separators().find( *path.rbegin() ) != string::npos )
  {
    sep = *path.rbegin();
    path = path.substr( 0, path.length() - 1 );
  }
  string result;
#if _WIN32
  wstring wpath = StringUtils::ToWide( path );
  DWORD size = ::GetFullPathNameW( wpath.c_str(), 0, 0, 0 );
  if( size > 0 )
  {
    wchar_t* pBuf = new wchar_t[size+1];
    if( ::GetFullPathNameW( wpath.c_str(), size, pBuf, 0 ) )
      wpath = pBuf;
    delete[] pBuf;
  }
  std::replace( wpath.begin(), wpath.end(), L'/', L'\\' );
  static const wstring prefix = L"\\\\?\\";
  wpath = prefix + wpath;
  size = ::GetLongPathNameW( wpath.c_str(), 0, 0 );
  if( size > 0 )
  {
    wchar_t* pBuf = new wchar_t[size+1];
    if( ::GetLongPathNameW( wpath.c_str(), pBuf, size ) )
      result = StringUtils::ToNarrow( pBuf ).substr( prefix.length() );
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
  char buffer[PATH_MAX + 1] = "";
  if( ::realpath( path.c_str(), buffer ) )
    result = buffer;
#else
# error Don´t know how to canonicalize a path on the current target OS.
#endif
  if( IsDirectory( inPath ) )
    result += sep;
  return result;
}

string
FileUtils::ParentDirectoryS( const std::string& inPath )
{
  if( IsFile( inPath ) )
    return FileUtils::ExtractDirectoryS( inPath );
  return EnsureSeparator( inPath ) + "..";
}

string
FileUtils::ExtractDirectoryS( const std::string& inPath )
{
  size_t pos = inPath.find_last_of( Separators() );
  if( pos == string::npos )
    return "";
  return inPath.substr( 0, pos );
}

string
FileUtils::ExtractFile( const std::string& inPath )
{
  size_t pos = inPath.find_last_of( Separators() );
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
  return !::stat( ClearSeparator( inPath ).c_str(), &s ) && ( S_ISDIR( s.st_mode ) );
}

bool
FileUtils::IsSymbolicLink( const std::string& inPath )
{
  struct stat s;
  return !::stat( ClearSeparator( inPath ).c_str(), &s ) && ( S_ISLNK( s.st_mode ) );
}

bool
FileUtils::Exists( const std::string& inPath )
{
  struct stat s;
  return !::stat( ClearSeparator( inPath ).c_str(), &s );
}

bool
FileUtils::IsAbsolutePath( const string& inPath )
{
#ifdef _WIN32
  return inPath.length() > 1 && ( inPath[1] == DriveSeparator || inPath[0] == DirSeparator && inPath[1] == DirSeparator );
#else
  return inPath.length() > 0 && inPath[0] == DirSeparator;
#endif
}

bool
FileUtils::MakeDirectory( const string& inName, bool inForce )
{
  bool result = false;
#ifdef _WIN32
  result = !::mkdir( inName.c_str() );
#else
  const int rwxr_xr_x = 0755;
  result = !::mkdir( inName.c_str(), rwxr_xr_x );
#endif
  if( inForce )
    result = result || IsDirectory( inName );
  if( !result && inForce )
    result = MakeDirectory( ParentDirectory( inName ), true ) && MakeDirectory( inName, false );
  return result;
}

bool
FileUtils::RemoveDirectory( const string& inName, bool inForce )
{
  bool success = true;
  string name = CanonicalPath( inName );
  if( inForce )
  {
    List entries;
    success = ListDirectory( name, entries );
    for( size_t i = 0; success && i < entries.size(); ++i )
    {
      string path = name + DirSeparator + entries[i];
      if( IsFile( path ) || IsSymbolicLink( path ) )
        success = RemoveFile( path );
      else if( IsDirectory( path ) )
        success = RemoveDirectory( path, true );
    }
  }
  if( success )
    success = !::rmdir( name.c_str() );
  return success;
}

bool
FileUtils::Rename( const string& inName, const string& inNewName )
{
  return !::rename( inName.c_str(), inNewName.c_str() );
}

bool
FileUtils::RemoveFile( const string& inName )
{
  return inName.empty() || !::unlink( inName.c_str() );
}

bool
FileUtils::ListDirectory( const string& inPath, List& outList )
{
  bool success = true;
  outList.clear();
  DIR* dir = ::opendir( CanonicalPath( inPath ).c_str() );
  success = ( dir != NULL );
  if( success )
  {
    struct dirent* entry;
    while( NULL != ( entry = ::readdir( dir ) ) )
    {
      string name = entry->d_name;
      if( name != "." && name != ".." )
        outList.push_back( name );
    }
    ::closedir( dir );
  }
  return success;
}

FileUtils::TemporaryFile::TemporaryFile()
: mpFile( new File )
{
  string prefix = ExtractBase( ExecutablePath() );
  for( int trial = 0; trial < 20 && !is_open(); ++trial )
  {
    char* p = ::tempnam( 0, prefix.c_str() );
    if( p )
    {
      mpFile->name = p;
      ::free( p );
      mpFile->name = CanonicalPath( ExtractDirectory( mpFile->name ) ) + ExtractFile( mpFile->name );
      open( mpFile->name.c_str(), ios::in | ios::out | ios::binary | ios::trunc );
    }
  }
  if( !is_open() )
    throw std_runtime_error( "Could not create temporary file: " << mpFile->name );
}

bool
FileUtils::TemporaryFile::Open()
{
  open( mpFile->name.c_str(), ios::in | ios::out | ios::binary );
  return is_open();
}

string
FileUtils::SearchURL( const string& inSearchTerms )
{
#ifndef WEBSEARCH_DOMAIN
  return "";
#else
  string url = "http://" WEBSEARCH_DOMAIN "/search?q=",
         searchTerms = inSearchTerms;
  if( searchTerms.empty() )
    searchTerms = ApplicationTitle();
  string site;
#ifdef PROJECT_DOMAIN
  if( !searchTerms.empty() )
    site = PROJECT_DOMAIN;
#endif
#ifdef PROJECT_NAME
  if( searchTerms.empty() )
    searchTerms = PROJECT_NAME;
#endif
  if( searchTerms.empty() )
    return "";
  if( !site.empty() )
    searchTerms = "site%3a" + site + " " + searchTerms;

  istringstream iss( searchTerms );
  string term;
  iss >> ws >> term;
  url += term;
  while( iss >> ws >> term )
    url += "+" + term;
  return url;
#endif // WEBSEARCH_DOMAIN
}

