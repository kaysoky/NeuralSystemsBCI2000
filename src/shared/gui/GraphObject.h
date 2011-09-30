////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class for graphical objects. GraphObjects are tied to a
//   GraphDisplay which, in turn, is linked to a rectangular area of an OS
//   drawing surface.
//   GraphObjects provide the following event handlers:
//     OnChange: A change in size or parameters has occurred, bitmap buffers
//       should be adapted to the new parameters.
//     OnPaint:  The object renders itself into the provided DrawContext.
//     OnClick:  The user clicked the area occupied by the object. The object
//       considers itself clicked when it returns true (the default).
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
#ifndef GRAPH_OBJECT_H
#define GRAPH_OBJECT_H

#include "GUI.h"

namespace GUI
{

 namespace AspectRatioModes
 {
   enum
   {
     AdjustNone,
     AdjustWidth,
     AdjustHeight,
     AdjustBoth,
   };
 }

// Forward declarations
class GraphDisplay;

class GraphObject
{
 public:
  struct CompareByZOrder;
  friend struct GraphObject::CompareByZOrder;

  enum
  { // Top to bottom
    MessageZOrder,
    StatusBarZOrder,
    TextStimulusZOrder,
    ImageStimulusZOrder,
    ShapeZOrder,
    SceneDisplayZOrder,
  };

 private:
  enum
  { // Flags for properties that may have changed
    All = -1,
    Position = 1 << 0,
    Size =     1 << 1,
  };

 public:
  GraphObject( GraphDisplay&, float zOrder );
  virtual ~GraphObject();

  // Properties
 public:
  const GraphDisplay& Display() const
    { return mDisplay; }
  GraphObject& Hide()
    { if( mVisible ) Invalidate(); mVisible = false; return *this; }
  GraphObject& Show()
    { if( !mVisible ) Invalidate(); mVisible = true; return *this; }
  bool Visible() const
    { return mVisible; }
  GraphObject& SetZOrder( float z )
    { Invalidate(); mZOrder = z; return *this; }
  float ZOrder() const
    { return mZOrder; }
  GraphObject& SetAspectRatioMode( int m )
    { Invalidate(); mAspectRatioMode = m; Change(); return *this; }
  int AspectRatioMode() const
    { return mAspectRatioMode; }
  GraphObject& SetDisplayRect( const GUI::Rect& );
  const GUI::Rect& DisplayRect() const
    { return mActualDisplayRect; }

  virtual bool NeedsGL() const
    { return false; }

  // Graphics
  GraphObject& Invalidate();

  // Events
  //  Calling side
  void Paint();
  void Change( int which = All );
  bool Click( const Point& p )
    { return PointInRect( p, mActualDisplayRect ) && OnClick( p ); }

 protected:
  //  Handling side
  //  This function implements drawing the object.
  virtual void OnPaint( const DrawContext& ) = 0;
  //  This function is called when a change of properties other than position
  //  or size occurs.
  virtual void OnChange( DrawContext& )
    {}
  //  These functions are called when position or size has changed.
  //  They call OnChange() by default, so you need not implement them.
  //  For AspectRatioModes that adapt the object's enclosing rectangle,
  //  OnMove() and OnResize() must change the DrawContext's rectangle to reflect adaptation.
  virtual void OnMove( DrawContext& dc )
    { OnChange( dc ); }
  virtual void OnResize( DrawContext& dc )
    { OnChange( dc ); }
  //  The OnClick event handler receives a point in continuous coordinates, and
  //  returns whether it considers itself clicked.
  virtual bool OnClick( const Point& )
    { return true; }

#ifndef __BORLANDC__
  // Access to OpenGL context from GraphObject descendants.
  class QGLWidget* GLContext() const;
#endif // __BORLANDC__

 public:
  // Sort order for drawing (smaller values correspond to top)
  struct CompareByZOrder
  { bool operator()( const GraphObject* s1, const GraphObject* s2 ) const
    { return ( s1->mZOrder == s2->mZOrder ) ? s1 > s2 : s1->mZOrder > s2->mZOrder; }
  };

 private:
  GraphDisplay& mDisplay;
  bool          mVisible,
                mRectSet;
  float         mZOrder;
  int           mAspectRatioMode;
  Rect          mUserSpecifiedDisplayRect,
                mActualDisplayRect;
};

} // namespace GUI

#endif // GRAPH_OBJECT_H
