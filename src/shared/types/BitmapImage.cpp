////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A bitmap image that reads/writes itself to/from a stream in
//   run-length encoding.
//   BitmapImages support subtraction and addition to allow for efficient
//   transmission of differences between subsequent frames.
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

#include "BitmapImage.h"

using namespace std;

ostream&
BitmapImage::WriteBinary( ostream& os ) const
{
  os.put( mWidth & 0xff ).put( mWidth >> 8 )
    .put( mHeight & 0xff ).put( mHeight >> 8 );

  uint16* pData = mpData,
        * pEnd = mpData + mWidth * mHeight;
  while( pData < pEnd )
  {
    int value = *pData,
        length = 0;
    while( pData < pEnd && *pData == value && length < 0x100 )
      ++pData, ++length;
    os.put( --length & 0xff ).put( value & 0xff ).put( value >> 8 );
  }
  return os;
}

istream&
BitmapImage::ReadBinary( istream& is )
{
  mWidth = uint8( is.get() ) | ( uint8( is.get() ) << 8 );
  mHeight = uint8( is.get() ) | ( uint8( is.get() ) << 8 );
  delete[] mpData;
  mpData = new uint16[ mWidth * mHeight ];

  uint16* pData = mpData,
        * pEnd = mpData + mWidth * mHeight;
  while( pData < pEnd )
  {
    int length_1 = uint8( is.get() );
    int value = uint8( is.get() ) | ( uint8( is.get() ) << 8 );
    for( int i = 0; i <= length_1 && pData < pEnd; ++i )
      *pData++ = value;
  }
  return is;
}

void
BitmapImage::DimensionCheck( const BitmapImage& b ) const
{
  if( mWidth != b.mWidth || mHeight != b.mHeight )
    throw "BitmapImage dimension mismatch";
}

