////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents a display rectangle for a set of
//   GraphObjects.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GraphDisplay.h"
#include "BitmapImage.h"

using namespace GUI;

GraphDisplay::GraphDisplay()
: mColor( RGBColor::Gray ),
  mNeedReorder( true )
{
  mContext.handle = NULL;
  mContext.rect.left = 0;
  mContext.rect.top = 0;
  mContext.rect.right = 0;
  mContext.rect.bottom = 0;
#ifdef _WIN32
  mOffscreenDC = NULL;
  mOffscreenBmp = NULL;
#endif // _WIN32
}

GraphDisplay&
GraphDisplay::Update() const
{
  if( mContext.handle != NULL )
  {
#ifdef _WIN32
    HWND window = ::WindowFromDC( mContext.handle );
    if( window != NULL )
      ::UpdateWindow( window );
#endif // _WIN32
  }
  return *const_cast<GraphDisplay*>( this );
}

void
GraphDisplay::Change()
{
#ifdef _WIN32
  ClearOffscreenBuffer();
#endif // _WIN32

  for( SetOfGraphObjects::iterator i = mObjects.begin(); i != mObjects.end(); ++i )
    ( *i )->Change();
}

void
GraphDisplay::Paint( void* inRegionHandle )
{
  if( mContext.handle != NULL )
  {
#ifdef _WIN32
    int formatFlags = PFD_SUPPORT_GDI;
    HDC outputDC = mContext.handle,
        drawDC = outputDC;
    int width = mContext.rect.right - mContext.rect.left,
        height = mContext.rect.bottom - mContext.rect.top;
    switch( ::GetObjectType( outputDC ) )
    {
      case OBJ_METADC:
      case OBJ_ENHMETADC:
        break;

      default:
      { // Adapt to various aspects of the DC's pixel format.
        int formatID = ::GetPixelFormat( outputDC );
        PIXELFORMATDESCRIPTOR pfd;
        if( formatID > 0 && ::DescribePixelFormat( outputDC, formatID, sizeof( pfd ), &pfd ) )
          formatFlags = pfd.dwFlags;

        if( ( formatFlags & PFD_SUPPORT_GDI ) && !( formatFlags & PFD_DOUBLEBUFFER ) )
        {
          if( mOffscreenDC == NULL )
            mOffscreenDC = ::CreateCompatibleDC( outputDC );
          if( mOffscreenBmp == NULL )
            mOffscreenBmp = ::CreateCompatibleBitmap( outputDC, width, height );
          ::DeleteObject( ::SelectObject( mOffscreenDC, mOffscreenBmp ) );
          if( inRegionHandle != NULL )
            ::SelectClipRgn( mOffscreenDC, inRegionHandle );

          if( formatFlags & PFD_SUPPORT_OPENGL )
            ::SetPixelFormat( mOffscreenDC, formatID, &pfd );

          drawDC = mOffscreenDC;
        }
      }
    }
    if( inRegionHandle != NULL )
      ::SelectClipRgn( outputDC, inRegionHandle );

    if( ( formatFlags & PFD_SUPPORT_GDI ) && !( formatFlags & PFD_SUPPORT_OPENGL ) )
    {
      RECT winRect =
      {
        mContext.rect.left,
        mContext.rect.top,
        mContext.rect.right,
        mContext.rect.bottom
      };
      HBRUSH brush = ::CreateSolidBrush( mColor.ToWinColor() );
      ::FillRect( drawDC, &winRect, brush );
      ::DeleteObject( brush );
    }

    mContext.handle = drawDC;
#endif // _WIN32

    Reorder();
    for( SetOfGraphObjects::iterator i = mObjects.begin(); i != mObjects.end(); ++i )
      ( *i )->Paint();

#ifdef _WIN32
    // Copy the data from the buffer into the target device context (usually a window).
    if( mOffscreenBmp )
    {
      ::BitBlt( outputDC,
                mContext.rect.left,
                mContext.rect.top,
                mContext.rect.right - mContext.rect.left,
                mContext.rect.bottom - mContext.rect.top,
                drawDC,
                0,
                0,
                SRCCOPY
      );
    }
    if( formatFlags & PFD_DOUBLEBUFFER )
      ::SwapBuffers( outputDC );

    mContext.handle = outputDC;
#endif // _WIN32
  }
}

void
GraphDisplay::Click( int inX, int inY )
{
  float width = mContext.rect.right - mContext.rect.left,
        height = mContext.rect.bottom - mContext.rect.top;
  GUI::Point p = {
    ( inX - mContext.rect.left ) / width,
    ( inY - mContext.rect.top ) / height
  };
  Reorder();
  for( SetOfGraphObjects::iterator i = mObjects.begin(); i != mObjects.end(); ++i )
    if( ( *i )->Visible() && ( *i )->Click( p ) )
      mObjectsClicked.push( *i );
}

GraphDisplay&
GraphDisplay::Invalidate() const
{
  Rect rect = { 0, 0, 1, 1 };
  return InvalidateRect( rect );
}

GraphDisplay&
GraphDisplay::InvalidateRect( const GUI::Rect& inRect ) const
{
  if( mContext.handle != NULL )
  {
#ifdef _WIN32
    HWND window = ::WindowFromDC( mContext.handle );
    const Rect& devRect = NormalizedToPixelCoords( inRect );
    RECT rect =
    {
      devRect.left,
      devRect.top,
      devRect.right,
      devRect.bottom
    };
    ::InvalidateRect( window, &rect, false );
#endif // _WIN32
  }
  return *const_cast<GraphDisplay*>( this );
}

GUI::Rect
GraphDisplay::NormalizedToPixelCoords( const GUI::Rect& inRect ) const
{
  float width = mContext.rect.right - mContext.rect.left,
        height = mContext.rect.bottom - mContext.rect.top;
  GUI::Rect result =
  {
    mContext.rect.left + width  * inRect.left,
    mContext.rect.top  + height * inRect.top,
    mContext.rect.left + width  * inRect.right,
    mContext.rect.top  + height * inRect.bottom
  };
  return result;
}

GUI::Rect
GraphDisplay::PixelToNormalizedCoords( const GUI::Rect& inRect ) const
{
  float width = mContext.rect.right - mContext.rect.left,
        height = mContext.rect.bottom - mContext.rect.top;
  GUI::Rect result =
  {
    ( inRect.left - mContext.rect.left ) / width,
    ( inRect.top - mContext.rect.top ) / height,
    ( inRect.right - mContext.rect.left ) / width,
    ( inRect.bottom - mContext.rect.top ) / height
  };
  return result;
}

void
GraphDisplay::Reorder()
{
  if( mNeedReorder )
  {
    SetOfGraphObjects tmp;
    for( SetOfGraphObjects::const_iterator i = mObjects.begin(); i != mObjects.end(); ++i )
      tmp.insert( *i );
    mObjects = tmp;
    mNeedReorder = false;
  }
}

const class BitmapImage&
GraphDisplay::BitmapData( int inWidth, int inHeight ) const
{
  int width = inWidth,
      height = inHeight,
      originalWidth = mContext.rect.right - mContext.rect.left,
      originalHeight = mContext.rect.bottom - mContext.rect.top;
  if( width == 0 && height == 0 )
  {
    width = originalWidth;
    height = originalHeight;
  }
  static BitmapImage image;
  image = BitmapImage( width, height );
#ifdef _WIN32
  HDC sourceDC = mOffscreenDC ? mOffscreenDC : mContext.handle;
  if( sourceDC != NULL )
  {
    HDC miniDC = ::CreateCompatibleDC( sourceDC );
    HBITMAP miniBmp = ::CreateCompatibleBitmap( sourceDC, width, height );
    ::DeleteObject( ::SelectObject( miniDC, miniBmp ) );
    // STRETCH_DELETESCANS is the only option that ignores intermediate pixels,
    // thus is considerably faster.
    ::SetStretchBltMode( miniDC, STRETCH_DELETESCANS );
    ::StretchBlt(
      miniDC, 0, 0, width, height,
      sourceDC, 0, 0, originalWidth, originalHeight,
      SRCCOPY
    );

    BITMAPINFO info;
    ::memset( &info, 0, sizeof( BITMAPINFO ) );
    info.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = 0;
    uint32* pBitmapData = new uint32[ width * height ];
    int result = ::GetDIBits(
                     miniDC, miniBmp,
                     0, height,
                     pBitmapData,
                     &info, DIB_RGB_COLORS
                   );
    if( result > 0 )
      for( int x = 0; x < width; ++x )
        for( int y = 0; y < height; ++y )
          image( x, y ) = RGBColor( pBitmapData[ x + y * width ] & 0xffffff );
    delete[] pBitmapData;
    ::DeleteDC( miniDC );
    ::DeleteObject( miniBmp );
  }
#endif // _WIN32
  return image;
}

#ifdef _WIN32
void
GraphDisplay::ClearOffscreenBuffer()
{
  if( mOffscreenDC != NULL )
  {
    ::DeleteDC( mOffscreenDC );
    mOffscreenDC = NULL;
  }
  if( mOffscreenBmp != NULL )
  {
    ::DeleteObject( mOffscreenBmp );
    mOffscreenBmp = NULL;
  }
}
#endif // _WIN32
