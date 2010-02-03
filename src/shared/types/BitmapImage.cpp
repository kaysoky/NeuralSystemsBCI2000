////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A bitmap image that reads/writes itself to/from a stream in
//   run-length encoding.
//   BitmapImages support subtraction and addition to allow for efficient
//   transmission of differences between subsequent frames.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
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

