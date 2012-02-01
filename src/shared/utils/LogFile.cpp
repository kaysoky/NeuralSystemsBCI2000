////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A std::ofstream descendant that centralizes/encapsulates details
//   of a log file.
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

