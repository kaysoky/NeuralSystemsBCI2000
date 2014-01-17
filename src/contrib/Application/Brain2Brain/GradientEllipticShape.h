#ifndef GRADIENTELLIPTICSHAPE_H
#define GRADIENTELLIPTICSHAPE_H

//#include "GraphObject.h"
#include "Shapes.h"

class GradientEllipticShape : public EllipticShape
{
 //public:
 // enum TestResult { false_ = 0, true_ = 1, undetermined };

 public:
  GradientEllipticShape( GUI::GraphDisplay& display, int zOrder = ShapeZOrder )
    : EllipticShape( display, zOrder )
    {}
  virtual ~GradientEllipticShape()
    {}
  virtual TestResult Contains( const GUI::Point& ) const;


 protected:
  virtual TestResult IntersectsArea( const Shape& ) const;

 protected:
  // GraphObject event handlers
  virtual void OnPaint( const GUI::DrawContext& );
};

#endif // GRADIENTELLIPTICSHAPE_H

