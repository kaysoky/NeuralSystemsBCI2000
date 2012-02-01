////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that represents a display rectangle for a set of
//   GraphObjects.
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
#ifndef GRAPH_DISPLAY_H
#define GRAPH_DISPLAY_H

#include "GraphObject.h"
#include "Uncopyable.h"

#include "Color.h"
#include <set>
#include <list>
#ifdef _WIN32
# include "windows.h"
#endif
#ifdef __BORLANDC__
# include "VCL.h"
#else // __BORLANDC__
# include <QWidget>
#endif // __BORLANDC__

class BitmapImage;

namespace GUI {

#ifndef __BORLANDC__
  class WidgetBase;
#endif // __BORLANDC__

typedef std::set<GraphObject*>   SetOfGraphObjects;
struct QueueOfGraphObjects : public std::list<GraphObject*>
{
  void pop()
    { pop_front(); }
  void push( GraphObject* p )
    { push_back( p ); }
};

class GraphDisplay : private Uncopyable
{
  friend class GraphObject;

 public:
  GraphDisplay();
  virtual ~GraphDisplay();

  // Properties
  GraphDisplay& SetContext( const DrawContext& dc )
    { mContext = dc; Change(); return *this; }
  const GUI::DrawContext& Context() const
    { return mContext; }
  GraphDisplay& SetColor( RGBColor c )
    { mColor = c; Invalidate(); return *this; }
  RGBColor Color() const
    { return mColor; }
  GraphDisplay& ClearClicks()
    { while( !mObjectsClicked.empty() ) mObjectsClicked.pop(); return *this; }
  QueueOfGraphObjects& ObjectsClicked()
    { return mObjectsClicked; }

  // Management of GraphObjects
 private:
  // Add() and Remove() are only provided for GraphObject self registering
  // and unregistering in the GraphObject constructor and destructor.
  // Calling them from elsewhere will lead to inconsistency between
  // a GraphObject's display reference, and the display it is attached to.
  GraphDisplay& Add( GraphObject* obj );
  GraphDisplay& Remove( GraphObject* );

public:
  GraphDisplay& DeleteObjects()
    { while( !mObjects.empty() ) delete *mObjects.begin(); return *this; }

  // Read bitmap data, resampled to target resolution
  BitmapImage BitmapData( int width = 0, int height = 0 ) const;

  // Graphics functions
  //  Invalidate the display's entire area
  virtual GraphDisplay& Invalidate();
  //  Invalidate a rectangle given in pixel coordinates
  virtual GraphDisplay& InvalidateRect( const Rect& );
  //  Force immediate (i.e., synchronous) redrawing of invalidated window areas
  virtual const GraphDisplay& Update() const;
  // Events
  void Paint( const void* RegionHandle = NULL );
  void Change();
  void Click( int x, int y );

  // Coordinate conversion for object drawing
  Rect NormalizedToPixelCoords( const Rect& ) const;
  Rect PixelToNormalizedCoords( const Rect& ) const;

 private:
  void ClearOffscreenBuffer();
#if _WIN32
  static void BitmapImageFromHDC( BitmapImage&, HDC, const RECT& );
#endif // _WIN32
#ifndef __BORLANDC__
  static void BitmapImageFromQPixmap( BitmapImage&, const QPixmap& );
#endif // __BORLANDC__

  DrawContext         mContext;
  RGBColor            mColor;
  SetOfGraphObjects   mObjects;
  QueueOfGraphObjects mObjectsClicked;

#ifdef __BORLANDC__
  HDC                 mOffscreenDC;
  HBITMAP             mOffscreenBmp;
#else // __BORLANDC__
  friend class GUI::WidgetBase;
  QPixmap* mOffscreenBmp;
  QRegion  mInvalidRegion;
  QWidget* mpWidget;
  bool     mUsingGL;
#endif // __BORLANDC__
};

} // namespace GUI

#endif // GRAPH_DISPLAY_H
