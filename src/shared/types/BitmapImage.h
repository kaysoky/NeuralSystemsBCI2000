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
#ifndef BITMAP_IMAGE_H
#define BITMAP_IMAGE_H

#include <iostream>
#include <cstring>
#include "Color.h"
#include "defines.h"

class BitmapImage
{
 public:
  class PixelRef;
  friend PixelRef;

  class PixelRef
  {
   public:
    PixelRef()
      : mpData( NULL )
      {}
    PixelRef& PointTo( BitmapImage* pBitmapImage, int x, int y )
      { mpData = pBitmapImage->mpData + pBitmapImage->mWidth * y + x; return *this; }
    bool IsBlack() const
      {
        return *mpData == 0;
      }
    operator RGBColor() const
      {
        int r = *mpData >> 8 & 0xf0,
            g = *mpData      & 0xf0,
            b = *mpData << 8 & 0xf0;
        return RGBColor( r, g, b );
      }
    PixelRef& operator=( const RGBColor& c )
      {
        int r = c >> 20 & 0xf,
            g = c >> 12 & 0xf,
            b = c >>  4 & 0xf;
        *mpData = r << 8 | g << 4 | b;
        return *this;
      }

   private:
    uint16* mpData;
  };

 public:
  BitmapImage( int inWidth = 0, int inHeight = 0 )
    : mWidth( inWidth ),
      mHeight( inHeight ),
      mpData( new uint16[ inWidth * inHeight ] )
    {}
  BitmapImage( const BitmapImage& b )
    : mWidth( b.mWidth ),
      mHeight( b.mHeight ),
      mpData( new uint16[ mWidth * mHeight ] )
    {
      std::memcpy( mpData, b.mpData, mWidth * mHeight * sizeof( *mpData ) );
    }
  ~BitmapImage()
    { delete[] mpData; }

  BitmapImage& operator=( const BitmapImage& b )
    {
      if( &b != this )
      {
        delete[] mpData;
        mWidth = b.mWidth;
        mHeight = b.mHeight;
        mpData = new uint16[ mWidth * mHeight ];
        std::memcpy( mpData, b.mpData, mWidth * mHeight * sizeof( *mpData ) );
      }
      return *this;
    }

  bool Empty() const
    {
      return ( mWidth == 0 ) || ( mHeight == 0 );
    }

  int Width() const
    {
      return mWidth;
    }

  int Height() const
    {
      return mHeight;
    }

  BitmapImage& SetBlack()
    {
      std::memset( mpData, 0, mWidth * mHeight * sizeof( *mpData ) );
      return *this;
    }

  const PixelRef& operator()( int x, int y ) const
    {
      static PixelRef ref;
      return ref.PointTo( const_cast<BitmapImage*>( this ), x, y );
    }
  PixelRef& operator()( int x, int y )
    {
      static PixelRef ref;
      return ref.PointTo( this, x, y );
    }

  BitmapImage& operator+=( const BitmapImage& b )
    {
      for( int i = 0; i < b.mWidth * b.mHeight; ++i )
        mpData[ i ] += b.mpData[ i ];
      return *this;
    }
  BitmapImage& operator-=( const BitmapImage& b )
    {
      for( int i = 0; i < b.mWidth * b.mHeight; ++i )
        mpData[ i ] -= b.mpData[ i ];
      return *this;
    }
  BitmapImage operator-( const BitmapImage& b ) const
    {
      return BitmapImage( *this ) -= b;
    }
  BitmapImage operator+( const BitmapImage& b ) const
    {
      return BitmapImage( *this ) += b;
    }

  std::ostream& WriteBinary( std::ostream& ) const;
  std::istream& ReadBinary( std::istream& );

 private:
  int     mWidth,
          mHeight;
  uint16* mpData;
};

#endif // BITMAP_IMAGE_H

