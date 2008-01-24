////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A FileWriter filter that stores data into a BCI2000 dat file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCI2000FileWriter.h"

#include "BCIError.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std;

// File writer filters must have a position string greater than
// that of the DataIOFilter.
RegisterFilter( BCI2000FileWriter, 1 );


BCI2000FileWriter::BCI2000FileWriter()
{
}


BCI2000FileWriter::~BCI2000FileWriter()
{
}


void
BCI2000FileWriter::Publish() const
{
  FileWriterBase::Publish();

  BEGIN_PARAMETER_DEFINITIONS
    "Storage string StorageTime= % % % % "
      "// time of beginning of data storage",
  END_PARAMETER_DEFINITIONS
}


void
BCI2000FileWriter::Preflight( const SignalProperties& Input,
                                    SignalProperties& Output ) const
{
  if( !string( Parameter( "StorageTime" ) ).empty() )
    bciout << "The StorageTime parameter will be overwritten with the"
           << " recording's actual date and time"
           << endl;

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
BCI2000FileWriter::Initialize( const SignalProperties& Input,
                                const SignalProperties& Output )
{
  mInputProperties = Input;
  FileWriterBase::Initialize( Input, Output );
}


void
BCI2000FileWriter::StartRun()
{
  FileWriterBase::StartRun();

  time_t now = ::time( NULL );
  const char* dateTime = ::ctime( &now );
  if( dateTime != NULL )
  {
    string strDateTime( dateTime, strlen( dateTime ) - 1 );
    Parameter( "StorageTime" ) = strDateTime;
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
         << "SourceCh= " << ( int )Parameter( "SourceCh" ) << " "
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
BCI2000FileWriter::StopRun()
{
  FileWriterBase::StopRun();
}


template<SignalType::Type T>
void
BCI2000FileWriter::PutBlock( const GenericSignal& inSignal, const StateVector& inStatevector )
{
  // Note that the order of Elements and Channels differs from the one in the
  // socket protocol.
  for( int j = 0; j < inSignal.Elements(); ++j )
  {
    for( int i = 0; i < inSignal.Channels(); ++i )
      inSignal.PutValueBinary<T>( OutputStream(), i, j );
    OutputStream().write( reinterpret_cast<const char*>( inStatevector.Data() ),
                       inStatevector.Length() );
  }
}


void
BCI2000FileWriter::Write( const GenericSignal& inSignal, const StateVector& inStatevector )
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

