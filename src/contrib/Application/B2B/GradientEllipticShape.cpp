////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GradientEllipticShape.h"

#ifdef __BORLANDC__
# include <VCL.h>
#else // __BORLANDC__
# include <QPainter>
#endif // __BORLANDC__

using namespace std;
using namespace GUI;

  //EllipticShape( GUI::GraphDisplay& display, int zOrder = ShapeZOrder )
  //  : Shape( display, zOrder )
  //  {}

  //~EllipticShape()
  //  {}

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

// protected:
//  TestResult IntersectsArea( const Shape& ) const;

// protected:
  // GraphObject event handlers

void
GradientEllipticShape::OnPaint( const GUI::DrawContext& inDC )
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
	//QRadialGradient gradient(0, 0, 8, 8, 8);
 //    gradient.setColorAt(0, QColor::fromRgbF(0, 1, 0, 1));
 //    gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));

  float r  = (mRect.right-mRect.left)/2;
  float cx = mRect.left + r;
  float cy = mRect.top  + r; // assumes a circle, not ellipse
  
  QRadialGradient fillGradient(cx, cy, r, cx, cy);


  if( this->FillColor() == RGBColor( RGBColor::NullColor ) ) {
    //fillBrush.setStyle( Qt::NoBrush );
  }
  else
  {
    QColor fillColor( this->FillColor().R(), this->FillColor().G(), this->FillColor().B() );
// //   /* QBrush brush(gradient);*/
//
////	 fillBrush.set
//	//QRadialGradient radialGrad(QPointF(100, 100), 100);
//  //   radialGrad.setColorAt(0, Qt::red);
//  //   radialGrad.setColorAt(0.5, Qt::blue);
//  //   radialGrad.setColorAt(1, Qt::green);
//
//    fillBrush.setStyle( Qt::CrossPattern ); //change from solid Pattern to Radial Gradient
	fillGradient.setColorAt(0, fillColor);
	fillGradient.setColorAt(1, Qt::black); // TODO, this should be set to the background color
//    fillBrush.setColor( fillColor );
  }

  QBrush fillBrush(fillGradient);
  p->setBrush( fillBrush );

  // Prepare the Pen
  QPen outlinePen;
  //if( this->Color() == RGBColor( RGBColor::NullColor ) )
  //  outlinePen.setStyle( Qt::NoPen );
  //else
  //{
  //  QColor outlineColor( this->Color().R(), this->Color().G(), this->Color().B() );
  //  outlinePen.setStyle( Qt::SolidLine );
  //  outlinePen.setColor( outlineColor );
  //  outlinePen.setWidth( static_cast<int>( this->LineWidth() ) );
  //}
  outlinePen.setStyle( Qt::NoPen );
  p->setPen( outlinePen );

  // Draw the rectangle
  p->drawEllipse( drawRect );
#endif // __BORLANDC__
}
