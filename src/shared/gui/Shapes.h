////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A number of GraphObjects representing geometric shapes.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SHAPES_H
#define SHAPES_H

#include "GraphObject.h"

#include "Color.h"

class Shape : public GUI::GraphObject
{
 public:
  Shape( GUI::GraphDisplay& display, int zOrder = ShapeZOrder );
  virtual ~Shape();
  // Properties
  Shape&     SetCenter( const GUI::Point& );
  GUI::Point Center() const;
  Shape&     SetLineWidth( float );
  float      LineWidth() const;
  Shape&     SetFillColor( RGBColor );
  RGBColor   FillColor() const;
  Shape&     SetColor( RGBColor );
  RGBColor   Color() const;

  // Intersection testing
 public:
  enum TestResult { false_ = 0, true_ = 1, undetermined };
  virtual TestResult Contains( const GUI::Point& ) const
        { return undetermined; };

 protected:
  virtual TestResult IntersectsArea( const Shape& ) const
        { return undetermined; };

 public:
  static bool AreaIntersection( const Shape&, const Shape& );


 protected:
  // GraphObject event handlers
  virtual void OnPaint( const GUI::DrawContext& ) = 0;

 private:
  float       mLineWidth;
  RGBColor    mColor,
              mFillColor;
};

class RectangularShape : public Shape
{
 public:
  RectangularShape( GUI::GraphDisplay& display, int zOrder = ShapeZOrder )
    : Shape( display, zOrder )
    {}
  virtual ~RectangularShape()
    {}
  virtual TestResult Contains( const GUI::Point& ) const;

 protected:
  virtual TestResult IntersectsArea( const Shape& ) const;

 protected:
  // GraphObject event handlers
  virtual void OnPaint( const GUI::DrawContext& );
};

class EllipticShape : public Shape
{
 public:
  EllipticShape( GUI::GraphDisplay& display, int zOrder = ShapeZOrder )
    : Shape( display, zOrder )
    {}
  virtual ~EllipticShape()
    {}
  virtual TestResult Contains( const GUI::Point& ) const;

 protected:
  virtual TestResult IntersectsArea( const Shape& ) const;

 protected:
  // GraphObject event handlers
  virtual void OnPaint( const GUI::DrawContext& );
};

#endif // SHAPES_H

