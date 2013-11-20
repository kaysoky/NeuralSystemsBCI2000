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
#include "BCIStream.h"
#include "FileUtils.h"
#include "ClassName.h"

#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

static const char* bciParameterExtension = ".prm";

static string
ParameterFile( const string& inDataFile )
{
  return FileUtils::ExtractDirectory( inDataFile )
       + FileUtils::ExtractBase( inDataFile )
       + bciParameterExtension;
}


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
  mrOutputFormat.Publish();

  string formatName = ClassName( typeid( *this ) );
  size_t offset = formatName.find( "FileWriter" );
  if( offset == string::npos || offset == 0 )
  {
    string ext = mrOutputFormat.DataFileExtension();
    size_t i = 0;
    while( i < ext.length() && ::ispunct( ext[i] ) )
      ++i;
    formatName = ext.substr( i );
  }
  else
    formatName = formatName.substr( 0, offset );

  if( Parameters->Exists( "FileFormat" ) )
    Parameters->Delete( "FileFormat" );
  string def = "Storage string FileFormat= " + formatName + " % % % // format of data file (readonly)";
  BEGIN_PARAMETER_DEFINITIONS
    def.c_str(),
  END_PARAMETER_DEFINITIONS

  if( OptionalParameter( "SavePrmFile" ) != 0 )
  {
    BEGIN_PARAMETER_DEFINITIONS
      "Storage:Documentation int SavePrmFile= 0 1 0 1 "
        "// save additional parameter file for each run (0=no, 1=yes) (boolean)",
    END_PARAMETER_DEFINITIONS
  }
}

void
FileWriterBase::Preflight( const SignalProperties& Input,
                                 SignalProperties& Output ) const
{
  mrOutputFormat.Preflight( Input, *Statevector );

  // State availability.
  State( "Recording" );

  // File accessibility.
  string dataFile = CurrentRun();

  // Does the data file exist?
  ifstream dataRead( dataFile.c_str() );
  if( dataRead.is_open() )
    bcierr << "Data file " << dataFile << " already exists, "
           << "will not be touched." << endl;
  else
  {
    // It does not exist, can we write to it?
    ofstream dataWrite( dataFile.c_str() );
    if( !dataWrite.is_open() )
      bcierr << "Cannot write to file " << dataFile << endl;
    else
    {
      dataWrite.close();
      ::remove( dataFile.c_str() );
    }
  }
  if( OptionalParameter( "SavePrmFile" ) == 1 )
  {
    string paramFile = ParameterFile( dataFile );
    ifstream paramRead( paramFile.c_str() );
    if( paramRead.is_open() )
      bcierr << "Parameter file " << paramFile << " already exists, "
             << "will not be touched." << endl;
    else
    {
      ofstream paramWrite( paramFile.c_str() );
      if( !paramWrite.is_open() )
        bcierr << "Cannot write to file " << paramFile << endl;
      else
      {
        paramWrite.close();
        ::remove( paramFile.c_str() );
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
  mOutputFile.close();
  mOutputFile.clear();
  mFileName = CurrentRun();
  mOutputFile.open( mFileName.c_str(), ios::out | ios::binary );

  if( OptionalParameter( "SavePrmFile" ) == 1 )
  {
    string paramFile = ParameterFile( mFileName );
    ofstream file( paramFile.c_str() );
    if( !( file << *Parameters << flush ) )
      bcierr << "Error writing parameters to file "
             << paramFile
             << endl;
  }

  mrOutputFormat.StartRun( mOutputFile, mFileName );
  mQueue.Clear();
  OSThread::Start();
}


void
FileWriterBase::StopRun()
{
  Halt();
  mrOutputFormat.StopRun( mOutputFile );
  mOutputFile.close();
  mOutputFile.clear();

  if( !mQueue.Empty() )
    bcierr << "Nonempty buffering queue" << endl;
}

void
FileWriterBase::Halt()
{
  SharedPointer<OSEvent> pTerminationEvent = OSThread::Terminate();
  mQueue.WakeConsumer();
  pTerminationEvent->Wait();
}

void
FileWriterBase::Write( const GenericSignal& Signal,
                       const StateVector&   Statevector )
{
  mQueue.Produce( make_pair( Signal, Statevector ) );
}

int FileWriterBase::OnExecute()
{
  Queue::Consumable c;
  while( mOutputFile && !IsTerminating() && ( c = mQueue.AwaitConsumption() ) )
  {
    mrOutputFormat.Write( mOutputFile, c->first, c->second );
    if( !mOutputFile )
    {
      bcierr << "Error writing to file \"" << mFileName << "\"" << endl;
      State( "Recording" ) = 0;
    }
  }
  while( !mQueue.Empty() )
  {
    c = mQueue.Consume();
    mrOutputFormat.Write( mOutputFile, c->first, c->second );
  }

  return 0;
}
