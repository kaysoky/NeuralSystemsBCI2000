////////////////////////////////////////////////////////////////////////////////
//
// File:   TaskLogFile.cpp
//
// Date:   Feb 8, 2004
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A std::ofstream descendant that centralizes/encapsulates details
//         of a user task log file.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "TaskLogFile.h"

#include "UBCIError.h"
#include "BCIDirectry.h"

using namespace std;

void
TaskLogFile::Preflight() const
{
  string name = FilePath();
  ofstream preflightFile( name.c_str(), ios::out | ios::app );
  if( !preflightFile.is_open() )
    bcierr << "Could not open '" << name << "' for writing" << endl;
}

void
TaskLogFile::Initialize()
{
  close();
  clear();
  string name = FilePath();
  open( name.c_str(), ios::out | ios::app );
  const char separators[] = "\\:/";
  size_t dirpos = name.find_last_of( separators );
  if( dirpos != name.npos )
    name.replace( 0, dirpos + 1, "" );
  time_t now = ::time( NULL );
  *this << name << '\n'
        << ::asctime( ::localtime( &now ) ) << endl;
}

string
TaskLogFile::FilePath() const
{
  return BCIDirectory()
    .SubjectDirectory( Parameter( "FileInitials" ) )
    .SubjectName( Parameter( "SubjectName" ) )
    .SessionNumber( Parameter( "SubjectSession" ) )
    .CreatePath()
    .FilePath() + mExtension;
}

