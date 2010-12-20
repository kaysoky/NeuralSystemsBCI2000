////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates BCI2000 directory and file naming
//   conventions.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIDirectory.h"

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>
#endif
#include <stdlib.h>
#include <dirent.h>
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
  if( result.length() < 2 || result[ 1 ] != DriveSeparator )
    result = InstallationDirectory() + result;
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
  return inPath.length() > 1 && inPath[ 1 ] == DriveSeparator;
#else
  return inPath.length() > 0 && inPath[ 0 ] == DirSeparator;
#endif
}


