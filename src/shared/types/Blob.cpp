////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A type for binary large objects (BLOBs) which may be embedded
//   into BCI2000 data streams.
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

#include "Blob.h"
#include "FileUtils.h"
#include "StringUtils.h"

#include <fstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

Blob::Blob( const string& inFileName )
: mFileName( inFileName ),
  mpData( 0 ), mLength( 0 ),
  mType( FileUtils::ExtractExtension( inFileName ) ),
  mpTempFile( 0 )
{
  if( !mType.empty() && *mType.begin() == '.' )
    mType = mType.substr( 1 );

  ifstream in( inFileName.c_str(), ios::binary );
  if( !in.is_open() )
    throw runtime_error( "Could not open file for reading: " + inFileName );
  in.seekg( 0, ios::end );
  mLength = static_cast<size_t>( in.tellg() );
}

Blob::Blob( const bci::Resource& inResource )
: mpData( inResource.data ), mLength( inResource.length ), mType( inResource.type ),
  mpTempFile( 0 )
{
}

Blob::Blob( const char* data, size_t length, const string& type )
: mpData( data ), mLength( length ),
  mType( type ),
  mpTempFile( 0 )
{
}

Blob::~Blob()
{
  delete mpTempFile;
}

void
Blob::GetStringData() const
{
  if( mpData )
  {
    if( mpData != mStringData.data() )
    {
      mStringData = string( mpData, mLength );
      mpData = mStringData.data();
    }
  }
  else if( mStringData.empty() )
  {
    mStringData.reserve( mLength );
    ifstream is( mFileName.c_str(), ios::binary );
    mStringData.assign( istreambuf_iterator<char>( is ), istreambuf_iterator<char>() );
  }
}

bool
Blob::SaveAs( const string& inFileName ) const
{
  string newFile = inFileName;
  if( FileUtils::ExtractExtension( newFile ) != "." + mType )
    newFile += "." + mType;
  if( FileUtils::CanonicalPath( newFile ) == FileUtils::CanonicalPath( mFileName ) )
    return true;

  ofstream out( newFile.c_str(), ios::binary );
  if( WriteBinary( out ) )
    mFileName = inFileName;
  return out;
}

string
Blob::SaveAsTemp() const
{
  if( !mpTempFile )
  {
    mpTempFile = new FileUtils::TemporaryFile;
    if( WriteBinary( *mpTempFile ) )
      mFileName = mpTempFile->Name();
    mpTempFile->Close();
  }
  return mFileName;
}

ostream&
Blob::WriteAsResource( ostream& os ) const
{
  GetStringData();
  string indent( static_cast<size_t>( os.width() ), os.fill() );
  ios::fmtflags format = os.flags();
  os << dec << indent << "\"" << mType << "\", " << mLength << ",\n" << indent << "\""
     << oct;
  streampos pos = os.tellp();
  bool wasEncoded = false;
  for( size_t i = 0; i < mLength; ++i )
  {
    if( ( os.tellp() - pos ) >= 80 )
    {
      os << "\"\n" << indent << "\"";
      pos = os.tellp();
      wasEncoded = false;
    }
    const unsigned char c = static_cast<unsigned char>( mStringData[i] );
    bool encode = !::isprint( c ) || ( '0' <= c && c <= '7' && wasEncoded );
    if( !encode )
      switch( mStringData[i] )
      {
        case '\\':
        case '"':
          encode = true;
          break;
      }
    if( encode )
      os << '\\' << static_cast<int>( c );
    else
      os << mStringData[i];
    wasEncoded = encode;
  }
  os << "\"";
  os.flags( format );
  return os;
}

ostream&
Blob::WriteBinary( ostream& os ) const
{
  if( mpData )
    os.write( mpData, mLength );
  else if( mStringData.length() == mLength )
    os.write( mStringData.data(), mLength );
  else if( !mFileName.empty() )
  {
    ifstream in( mFileName.c_str(), ios::binary );
    if( !in.is_open() )
      os.setstate( ios::failbit );
    os << in.rdbuf();
  }
  else
    os.setstate( ios::failbit );
  return os;
}

ostream&
Blob::WriteToStream( ostream& os ) const
{
  os << mType << ":";
  GetStringData();
  return StringUtils::WriteAsBase64( os, mStringData );
}

istream&
Blob::ReadFromStream( istream& is )
{
  getline( is, mType, ':' );
  mpData = 0;
  delete mpTempFile;
  mpTempFile = 0;
  if( StringUtils::ReadAsBase64( is, mStringData, &::isspace ) )
    mpData = mStringData.data();
  return is;
}
