////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A std::ofstream descendant that centralizes/encapsulates details
//   of a log file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "LogFile.h"

#include "BCIError.h"
#include "BCIDirectory.h"

using namespace std;

void
LogFile::Preflight() const
{
  string name = FilePath();
  ofstream preflightFile( name.c_str(), ios::out | ios::app );
  if( !preflightFile.is_open() )
    bcierr << "Could not open '" << name << "' for writing" << endl;
}

void
LogFile::StartRun()
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
LogFile::FilePath() const
{
  return BCIDirectory()
    .SetDataDirectory( Parameter( "DataDirectory" ) )
    .SetSubjectName( Parameter( "SubjectName" ) )
    .SetSessionNumber( Parameter( "SubjectSession" ) )
    .CreatePath()
    .FilePath() + mExtension;
}

