////////////////////////////////////////////////////////////////////////////////
// $Id$
// File: FileWriterBase.cpp
//
// Date: Feb 17, 2006
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A base class that implements functionality common to all
//              file writer classes that output into a file.
//
// $Log$
// Revision 1.2  2006/02/23 19:34:08  mellinger
// Moved OutputStream() accessor definition into cpp file.
//
// Revision 1.1  2006/02/18 12:11:00  mellinger
// Support for EDF and GDF data formats.
//
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "FileWriterBase.h"

#include "UBCIError.h"
#include "BCIDirectry.h"

#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

static const char* bciParameterExtension = ".prm";

FileWriterBase::FileWriterBase()
{
}


FileWriterBase::~FileWriterBase()
{
}

void
FileWriterBase::Publish() const
{
  BEGIN_PARAMETER_DEFINITIONS
    "Storage int SavePrmFile= 0 1 0 1 "
      "// save additional parameter file (0=no, 1=yes) (boolean)",
  END_PARAMETER_DEFINITIONS
}

void
FileWriterBase::Preflight( const SignalProperties& Input,
                                 SignalProperties& Output ) const
{
  // File accessibility.
  string baseFileName = BCIDirectory()
    .SubjectDirectory( Parameter( "FileInitials" ) )
    .SubjectName( Parameter( "SubjectName" ) )
    .SessionNumber( Parameter( "SubjectSession" ) )
    .RunNumber( Parameter( "SubjectRun" ) )
    .FileExtension( DataFileExtension() )
    .CreatePath()
    .FilePath();

  {
    string dataFileName = baseFileName + DataFileExtension();

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
FileWriterBase::Initialize2( const SignalProperties& Input,
                             const SignalProperties& Output )
{
  mOutputFile.close();
  mOutputFile.clear();
}


void
FileWriterBase::StartRun()
{
  BCIDirectory bciDirectory = BCIDirectory()
                              .SubjectDirectory( Parameter( "FileInitials" ) )
                              .SubjectName( Parameter( "SubjectName" ) )
                              .SessionNumber( Parameter( "SubjectSession" ) )
                              .RunNumber( Parameter( "SubjectRun" ) )
                              .FileExtension( DataFileExtension() );
  string baseFileName = bciDirectory.FilePath();
  mFileName = baseFileName + DataFileExtension();
  // BCIDirectory will update the run number to the largest unused one
  // -- we want this to be reflected by the "SubjectRun" parameter.
  ostringstream oss;
  oss << setfill( '0' ) << setw( 2 ) << bciDirectory.RunNumber();
  Parameter( "SubjectRun" ) = oss.str().c_str();

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
}


void
FileWriterBase::StopRun()
{
  mOutputFile.close();
  mOutputFile.clear();
}


void
FileWriterBase::Write( const GenericSignal& Signal,
                       const STATEVECTOR&   Statevector )
{
  if( !mOutputFile )
    bcierr << "Error writing to file \"" << mFileName << "\"" << endl;
  State( "Recording" ) = ( mOutputFile ? 1 : 0 );
}


std::ostream&
FileWriterBase::OutputStream()
{
  return mOutputFile;
}

