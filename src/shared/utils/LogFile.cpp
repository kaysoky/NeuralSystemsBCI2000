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

#include "BCIStream.h"
#include "FileUtils.h"

using namespace std;

void
LogFile::Preflight() const
{
  string name = CurrentSession() + mExtension;
  ofstream preflightFile( name.c_str(), ios::out | ios::app );
  if( !preflightFile.is_open() )
    bcierr << "Could not open '" << name << "' for writing" << endl;
}

void
LogFile::StartRun()
{
  close();
  clear();
  string name = CurrentSession() + mExtension;
  open( name.c_str(), ios::out | ios::app );
  time_t now = ::time( NULL );
  *this << FileUtils::ExtractFile( name ) << '\n'
        << ::asctime( ::localtime( &now ) ) << endl;
}
