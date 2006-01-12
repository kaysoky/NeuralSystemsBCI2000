////////////////////////////////////////////////////////////////////////////////
//
// File: BCIDatFileWriter.cpp
//
// Date: Nov 11, 2003
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A filter that stores data into a BCI2000 dat file.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIDatFileWriter.h"

#include "UBCIError.h"
#include "BCIDirectry.h"

#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

// File writer filters must have a position string greater than
// that of the DataIOFilter.
RegisterFilter( BCIDatFileWriter, 1 );

static const char* bciDataExtension = ".dat",
                 * bciParameterExtension = ".prm";

BCIDatFileWriter::BCIDatFileWriter()
{
  BEGIN_PARAMETER_DEFINITIONS
    "Storage int SavePrmFile= 0 1 0 1 "
      "// save additional parameter file (0=no, 1=yes) (boolean)",
  END_PARAMETER_DEFINITIONS
}


BCIDatFileWriter::~BCIDatFileWriter()
{
}


void
BCIDatFileWriter::Preflight( const SignalProperties& Input,
                                   SignalProperties& Output ) const
{
  // File accessibility.
  string baseFileName = BCIDirectory()
    .SubjectDirectory( Parameter( "FileInitials" ) )
    .SubjectName( Parameter( "SubjectName" ) )
    .SessionNumber( Parameter( "SubjectSession" ) )
    .RunNumber( Parameter( "SubjectRun" ) )
    .CreatePath()
    .FilePath();

  {
    string dataFileName = baseFileName + bciDataExtension;

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

  switch( Input.Type() )
  {
    case SignalType::int16:
    case SignalType::int32:
    case SignalType::float32:
      /* These types are OK */
      break;

    default:
      bcierr << Input.Type().Name()
             << " data type unsupported for BCI2000 files"
             << endl;
  }
  Output = SignalProperties( 0, 0 );
}


void
BCIDatFileWriter::Initialize2( const SignalProperties& Input,
                               const SignalProperties& Output )
{
  mOutputFile.close();
  mOutputFile.clear();
  mInputProperties = Input;
}


void
BCIDatFileWriter::StartRun()
{
  BCIDirectory bciDirectory = BCIDirectory()
                              .SubjectDirectory( Parameter( "FileInitials" ) )
                              .SubjectName( Parameter( "SubjectName" ) )
                              .SessionNumber( Parameter( "SubjectSession" ) )
                              .RunNumber( Parameter( "SubjectRun" ) );
  string baseFileName = bciDirectory.FilePath(),
         dataFileName = baseFileName + bciDataExtension;
  // BCIDirectory will update the run number to the largest unused one
  // -- we want this to be reflected by the "SubjectRun" parameter.
  ostringstream oss;
  oss << setfill( '0' ) << setw( 2 ) << bciDirectory.RunNumber();
  Parameter( "SubjectRun" ) = oss.str().c_str();

  mOutputFile.close();
  mOutputFile.clear();
  mOutputFile.open( dataFileName.c_str(), ios::out | ios::binary );

  // We write 16 bit data in the old format to maintain backward compatibility.
  bool useOldFormat = ( mInputProperties.Type() == SignalType::int16 );

  // Write the header.
  //
  // Because the header contains its own length in ASCII format, it is a bit
  // tricky to get this right if we don't want to imply a fixed width for the
  // HeaderLen field.
  ostringstream header;
  header << " "
         << "SourceCh= " << ( int )Parameter( "SoftwareCh" ) << " "
         << "StatevectorLen= " << Statevector->Length();
  if( !useOldFormat )
    header << " "
           << "DataFormat= "
           << mInputProperties.Type().Name();
  header << "\r\n"
         << "[ State Vector Definition ] \r\n";
  States->WriteBinary( header );
  header << "[ Parameter Definition ] \r\n";
  Parameters->WriteBinary( header );
  header << "\r\n";

  string headerBegin;
  if( !useOldFormat )
    headerBegin = "BCI2000V= 1.1 ";
  headerBegin += "HeaderLen= ";
  size_t fieldLength = 5; // Follow the old scheme
                          // (5 characters for the header length field),
                          // but allow for a longer HeaderLen field
                          // if necessary.
  ostringstream headerLengthField;
  do
  { // This trial-and-error scheme is invariant against changes in number
    // formatting and character set (unicode, hex, ...).
    size_t headerLength = headerBegin.length() + fieldLength + header.str().length();
    headerLengthField.str( "" );
    headerLengthField << headerBegin
                      << setfill( ' ' ) << setw( fieldLength )
                      << headerLength;
  } while( headerLengthField
           && ( headerLengthField.str().length() - headerBegin.length() != fieldLength++ ) );

  mOutputFile.write( headerLengthField.str().data(), headerLengthField.str().size() );
  mOutputFile.write( header.str().data(), header.str().size() );

  if( !mOutputFile )
    bcierr << "Error writing to file " << dataFileName << endl;

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
BCIDatFileWriter::StopRun()
{
  mOutputFile.close();
  mOutputFile.clear();
}

template<SignalType::Type T>
void
BCIDatFileWriter::PutBlock( const GenericSignal& inSignal, const STATEVECTOR& inStatevector )
{
  // Note that the order of Elements and Channels differs from the one in the
  // socket protocol.
  for( size_t j = 0; j < inSignal.Elements(); ++j )
  {
    for( size_t i = 0; i < inSignal.Channels(); ++i )
      inSignal.PutValueBinary<T>( mOutputFile, i, j );
    mOutputFile.write( inStatevector.Data(),
                       inStatevector.Length() );
  }
}

void
BCIDatFileWriter::Write( const GenericSignal& inSignal, const STATEVECTOR& inStatevector )
{
  switch( mInputProperties.Type() )
  {
    case SignalType::int16:
      PutBlock<SignalType::int16>( inSignal, inStatevector );
      break;

    case SignalType::float32:
      PutBlock<SignalType::float32>( inSignal, inStatevector );
      break;

    case SignalType::int32:
      PutBlock<SignalType::int32>( inSignal, inStatevector );
      break;

    default:
      bcierr << "Unsupported signal data type" << endl;
  }
  if( !mOutputFile )
    bcierr << "Error writing to file" << endl;
  State( "Recording" ) = ( mOutputFile ? 1 : 0 );
}

