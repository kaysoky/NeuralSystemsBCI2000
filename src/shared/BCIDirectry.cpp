///////////////////////////////////////////////////
//  BCIDirectry.cpp
//  BCI Directory Management Functions
///////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIDirectry.h"
// Directory management is platform specific.
#include <vcl.h>
#include <dir.h>

#include <sstream>
#include <iomanip>

using namespace std;

static const char BCIFileExtension[] = ".dat";

const string&
BCIDirectory::InstallationDirectory()
{
  static string installationDirectory
    = Sysutils::ExtractFilePath( System::ParamStr( 0 ) ).c_str();
  return installationDirectory;
}


string
BCIDirectory::DirectoryPath() const
{
  string result = mSubjectDirectory;
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
BCIDirectory::FilePath() const
{
  return DirectoryPath() + ConstructFileName();
}


const BCIDirectory&
BCIDirectory::CreatePath() const
{
  ChangeForceDir( DirectoryPath() );
  return *this;
}


int
BCIDirectory::ChangeForceDir( const string& inPath )
{
  string fullPath = inPath;
  if( fullPath.length() < 2 || fullPath[ 1 ] != DriveSeparator )
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
      err = ::mkdir( fullPath.c_str() );
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
  int runNumber = GetLargestRun( DirectoryPath() ) + 1;
  if( mRunNumber > runNumber )
    runNumber = mRunNumber;
  ostringstream oss;
  oss << mSubjectName;
  if( mSessionNumber != none )
  {
     oss << "S" << setfill( '0' ) << setw( 3 ) << mSessionNumber;
     if( mRunNumber != none )
       oss << "R" << setfill( '0' ) << setw( 2 ) << runNumber;
  }
  return oss.str();
}


int
BCIDirectory::GetLargestRun( const string& inPath )
{
  int largestRun = 0;
  AnsiString path = inPath.c_str();
  path += "*";
  path += BCIFileExtension;
  TSearchRec sr;
  if( !Sysutils::FindFirst( path, faAnyFile, sr ) )
  {
    do
    {
      int curRun = ExtractRunNumber( sr.Name.c_str() );
      if( curRun > largestRun )
        largestRun = curRun;
    } while( !Sysutils::FindNext( sr ) );
    Sysutils::FindClose( sr );
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


#ifdef OLD_BCIDTRY
// Legacy functions.
int
BCIDirectory::ProcPath()
{
  return ChangeForceDir( mSubjectDirectory );
}


const char*
BCIDirectory::ProcSubDir()
{
  mSubjectPath = DirectoryPath();
  ChangeForceDir( mSubjectPath );
  return mSubjectPath.c_str();
}

#endif // OLD_BCIDTRY

