////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An output class for the BCI2000 dat format.
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

#include "BCI2000OutputFormat.h"

#include "BCIError.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std;

void
BCI2000OutputFormat::Publish() const
{
}


void
BCI2000OutputFormat::Preflight( const SignalProperties& inProperties,
                                const StateVector& ) const
{
  Parameter( "SourceCh" );

  switch( inProperties.Type() )
  {
    case SignalType::int16:
    case SignalType::int32:
    case SignalType::float32:
      /* These types are OK */
      break;

    default:
      bcierr << inProperties.Type().Name()
             << " data type unsupported for BCI2000 files"
             << endl;
  }
}

void
BCI2000OutputFormat::Initialize( const SignalProperties& inProperties,
                                 const StateVector& inStatevector )
{
  mInputProperties = inProperties;
  mStatevectorLength = inStatevector.Length();
}

void
BCI2000OutputFormat::StartRun( ostream& os, const string& )
{
  // We write 16 bit data in the old format to maintain backward compatibility.
  bool useOldFormat = ( mInputProperties.Type() == SignalType::int16 );

  // Write the header.
  //
  // Because the header contains its own length in ASCII format, it is a bit
  // tricky to get this right if we don't want to imply a fixed width for the
  // HeaderLen field.
  std::ostringstream header;
  header << " "
         << "SourceCh= " << ( int )Parameter( "SourceCh" ) << " "
         << "StatevectorLen= " << mStatevectorLength;
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

  std::string headerBegin;
  if( !useOldFormat )
    headerBegin = "BCI2000V= 1.1 ";
  headerBegin += "HeaderLen= ";
  size_t fieldLength = 5; // Follow the old scheme
                          // (5 characters for the header length field),
                          // but allow for a longer HeaderLen field
                          // if necessary.
  std::ostringstream headerLengthField;
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

  os.write( headerLengthField.str().data(), headerLengthField.str().size() );
  os.write( header.str().data(), header.str().size() );
}

void
BCI2000OutputFormat::StopRun( ostream& )
{
}


template<SignalType::Type T>
void
BCI2000OutputFormat::PutBlock( ostream& os, const GenericSignal& inSignal, const StateVector& inStatevector )
{
  // Note that the order of Elements and Channels differs from the one in the
  // socket protocol.
  for( int j = 0; j < inSignal.Elements(); ++j )
  {
    for( int i = 0; i < inSignal.Channels(); ++i )
      inSignal.PutValueBinary<T>( os, i, j );
    os.write(
      reinterpret_cast<const char*>( inStatevector( min( j, inStatevector.Samples() - 1 ) ).Data() ),
      inStatevector.Length()
    );
  }
}

void
BCI2000OutputFormat::Write( ostream& os, const GenericSignal& inSignal, const StateVector& inStatevector )
{
  switch( mInputProperties.Type() )
  {
    case SignalType::int16:
      PutBlock<SignalType::int16>( os, inSignal, inStatevector );
      break;

    case SignalType::float32:
      PutBlock<SignalType::float32>( os, inSignal, inStatevector );
      break;

    case SignalType::int32:
      PutBlock<SignalType::int32>( os, inSignal, inStatevector );
      break;

    default:
      bcierr << "Unsupported signal data type" << endl;
  }
}
