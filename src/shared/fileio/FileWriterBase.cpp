////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class that implements functionality common to all
//              file writer classes that output into a file.
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

#include "FileWriterBase.h"

#include "BCIError.h"
#include "BCIDirectory.h"

#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

static const char* bciParameterExtension = ".prm";

FileWriterBase::FileWriterBase( GenericOutputFormat& inOutputFormat )
: mrOutputFormat( inOutputFormat )
{
}

FileWriterBase::~FileWriterBase()
{
  Halt();
}

void
FileWriterBase::Publish()
{
  if( ( OptionalParameter( "SavePrmFile" ) != 0 ) )
  {
    BEGIN_PARAMETER_DEFINITIONS
      "Storage:Documentation int SavePrmFile= 0 1 0 1 "
        "// save additional parameter file for each run (0=no, 1=yes) (boolean)",
    END_PARAMETER_DEFINITIONS
  }

  mrOutputFormat.Publish();
}

void
FileWriterBase::Preflight( const SignalProperties& Input,
                                 SignalProperties& Output ) const
{
  mrOutputFormat.Preflight( Input, *Statevector );

  // State availability.
  State( "Recording" );

  // File accessibility.
  string baseFileName = BCIDirectory()
    .SetDataDirectory( Parameter( "DataDirectory" ) )
    .SetSubjectName( Parameter( "SubjectName" ) )
    .SetSessionNumber( Parameter( "SubjectSession" ) )
    .SetRunNumber( Parameter( "SubjectRun" ) )
    .SetFileExtension( mrOutputFormat.DataFileExtension() )
    .CreatePath()
    .FilePath();

  {
    string dataFileName = baseFileName + mrOutputFormat.DataFileExtension();

    // Does the data file exist?
    ifstream dataRead( dataFileName.c_str() );
    if( dataRead.is_open() )
      bcierr << "Data file " << dataFileName << " already exists, "
             << "will not be touched." << endl;
    else
    {
      // It does not exist, can we write to it?
      ofstream dataWrite( dataFileName.c_str() );
      if( !dataWrite.is_open() )
        bcierr << "Cannot write to file " << dataFileName << endl;
      else
      {
        dataWrite.close();
        ::remove( dataFileName.c_str() );
      }
    }
  }
  if( OptionalParameter( "SavePrmFile" ) == 1 )
  {
    string paramFileName =  baseFileName + bciParameterExtension;
    ifstream paramRead( paramFileName.c_str() );
    if( paramRead.is_open() )
      bcierr << "Parameter file " << paramFileName << " already exists, "
             << "will not be touched." << endl;
    else
    {
      ofstream paramWrite( paramFileName.c_str() );
      if( !paramWrite.is_open() )
        bcierr << "Cannot write to file " << paramFileName << endl;
      else
      {
        paramWrite.close();
        ::remove( paramFileName.c_str() );
      }
    }
  }
  Output = SignalProperties( 0, 0 );
}


void
FileWriterBase::Initialize( const SignalProperties& Input,
                            const SignalProperties& /*Output*/ )
{
  mOutputFile.close();
  mOutputFile.clear();

  mrOutputFormat.Initialize( Input, *Statevector );
}


void
FileWriterBase::StartRun()
{
  BCIDirectory bciDirectory = BCIDirectory()
    .SetDataDirectory( Parameter( "DataDirectory" ) )
    .SetSubjectName( Parameter( "SubjectName" ) )
    .SetSessionNumber( Parameter( "SubjectSession" ) )
    .SetRunNumber( Parameter( "SubjectRun" ) )
    .SetFileExtension( mrOutputFormat.DataFileExtension() );
  string baseFileName = bciDirectory.FilePath();
  mFileName = baseFileName + mrOutputFormat.DataFileExtension();
  // BCIDirectory will update the run number to the largest unused one
  // -- we want this to be reflected by the "SubjectRun" parameter.
  ostringstream oss;
  oss << setfill( '0' ) << setw( 2 ) << bciDirectory.RunNumber();
  Parameter( "SubjectRun" ) = oss.str();

  mOutputFile.close();
  mOutputFile.clear();
  mOutputFile.open( mFileName.c_str(), ios::out | ios::binary );

  if( OptionalParameter( "SavePrmFile" ) == 1 )
  {
    string paramFileName =  baseFileName + bciParameterExtension;
    ofstream file( paramFileName.c_str() );
    if( !( file << *Parameters << flush ) )
      bcierr << "Error writing parameters to file "
             << paramFileName
             << endl;
  }

  mrOutputFormat.StartRun( mOutputFile, mFileName );
  OSThread::Start();
}


void
FileWriterBase::StopRun()
{
  Halt();
  mrOutputFormat.StopRun( mOutputFile );
  mOutputFile.close();
  mOutputFile.clear();

  if( !mSignalQueue.empty() )
    bcierr << "Nonempty buffering queue" << endl;
}

void
FileWriterBase::Halt()
{
  SharedPointer<OSEvent> pTerminationEvent = OSThread::Terminate();
  mEvent.Set(); // Trigger a last execution of the thread's while loop.
  pTerminationEvent->Wait();
}

void
FileWriterBase::Write( const GenericSignal& Signal,
                       const StateVector&   Statevector )
{
  OSMutex::Lock lock( mMutex );
  mSignalQueue.push( Signal );
  mStateVectorQueue.push( Statevector );
  mEvent.Set();
}

int FileWriterBase::Execute()
{
  OSMutex::Lock lock( mMutex );
  while( !IsTerminating() || !mSignalQueue.empty() )
  {
    if( !IsTerminating() && mSignalQueue.empty() )
    {
      OSMutex::Unlock unlock( mMutex );
      mEvent.Wait();
      mEvent.Reset();
    }
    while( !mSignalQueue.empty() )
    {
      GenericSignal signal = mSignalQueue.front();
      StateVector stateVector = mStateVectorQueue.front();
      mSignalQueue.pop();
      mStateVectorQueue.pop();
      OSMutex::Unlock unlock( mMutex );// Allow queue buffering during file writes (which may block).
      mrOutputFormat.Write( mOutputFile, signal, stateVector );
    }
    if( !mOutputFile )
    {
      bcierr << "Error writing to file \"" << mFileName << "\"" << endl;
      State( "Recording" ) = 0;
    }
  }
  return 0;
}

