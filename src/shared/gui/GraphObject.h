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
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
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
 class CompareByZOrder;
 friend class GraphObject::CompareByZOrder;

 protected:
  enum
  { // Top to bottom
    MessageZOrder,
    StatusBarZOrder,
    TextStimulusZOrder,
    ImageStimulusZOrder,
    ShapeZOrder,
    SceneDisplayZOrder,
  };

 public:
  GraphObject( GraphDisplay&, int zOrder );
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
  GraphObject& SetAspectRatioMode( int m )
    { Invalidate(); mAspectRatioMode = m; Change(); return *this; }
  int AspectRatioMode() const
    { return mAspectRatioMode; }
  GraphObject& SetDisplayRect( const GUI::Rect& );
  const GUI::Rect& DisplayRect() const
    { return mDisplayRect; }

  // Graphics
  GraphObject& Invalidate();

  // Events
  //  Calling side
  void Paint();
  void Change();
  bool Click( const Point& p )
    { return PointInRect( p, mDisplayRect ) && OnClick( p ); }

 protected:
  //  Handling side
  //  This function implements drawing the object.
  virtual void OnPaint( const DrawContext& ) = 0;
  //  This function is called when any change of properties occurs.
  //  For AspectRatioModes that adapt the object's enclosing rectangle,
  //  OnChange must change the DrawContext's rectangle to reflect the adaptation.
  virtual void OnChange( DrawContext& )
    {}
  //  The OnClick event handler receives a point in continuous coordinates, and
  //  returns whether it considers itself clicked.
  virtual bool OnClick( const Point& )
    { return true; }

 public:
  // Sort order for drawing (smaller values correspond to top)
  struct CompareByZOrder
  { bool operator()( const GraphObject* s1, const GraphObject* s2 )
    { return ( s1->mZOrder == s2->mZOrder ) ? s1 > s2 : s1->mZOrder > s2->mZOrder; }
  };

 private:
  bool          mVisible,
                mRectSet;
  int           mZOrder,
                mAspectRatioMode;
  Rect          mDisplayRect;
  GraphDisplay& mDisplay;
};

} // namespace GUI

#endif // GRAPH_OBJECT_H
