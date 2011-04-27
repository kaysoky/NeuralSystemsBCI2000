////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class that implements functionality common to all
//              file writer classes that output into a file.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
: mrOutputFormat( inOutputFormat ),
mEventWrite(NULL)
{
	mWriter = NULL;
}

FileWriterBase::~FileWriterBase()
{
	if (mWriter)
	{
		mWriter->finish();
		mWriter->Terminate();
		delete mWriter;
	}
	if (mEventWrite)
		CloseHandle(mEventWrite);
}

void
FileWriterBase::Publish() const
{
  BEGIN_PARAMETER_DEFINITIONS
    "Storage:Documentation int SavePrmFile= 0 1 0 1 "
      "// save additional parameter file (0=no, 1=yes) (boolean)",
  END_PARAMETER_DEFINITIONS

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
  if( Parameter( "SavePrmFile" ) == 1 )
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

  if( Parameter( "SavePrmFile" ) == 1 )
  {
    string paramFileName =  baseFileName + bciParameterExtension;
    ofstream file( paramFileName.c_str() );
    if( !( file << *Parameters << flush ) )
      bcierr << "Error writing parameters to file "
             << paramFileName
             << endl;
  }

  mrOutputFormat.StartRun( mOutputFile, mFileName );
	if (mWriter){
		delete mWriter;
	}

	while (!mSignalQueue.empty())
		mSignalQueue.pop();
	while (!mSVQueue.empty())
		mSVQueue.pop();

	ResetEvent( mEventWrite );
	mWriter = new FileWriterBase::Writer(this);
	mWriter->Start();
}


void
FileWriterBase::StopRun()
{
  mrOutputFormat.StopRun( mOutputFile );

  mOutputFile.close();
  mOutputFile.clear();

	mWriter->finish();
	mWriter->Terminate();
	delete mWriter;
	mWriter = NULL;
	CloseHandle( mEventWrite );
	mEventWrite = NULL;
}


void
FileWriterBase::Write( const GenericSignal& Signal,
                       const StateVector&   Statevector )
{
	mMutex.Acquire();
	mSignalQueue.push(Signal);
	mSVQueue.push(Statevector);
	mMutex.Release();
	SetEvent(mEventWrite);

}
void
FileWriterBase::WriteError()
{
    bcierr << "Error writing to file \"" << mFileName << "\"" << endl;
	State( "Recording" ) = 0;
}

FileWriterBase::Writer::Writer(FileWriterBase *parent)
{
	mParent = parent;
	mFinish = false;

}
FileWriterBase::Writer::~Writer()
{
	mFinish = true;
}

int FileWriterBase::Writer::Execute()
{
	while(!mFinish){
		while (!mFinish && mParent->mSignalQueue.size() == 0){
			if (mFinish) continue;
			this->Sleep(10);
		}
		if (mFinish) continue;
		mParent->mMutex.Acquire();
		if (mParent->mSignalQueue.size() == 0) continue;
		GenericSignal sig = mParent->mSignalQueue.front();
		StateVector sv = mParent->mSVQueue.front();
		mParent->mSignalQueue.pop();
		mParent->mSVQueue.pop();
		mParent->mMutex.Release();
		mParent->OutputFormat().Write( mParent->File(), sig, sv);
		if( !mParent->File() )
			mParent->WriteError();
		ResetEvent(mParent->mEventWrite);
	}
	return 0;
}
