////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents a display rectangle for a set of
//   GraphObjects.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GRAPH_DISPLAY_H
#define GRAPH_DISPLAY_H

#include "GraphObject.h"

#include "Color.h"
#include <set>
#include <queue>
#ifdef __BORLANDC__
# include "VCL.h"
#endif // __BORLANDC__

class BitmapImage;

namespace GUI {

typedef std::set<GraphObject*, GraphObject::CompareByZOrder> SetOfGraphObjects;
typedef std::queue<GraphObject*> QueueOfGraphObjects;

class GraphDisplay
{
 public:
  GraphDisplay();
  virtual ~GraphDisplay()
    { DeleteObjects(); }
  // Properties
  GraphDisplay& SetContext( const DrawContext& dc )
    { mContext = dc; Change(); return *this; }
  const GUI::DrawContext& Context() const
    { return mContext; }
  GraphDisplay& SetColor( RGBColor c )
    { mColor = c; return Invalidate(); }
  RGBColor Color() const
    { return mColor; }
  GraphDisplay& ClearClicks()
    { while( !mObjectsClicked.empty() ) mObjectsClicked.pop(); return *this; }
  QueueOfGraphObjects& ObjectsClicked()
    { return mObjectsClicked; }
  // Management of GraphObjects
  GraphDisplay& Add( GraphObject* obj )
    { mObjects.insert( obj ); mNeedReorder = true; return *this; }
  GraphDisplay& Remove( GraphObject* obj )
    { obj->Invalidate(); mObjects.erase( obj ); mNeedReorder = true; return *this; }
  GraphDisplay& DeleteObjects()
    { while( !mObjects.empty() ) delete *mObjects.begin(); return *this; }
  // Read bitmap data, resampled to target resolution
  const BitmapImage& BitmapData( int width = 0, int height = 0 ) const;
  // Graphics functions
  //  Invalidate the display's entire area
  GraphDisplay& Invalidate() const;
  //  Invalidate a rectangle given in normalized coordinates
  GraphDisplay& InvalidateRect( const Rect& ) const;
  //  Force immediate (i.e., synchronous) redrawing of invalidated window areas
  GraphDisplay& Update() const;
  // Events
  void Paint( void* RegionHandle = NULL );
  void Change();
  void Click( int x, int y );

  // Coordinate conversion for object drawing
  Rect NormalizedToPixelCoords( const Rect& ) const;
  Rect PixelToNormalizedCoords( const Rect& ) const;

 private:
  void Reorder();

  DrawContext         mContext;
  RGBColor            mColor;
  SetOfGraphObjects   mObjects;
  bool                mNeedReorder;
  QueueOfGraphObjects mObjectsClicked;

#ifdef _WIN32
  void ClearOffscreenBuffer();

  HDC                 mOffscreenDC;
  HBITMAP             mOffscreenBmp;
#endif // _WIN32
};

} // namespace GUI

#endif // GRAPH_DISPLAY_H
