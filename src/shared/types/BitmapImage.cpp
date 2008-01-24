////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A bitmap image that reads/writes itself to/from a stream in
//   run-length encoding.
//   BitmapImages support subtraction and addition to allow for efficient
//   transmission of differences between subsequent frames.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "BitmapImage.h"

using namespace std;

ostream&
BitmapImage::WriteBinary( ostream& os ) const
{
  os.put( mWidth & 0xff ).put( mWidth >> 16 )
    .put( mHeight & 0xff ).put( mHeight >> 16 );

  uint16* pData = mpData,
        * pEnd = mpData + mWidth * mHeight;
  while( pData < pEnd )
  {
    int value = *pData,
        length = 0;
    while( pData < pEnd && *pData == value && length <= 256 )
      ++pData, ++length;
    os.put( length & 0xff ).put( value & 0xff ).put( value >> 16 );
  }
  return os;
}

istream&
BitmapImage::ReadBinary( istream& is )
{
  mWidth = uint8( is.get() ) | ( uint8( is.get() ) << 16 );
  mHeight = uint8( is.get() ) | ( uint8( is.get() ) << 16 );
  delete[] mpData;
  mpData = new uint16[ mWidth * mHeight ];

  uint16* pData = mpData,
        * pEnd = mpData + mWidth * mHeight;
  while( pData < pEnd )
  {
    int length = uint8( is.get() );
    int value = uint8( is.get() ) | ( uint8( is.get() ) << 16 );
    for( int i = 0; i <= length && pData < pEnd; ++i )
      *pData++ = value;
  }
  return is;
}

