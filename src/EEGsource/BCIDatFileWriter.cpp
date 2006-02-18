////////////////////////////////////////////////////////////////////////////////
// $Id$
// File: BCIDatFileWriter.cpp
//
// Date: Jun 22, 2005
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A filter that stores data into a BCI2000 dat file.
//
// $Log$
// Revision 1.5  2006/02/18 12:11:00  mellinger
// Support for EDF and GDF data formats.
//
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIDatFileWriter.h"

#include "UBCIError.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std;

// File writer filters must have a position string greater than
// that of the DataIOFilter.
RegisterFilter( BCIDatFileWriter, 1 );


BCIDatFileWriter::BCIDatFileWriter()
{
}


BCIDatFileWriter::~BCIDatFileWriter()
{
}


void
BCIDatFileWriter::Publish() const
{
  FileWriterBase::Publish();

  BEGIN_PARAMETER_DEFINITIONS
    "Storage string StorageTime= % % % % "
      "// time of beginning of data storage",
  END_PARAMETER_DEFINITIONS
}


void
BCIDatFileWriter::Preflight( const SignalProperties& Input,
                                   SignalProperties& Output ) const
{
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
  FileWriterBase::Preflight( Input, Output );
}


void
BCIDatFileWriter::Initialize2( const SignalProperties& Input,
                               const SignalProperties& Output )
{
  mInputProperties = Input;
  FileWriterBase::Initialize2( Input, Output );
}


void
BCIDatFileWriter::StartRun()
{
  FileWriterBase::StartRun();

  time_t now = ::time( NULL );
  const char* dateTime = ::ctime( &now );
  if( dateTime != NULL )
  {
    string strDateTime( dateTime, strlen( dateTime ) - 1 );
    Parameter( "StorageTime" ) = strDateTime.c_str();
  }

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

  OutputStream().write( headerLengthField.str().data(), headerLengthField.str().size() );
  OutputStream().write( header.str().data(), header.str().size() );
}


void
BCIDatFileWriter::StopRun()
{
  FileWriterBase::StopRun();
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
      inSignal.PutValueBinary<T>( OutputStream(), i, j );
    OutputStream().write( inStatevector.Data(),
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
  FileWriterBase::Write( inSignal, inStatevector );
}

