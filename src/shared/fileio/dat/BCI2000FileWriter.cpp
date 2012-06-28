////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A FileWriter filter that stores data into a BCI2000 dat file.
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

#include "BCI2000FileWriter.h"

#include <string>

using namespace std;

// File writer filters must have a position string greater than
// that of the DataIOFilter.
RegisterFilter( BCI2000FileWriter, 1 );

void
BCI2000FileWriter::Publish()
{
  FileWriterBase::Publish();

  BEGIN_PARAMETER_DEFINITIONS
    "Storage:Documentation string StorageTime= % % % % "
      "// time of beginning of data storage",
  END_PARAMETER_DEFINITIONS
}

void
BCI2000FileWriter::Preflight( const SignalProperties& Input,
                                    SignalProperties& Output ) const
{
  FileWriterBase::Preflight( Input, Output );
  
  if( !string( Parameter( "StorageTime" ) ).empty() )
    bciout << "The StorageTime parameter will be overwritten with the"
           << " recording's actual date and time"
           << endl;
}

void
BCI2000FileWriter::StartRun()
{
  time_t now = ::time( NULL );
  struct tm* timeinfo = ::localtime( &now );
  char buffer[20];
  ::strftime( buffer, sizeof( buffer ), "%Y-%m-%dT%H:%M:%S", timeinfo );
  Parameter( "StorageTime" ) = buffer;
  FileWriterBase::StartRun();
}

