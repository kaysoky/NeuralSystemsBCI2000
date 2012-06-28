////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Platform-independent GUI data structures and functions.
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
#ifndef GUI_H
#define GUI_H

#include "Color.h"
#include "BCIException.h"

#ifdef __BORLANDC__
# include <vcl.h>
#else
# include <QPaintDevice>
# include <QPainter>
# include <QImage>
class QGLWidget;
#endif // __BORLANDC__

namespace GUI
{

// A point in relative continuous coordinates
// (range 0..1 is visible in an object's associated GraphDisplay).
struct Point
{
  float x, y;

  Point operator+( const Point& p ) const
  {
    Point r = { x + p.x, y + p.y };
    return r;
  }

  Point operator-( const Point& p ) const
  {
    Point r = { x - p.x, y - p.y };
    return r;
  }

  bool operator==( const Point& p ) const
  {
    return x == p.x && y == p.y;
  }

  bool operator!=( const Point& p ) const
  {
    return !operator==( p );
  }
};

// A rectangle in continuous coordinates.
struct Rect
{
  float left, top, right, bottom;

  float Width() const
  {
    return right - left;
  }

  float Height() const
  {
    return bottom - top;
  }

  bool operator==( const Rect& r ) const
  {
    return left == r.left
        && right == r.right
        && top == r.top
        && bottom == r.bottom;
  }

  bool operator!=( const Rect& r ) const
  {
    return !operator==( r );
  }
};

// Test whether a point is contained in a rectangle.
bool PointInRect( const Point& p, const Rect& r );
bool EmptyRect( const Rect& r );

// A draw context consists of an OS-dependent GUI device handle, and a target
// rectangle in that device's coordinates.
// For Win32, the GUI device handle is a HDC (device context handle), not a
// window handle.
// If we're using Qt, we need to pass around a QPaintDevice, as that
// is the equivalent of an HDC for a QWidget (our main window), _and_
// a QPainter because this is the only way to implement clipping when drawing
// to an offscreen buffer.
struct DrawContext
{
#ifdef __BORLANDC__
  HDC handle;
#else // __BORLANDC__
  struct
  {
    QPaintDevice* device;
    QPainter*     painter;
    QGLWidget*    glContext;
  } handle;
#endif // __BORLANDC__
  Rect  rect;
};

namespace RenderingMode
{
  enum
  {
    Transparent,
    Opaque
  };
}

namespace GraphicResource
{
  template<class T>
   int Width( const T& t )
    { return t[ 0 ].count; }
  template<class T>
   int Height( const T& t )
    { return t[ 1 ].count; }
  template<int Mode, class T>
   void Render( const T&, const DrawContext& );
};

} // namespace GUI

template<int Mode, class T>
void
GUI::GraphicResource::Render( const T& inGraphic, const GUI::DrawContext& inDC )
{
#ifdef __BORLANDC__
  TCanvas* pCanvas = new TCanvas;
  pCanvas->Handle = reinterpret_cast<HDC>( inDC.handle );
  int left = inDC.rect.left,
      right = left + Width( inGraphic ),
      run = 2,
      x = left,
      y = inDC.rect.top;
  while( inGraphic[ run ].count > 0 )
  {
    int grayValue = inGraphic[ run ].color;
    for( int i = 0; i < inGraphic[ run ].count; ++i )
    {
      switch( Mode ) // evaluated at compile time
      {
        case RenderingMode::Transparent:
        { COLORREF pixelColor = pCanvas->Pixels[x][y];
          int r = ( GetRValue( pixelColor ) * grayValue ) / 0xff,
              g = ( GetGValue( pixelColor ) * grayValue ) / 0xff,
              b = ( GetBValue( pixelColor ) * grayValue ) / 0xff;
          pCanvas->Pixels[x][y] = TColor( RGB( r, g, b ) );
        } break;
        case RenderingMode::Opaque:
          pCanvas->Pixels[x][y] = TColor( RGB( grayValue, grayValue, grayValue ) );
          break;
      }
      if( ++x >= right )
      {
        ++y;
        x = left;
      }
    }
    ++run;
  }
  delete pCanvas;
#else // __BORLANDC__
  if( inDC.handle.device != NULL )
  {
    if( Mode != RenderingMode::Opaque )
      throw bciexception( "Unsupported rendering mode: " << Mode );

    int width = Width( inGraphic ),
        height = Height( inGraphic );
    QImage image( width, height, QImage::Format_RGB888 );
    int run = 2,
        x = 0,
        y = 0;
    while( inGraphic[ run ].count > 0 )
    {
      int grayValue = inGraphic[ run ].color;
      for( int i = 0; i < inGraphic[ run ].count; ++i )
      {
        image.setPixel( x, y, qRgb( grayValue, grayValue, grayValue ) );
        if( ++x >= width )
        {
          ++y;
          x = 0;
        }
      }
      ++run;
    }
    QRect targetRect(
      static_cast<int>( inDC.rect.left ),
      static_cast<int>( inDC.rect.top ),
      static_cast<int>( inDC.rect.right - inDC.rect.left ),
      static_cast<int>( inDC.rect.bottom - inDC.rect.top )
    );
    if( inDC.handle.painter != NULL )
    {
      inDC.handle.painter->drawImage( targetRect, image );
    }
    else
    {
      QPainter p( inDC.handle.device );
      p.drawImage( targetRect, image );
    }
  }
#endif // __BORLANDC__
}

#endif // GUI_H
