////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Platform-independent GUI data structures and functions.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GUI_H
#define GUI_H

#include "Color.h"

namespace GUI
{

// A point in relative continuous coordinates
// (range 0..1 is visible in an object's associated GraphDisplay).
struct Point
{
  float x, y;

  Point operator+( const Point& p )
  {
    Point r = { x + p.x, y + p.y };
    return r;
  }

  Point operator-( const Point& p )
  {
    Point r = { x - p.x, y - p.y };
    return r;
  }

};

// A rectangle in continuous coordinates.
struct Rect
{
  float left, top, right, bottom;
};

// Test whether a point is contained in a rectangle.
bool
PointInRect( const Point& p, const Rect& r )
{
  return p.x >= r.left
      && p.y >= r.top
      && p.x < r.right
      && p.y < r.bottom;
}

// Test whether a rectangle is empty.
bool
EmptyRect( const Rect& r )
{
  return r.left >= r.right || r.top >= r.bottom;
}

// A draw context consists of an OS-dependent GUI device handle, and a target
// rectangle in that device's coordinates.
// For Win32, the GUI device handle is a HDC (device context handle), not a
// window handle.
struct DrawContext
{
  void* handle;
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
#ifdef _WIN32
  TCanvas* pCanvas = new TCanvas;
  pCanvas->Handle = inDC.handle;
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
#endif // _WIN32
}

#endif // GUI_H
