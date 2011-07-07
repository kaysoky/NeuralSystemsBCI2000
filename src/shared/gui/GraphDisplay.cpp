////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents a display rectangle for a set of
//   GraphObjects.
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

#ifndef __BORLANDC__
# include <QPainter>
# include <QWidget>
# include <QImage>
# include <QPixmap>
#endif // __BORLANDC__

#include "GraphDisplay.h"
#include "BitmapImage.h"
#ifdef QT_OPENGL_LIB
# include "Scene.h"
#endif // QT_OPENGL_LIB

#include <algorithm>

using namespace GUI;
using namespace std;

GraphDisplay::GraphDisplay()
: mColor( RGBColor::Gray )
{
  mContext.rect.left = 0;
  mContext.rect.top = 0;
  mContext.rect.right = 0;
  mContext.rect.bottom = 0;
#ifdef __BORLANDC__
  mContext.handle = NULL;
  mOffscreenDC = NULL;
#else // __BORLANDC__
  mContext.handle.device = NULL;
  mContext.handle.painter = NULL;
#endif // __BORLANDC__
  mOffscreenBmp = NULL;
}

const GraphDisplay&
GraphDisplay::Update() const
{
#ifdef __BORLANDC__
  if( mContext.handle != NULL )
  {
#ifdef _WIN32
    HWND window = ::WindowFromDC( ( HDC )mContext.handle );
    if( window != NULL )
      ::UpdateWindow( window );
#endif // _WIN32
  }
#else // __BORLANDC__
  QWidget* pWidget = dynamic_cast< QWidget* >( mContext.handle.device );
  if( pWidget )
    pWidget->repaint( mInvalidRegion );
#endif // __BORLANDC__
  return *this;
}

void
GraphDisplay::Change()
{
  ClearOffscreenBuffer();
  Invalidate();

  for( SetOfGraphObjects::iterator i = mObjects.begin(); i != mObjects.end(); ++i )
    ( *i )->Change();
}

void
GraphDisplay::Paint( const void* inRegionHandle )
{
#ifdef __BORLANDC__
  int formatFlags = PFD_SUPPORT_GDI;
  HDC outputDC = (HDC)mContext.handle,
      drawDC = outputDC;
  int width = mContext.rect.right,
      height = mContext.rect.bottom;

  if( mContext.handle != NULL )
  {
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
            ::SelectClipRgn( mOffscreenDC, (HRGN)inRegionHandle );

          if( formatFlags & PFD_SUPPORT_OPENGL )
            ::SetPixelFormat( mOffscreenDC, formatID, &pfd );

          drawDC = mOffscreenDC;
        }
      }
    }
    if( inRegionHandle != NULL )
      ::SelectClipRgn( outputDC, (HRGN)inRegionHandle );
  } 
  else 
  {
    if( mOffscreenDC == NULL )
      mOffscreenDC = ::CreateCompatibleDC( NULL );
    if( mOffscreenBmp == NULL )
      mOffscreenBmp = ::CreateBitmap( width, height, 1, GetDeviceCaps( mOffscreenDC, BITSPIXEL ), NULL );
    ::DeleteObject( ::SelectObject( mOffscreenDC, mOffscreenBmp ) );

    drawDC = mOffscreenDC;
  }

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

#else // __BORLANDC__

  // Create the QPixmap for the background buffer
  if( mOffscreenBmp == NULL )
  {
    int width = static_cast<int>( mContext.rect.right - mContext.rect.left ),
        height = static_cast<int>( mContext.rect.bottom - mContext.rect.top );
    QImage im( width, height, QImage::Format_ARGB32_Premultiplied );
    mOffscreenBmp = new QPixmap();
    *mOffscreenBmp = QPixmap::fromImage( im );
  }

  if( mColor == RGBColor::NullColor )
    mOffscreenBmp->fill( Qt::transparent );

  // Set up the painter object used by the objects' drawing code.
  QPainter* pOffscreenPainter = new QPainter( mOffscreenBmp );
  pOffscreenPainter->translate( -mContext.rect.left, -mContext.rect.top );
  pOffscreenPainter->setClipping( true );
  pOffscreenPainter->setClipRegion( mInvalidRegion );

  QRect rect;
  rect.setLeft( static_cast<int>( mContext.rect.left ) );
  rect.setTop( static_cast<int>( mContext.rect.top ) );
  rect.setRight( static_cast<int>( mContext.rect.right ) );
  rect.setBottom( static_cast<int>( mContext.rect.bottom ) );

  // Create a rect for the background color
  if( mColor != RGBColor::NullColor )
  {
    // Create the color to brush
    QColor brushColor( mColor.R(), mColor.G(), mColor.B() );

    // Create a brush to color
    QBrush brush( brushColor );
    brush.setStyle( Qt::SolidPattern );

    // Paint the background color
    pOffscreenPainter->fillRect( rect, brush ); 
  }

  // Set the backbuffer draw context
  mContext.handle.painter = pOffscreenPainter;

#endif // __BORLANDC__

  vector<GraphObject*> objects( mObjects.begin(), mObjects.end() );
  sort( objects.begin(), objects.end(), GraphObject::CompareByZOrder() );
  for( vector<GraphObject*>::iterator i = objects.begin(); i != objects.end(); ++i )
    ( *i )->Paint();

#ifndef __BORLANDC__

  mContext.handle.painter = NULL;
  delete pOffscreenPainter;

#endif // __BORLANDC__

#ifdef __BORLANDC__

  // Copy the data from the buffer into the target device context (usually a window).
  if( mContext.handle != NULL )
  {
    if( mOffscreenBmp )
    {
      ::BitBlt( outputDC,
                0,
                0,
                mContext.rect.right,
                mContext.rect.bottom,
                drawDC,
                0,
                0,
                SRCCOPY
      );
    }
    if( formatFlags & PFD_DOUBLEBUFFER )
      ::SwapBuffers( outputDC );

    mContext.handle = outputDC;
  }

#else // __BORLANDC__

  if( mContext.handle.device != NULL )
  {
    // Output the offscreen pixmap to the screen
    QPainter outputPainter( mContext.handle.device );
    outputPainter.drawPixmap( static_cast<int>( mContext.rect.left ),
                              static_cast<int>( mContext.rect.top ),
                              *mOffscreenBmp ); 
  }
  
  // Clear the invalid region.
  mInvalidRegion = QRegion();

#endif // __BORLANDC__
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
  vector<GraphObject*> objects( mObjects.begin(), mObjects.end() );
  sort( objects.begin(), objects.end(), GraphObject::CompareByZOrder() );
  for( vector<GraphObject*>::iterator i = objects.begin(); i != objects.end(); ++i )
    if( ( *i )->Visible() && ( *i )->Click( p ) )
      mObjectsClicked.push( *i );
}

GraphDisplay&
GraphDisplay::Invalidate()
{
  Rect rect = { 0, 0, 1, 1 };
  return InvalidateRect( rect );
}

GraphDisplay&
GraphDisplay::InvalidateRect( const GUI::Rect& inRect )
{
#ifdef __BORLANDC__
  if( mContext.handle != NULL )
  {
#ifdef _WIN32
    HWND window = ::WindowFromDC( (HDC)mContext.handle );
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
#else // __BORLANDC__
  Rect rt = NormalizedToPixelCoords( inRect );
  QRegion rgn(
      static_cast<int>( rt.left ),
      static_cast<int>( rt.top ),
      static_cast<int>( rt.right - rt.left ),
      static_cast<int>( rt.bottom - rt.top )
  );
  mInvalidRegion += rgn;

  QWidget* pWidget = dynamic_cast< QWidget* >( mContext.handle.device );
  if( pWidget )
    pWidget->update( rgn );
#endif // __BORLANDC__
  return *this;
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
  const GUI::Rect unitRect = { 0, 0, 1, 1 };
  if( EmptyRect( mContext.rect ) )
    return unitRect;

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

const class BitmapImage&
GraphDisplay::BitmapData( int inWidth, int inHeight ) const
{
  int width = inWidth,
      height = inHeight,
      originalWidth = static_cast<int>( mContext.rect.right - mContext.rect.left ),
      originalHeight = static_cast<int>( mContext.rect.bottom - mContext.rect.top );
  if( width == 0 && height == 0 )
  {
    width = originalWidth;
    height = originalHeight;
  }
  static BitmapImage image;
  image = BitmapImage( width, height );

#ifdef __BORLANDC__

  HDC sourceDC = mOffscreenDC ? mOffscreenDC : (HDC)mContext.handle;
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

#else // __BORLANDC__

  if( mOffscreenBmp != NULL )
  {
#ifdef QT_OPENGL_LIB
    for( SetOfGraphObjects::const_iterator i = mObjects.begin(); i != mObjects.end(); ++i )
    { // OpenGL-based scenes are not rendered into the offscreen buffer, and require
      // a different approach to retrieving pixel data.
      Scene* pScene = dynamic_cast<Scene*>( *i );
      if( pScene )
      {
        Rect r = NormalizedToPixelCoords( pScene->DisplayRect() );
        QImage sceneImage = const_cast<GLScene&>( pScene->GLScene() ).grabFrameBuffer();
        QPainter p( mOffscreenBmp );
        p.drawImage(
          static_cast<int>( r.left ),
          static_cast<int>( r.top ),
          sceneImage
        );
      }
    }
#endif // QT_OPENGL_LIB
    QImage img = mOffscreenBmp->scaled( width, height ).toImage();
    for( int x = 0; x < width; x++ )
    {
      for( int y = 0; y < height; y++ )
      {
        QColor color = QColor::fromRgba( img.pixel( x, y ) );
        if( color.alpha() == 0 )
          image( x, y ) = RGBColor::NullColor;
        else
          image( x, y ) = RGBColor( color.red(), color.green(), color.blue() );
      }
    }
  }

#endif // __BORLANDC__
  return image;
}

void
GraphDisplay::ClearOffscreenBuffer()
{
#ifdef __BORLANDC__

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

#else // __BORLANDC__

  if( mOffscreenBmp != NULL )
  {
    delete mOffscreenBmp;
    mOffscreenBmp = NULL;
  }
#endif // __BORLANDC__
}
