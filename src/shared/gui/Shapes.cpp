////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A number of GraphObjects representing geometric shapes.
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

#include "Shapes.h"
#include <limits>

#ifdef __BORLANDC__
# include <VCL.h>
#else // __BORLANDC__
# include <QPainter>
#endif // __BORLANDC__

using namespace std;
using namespace GUI;

static const float eps = numeric_limits<float>::epsilon();

Shape::Shape( GraphDisplay& display, int zOrder )
: GraphObject( display, zOrder ),
  mLineWidth( 0.0 ),
  mColor( RGBColor::White ),
  mFillColor( RGBColor::NullColor )
{
}

Shape::~Shape()
{
}

Shape&
Shape::SetCenter( const GUI::Point& p )
{
  GUI::Point center = Center();
  float dx = p.x - center.x,
        dy = p.y - center.y;
  if( dx != 0 || dy != 0 )
  {
    GUI::Rect r = ObjectRect();
    r.left += dx;
    r.right += dx;
    r.top += dy;
    r.bottom += dy;
    SetObjectRect( r );
  }
  return *this;
}

GUI::Point
Shape::Center() const
{
  GUI::Point p =
  {
    ( ObjectRect().left + ObjectRect().right ) / 2,
    ( ObjectRect().top + ObjectRect().bottom ) / 2
  };
  return p;
}

Shape&
Shape::SetLineWidth( float f )
{
  mLineWidth = f;
  Change();
  return *this;
}

float
Shape::LineWidth() const
{
  return mLineWidth;
}

Shape&
Shape::SetFillColor( RGBColor c )
{
  mFillColor = c;
  Change();
  return *this;
}

RGBColor
Shape::FillColor() const
{
  return mFillColor;
}

Shape&
Shape::SetColor( RGBColor c )
{
  mColor = c;
  Change();
  return *this;
}

RGBColor
Shape::Color() const
{
  return mColor;
}

void
Shape::OnMove( GUI::DrawContext& ioDC )
{
  // Correct invalidation rect
  GUI::Rect corrected = {
    static_cast<int>( ioDC.rect.left - ( this->LineWidth() / 2.0f ) ),
    static_cast<int>( ioDC.rect.top - ( this->LineWidth() / 2.0f ) ),
    static_cast<int>( ioDC.rect.right + ( this->LineWidth() / 2.0f ) + 1 ),
    static_cast<int>( ioDC.rect.bottom + ( this->LineWidth() / 2.0f ) + 1 ) 
  };

  mRect = ioDC.rect;
  ioDC.rect = corrected;
}

bool
Shape::AreaIntersection( const Shape& s1, const Shape& s2 )
{
  TestResult r = s1.IntersectsArea( s2 );
  if( r == undetermined )
    r = s2.IntersectsArea( s1 );
  return r == true_;
}

// RectangularShape
Shape::TestResult
RectangularShape::Contains( const GUI::Point& p ) const
{
  return PointInRect( p, ObjectRect() ) ? true_ : false_;
}

Shape::TestResult
RectangularShape::IntersectsArea( const Shape& s ) const
{
  TestResult result = undetermined;
  const RectangularShape* pRect = dynamic_cast<const RectangularShape*>( &s );
  if( pRect != NULL )
  {
    GUI::Rect r1 = pRect->ObjectRect(),
              r2 = this->ObjectRect(),
              ov;
    ov.left = max( r1.left, r2.left );
    ov.right = min( r1.right, r2.right );
    ov.top = max( r1.top, r2.top );
    ov.bottom = min( r1.bottom, r2.bottom );
    result = EmptyRect( ov ) ? false_ : true_;
  }
  return result;
}

void
RectangularShape::OnPaint( const GUI::DrawContext& inDC )
{
#ifdef __BORLANDC__
  TCanvas* pCanvas = new TCanvas;
  try
  {
    pCanvas->Handle = ( HDC )inDC.handle;
    TRect winRect( mRect.left, mRect.top, mRect.right, mRect.bottom );
    if( this->FillColor() == RGBColor::NullColor )
    {
      pCanvas->Brush->Style = bsClear;
    }
    else
    {
      pCanvas->Brush->Style = bsSolid;
      pCanvas->Brush->Color = TColor( this->FillColor().ToWinColor() );
    }
    if( this->Color() == RGBColor::NullColor )
    {
      pCanvas->Pen->Style = psClear;
    }
    else
    {
      pCanvas->Pen->Style = psSolid;
      pCanvas->Pen->Color = TColor( this->Color().ToWinColor() );
      pCanvas->Pen->Width = this->LineWidth();
    }
    pCanvas->Rectangle( winRect );
  }
  __finally
  {
    delete pCanvas;
  }
#else // __BORLANDC__
  // Prepare the brush
  QPainter* p = inDC.handle.painter;
  QRect drawRect(
    static_cast<int>( mRect.left ),
    static_cast<int>( mRect.top ),
    static_cast<int>( mRect.right - mRect.left ),
    static_cast<int>( mRect.bottom - mRect.top )
  );
  QBrush fillBrush;
  if( this->FillColor() == RGBColor( RGBColor::NullColor ) )
    fillBrush.setStyle( Qt::NoBrush );
  else
  {
    QColor fillColor( this->FillColor().R(), this->FillColor().G(), this->FillColor().B() );
    fillBrush.setStyle( Qt::SolidPattern );
    fillBrush.setColor( fillColor );
  }
  p->setBrush( fillBrush );

  // Prepare the pen
  QPen outlinePen;
  if( this->Color() == RGBColor( RGBColor::NullColor ) )
    outlinePen.setStyle( Qt::NoPen );
  else
  {
    QColor outlineColor( this->Color().R(), this->Color().G(), this->Color().B() );
    outlinePen.setStyle( Qt::SolidLine );
    outlinePen.setColor( outlineColor );
    outlinePen.setWidth( static_cast<int>( this->LineWidth() ) );
  }
  p->setPen( outlinePen );

  // Draw the rectangle
  p->drawRect( drawRect );
#endif // __BORLANDC__
}

// EllipticShape
Shape::TestResult
EllipticShape::Contains( const GUI::Point& p ) const
{
  TestResult result = false_;
  GUI::Rect r = this->ObjectRect();
  float width = r.right - r.left,
        height = r.bottom - r.top;
  if( width > eps && height > eps )
  {
    GUI::Point c = this->Center();
    float dx = p.x - c.x,
          dy = p.y - c.y;
    if( dx * dx * 4 / width / width + dy * dy * 4 / height / height <= 1 )
      result = true_;
  }
  return result;
}

Shape::TestResult
EllipticShape::IntersectsArea( const Shape& s ) const
{
  TestResult result = undetermined;
  const EllipticShape* pEll = dynamic_cast<const EllipticShape*>( &s );
  if( pEll != NULL )
  {
    GUI::Point c1 = pEll->Center(),
               c2 = this->Center();
    if( this->Contains( c1 ) || pEll->Contains( c2 ) )
    {
      result = true_;
    }
    else
    {
      GUI::Rect r1 = pEll->ObjectRect(),
                r2 = this->ObjectRect();
      float width1 = r1.right - r1.left,
            width2 = r2.right - r2.left,
            height1 = r1.bottom - r1.top,
            height2 = r2.bottom - r2.top,
            dx = c1.x - c2.x,
            dy = c1.y - c2.y;
      if( width1 < eps || width2 < eps || height1 < eps || height2 < eps )
      {
        result = false_;
      }
      else if( width1 * height2 == width2 * height1 )
      {
        if( dx * dx + dy * dy * width1 * width1 / height1 / height1 <= width1 * width1 + width2 * width2 )
          result = true_;
        else
          result = false_;
      } // Remaining cases (ellipses with different axes ratios) require finding
        // roots of a quartic polynomial, so we leave them undecided for now.
    }
  }
  const RectangularShape* pRect = dynamic_cast<const RectangularShape*>( &s );
  if( pRect != NULL )
  {
    result = false_;
    GUI::Point c = this->Center();
    if( pRect->Contains( c ) )
    { // Degenerate case
      result = true_;
    }
    else
    { // For each of the rectangle's line segments, test whether the point
      // closest to the ellipse's center is inside the ellipse.
      // The ellipse's axes are aligned with the rectangle, so the closest
      // points are easy to find.
      GUI::Rect r = pRect->ObjectRect();
      if( c.x > r.left && c.x <= r.right )
      {
        GUI::Point p1 = { c.x, r.top },
                   p2 = { c.x, r.bottom };
        if( this->Contains( p1 ) || this->Contains( p2 ) )
          result = true_;
      }
      if( result != true_ && c.y > r.top && c.y <= r.bottom )
      {
        GUI::Point p3 = { r.left, c.y },
                   p4 = { r.right, c.y };
        if( this->Contains( p3 ) || this->Contains( p4 ) )
          result = true_;
      }
      if( result != true_ )
      { // For each of the rectangle's vertices, test whether it is inside the
        // ellipse.
        GUI::Point v1 = { r.left, r.top },
                   v2 = { r.left, r.bottom },
                   v3 = { r.right, r.top },
                   v4 = { r.right, r.bottom };
        if( this->Contains( v1 ) || this->Contains( v2 )
            || this->Contains( v3 ) || this->Contains( v4 ) )
          result = true_;
      }
    }
  }
  return result;
}

void
EllipticShape::OnPaint( const GUI::DrawContext& inDC )
{
#ifdef __BORLANDC__
  TCanvas* pCanvas = new TCanvas;
  try
  {
    pCanvas->Handle = ( HDC )inDC.handle;
    TRect winRect( mRect.left, mRect.top, mRect.right, mRect.bottom );
    if( this->FillColor() == RGBColor::NullColor )
    {
      pCanvas->Brush->Style = bsClear;
    }
    else
    {
      pCanvas->Brush->Style = bsSolid;
      pCanvas->Brush->Color = TColor( this->FillColor().ToWinColor() );
    }
    if( this->Color() == RGBColor::NullColor )
    {
      pCanvas->Pen->Style = psClear;
    }
    else
    {
      pCanvas->Pen->Style = psSolid;
      pCanvas->Pen->Color = TColor( this->Color().ToWinColor() );
      pCanvas->Pen->Width = this->LineWidth();
    }
    pCanvas->Ellipse( winRect );
  }
  __finally
  {
    delete pCanvas;
  }
#else // __BORLANDC__
  // Prepare the Brush
  QPainter* p = inDC.handle.painter;
  QRect drawRect(
    static_cast<int>( mRect.left ),
    static_cast<int>( mRect.top ),
    static_cast<int>( mRect.right - mRect.left ),
    static_cast<int>( mRect.bottom - mRect.top )
  );
  QBrush fillBrush;
  if( this->FillColor() == RGBColor( RGBColor::NullColor ) )
    fillBrush.setStyle( Qt::NoBrush );
  else
  {
    QColor fillColor( this->FillColor().R(), this->FillColor().G(), this->FillColor().B() );
    fillBrush.setStyle( Qt::SolidPattern );
    fillBrush.setColor( fillColor );
  }
  p->setBrush( fillBrush );

  // Prepare the Pen
  QPen outlinePen;
  if( this->Color() == RGBColor( RGBColor::NullColor ) )
    outlinePen.setStyle( Qt::NoPen );
  else
  {
    QColor outlineColor( this->Color().R(), this->Color().G(), this->Color().B() );
    outlinePen.setStyle( Qt::SolidLine );
    outlinePen.setColor( outlineColor );
    outlinePen.setWidth( static_cast<int>( this->LineWidth() ) );
  }
  p->setPen( outlinePen );

  // Draw the rectangle
  p->drawEllipse( drawRect );
#endif // __BORLANDC__
}

