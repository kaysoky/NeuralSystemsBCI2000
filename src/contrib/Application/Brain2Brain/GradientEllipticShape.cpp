#include "PCHIncludes.h"
#pragma hdrstop

#include "GradientEllipticShape.h"
#include <QPainter>

using namespace std;
using namespace GUI;

Shape::TestResult
	  GradientEllipticShape::Contains( const GUI::Point& point) const
  {
	  return EllipticShape::Contains(point);
  }

Shape::TestResult
	  GradientEllipticShape::IntersectsArea (const Shape& shape ) const
  {
	  return EllipticShape::IntersectsArea(shape);
  }

void
GradientEllipticShape::OnPaint( const GUI::DrawContext& inDC )
{
  // Prepare the Brush
  QPainter* p = inDC.handle.painter;
  QRect drawRect(
    static_cast<int>( Rect().left ),
    static_cast<int>( Rect().top ),
    static_cast<int>( Rect().right - Rect().left ),
    static_cast<int>( Rect().bottom - Rect().top )
  );

  float r  = (Rect().right-Rect().left)/2;
  float cx = Rect().left + r;
  float cy = Rect().top  + r; // assumes a circle, not ellipse
  
  QRadialGradient fillGradient(cx, cy, r, cx, cy);


  if( this->FillColor() == RGBColor( RGBColor::NullColor ) ) {
    //fillBrush.setStyle( Qt::NoBrush );
  }
  else
  {
    QColor fillColor( this->FillColor().R(), this->FillColor().G(), this->FillColor().B() );
	fillGradient.setColorAt(0, fillColor);
	fillGradient.setColorAt(1, Qt::black); // TODO, this should be set to the background color
  }

  QBrush fillBrush(fillGradient);
  p->setBrush( fillBrush );

  // Prepare the Pen
  QPen outlinePen;
  outlinePen.setStyle( Qt::NoPen );
  p->setPen( outlinePen );

  // Draw the rectangle
  p->drawEllipse( drawRect );
}
