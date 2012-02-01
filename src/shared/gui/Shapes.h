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

  // For correcting the invalidation rect
  virtual void OnMove( GUI::DrawContext& ioDC );

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
  GUI::Rect   mRect;

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

