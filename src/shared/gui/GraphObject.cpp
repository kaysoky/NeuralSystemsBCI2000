////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
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

#include "GraphObject.h"
#include "GraphDisplay.h"
#include <algorithm>

using namespace GUI;

static GUI::Rect sNullRect = { 0, 0, 0, 0 };

static void Normalize( GUI::Rect& ioRect )
{
  if( ioRect.Width() < 0 )
    std::swap( ioRect.right, ioRect.left );
  if( ioRect.Height() < 0 )
    std::swap( ioRect.top, ioRect.bottom );
}


GraphObject::GraphObject( GraphDisplay& display, float zOrder )
: mDisplay( display ),
  mVisible( true ),
  mRectSet( false ),
  mZOrder( zOrder ),
  mAspectRatioMode( AspectRatioModes::AdjustNone )
{
  mObjectRect = sNullRect;
  mBoundingRect = sNullRect;
  mDisplay.Add( this );
  Invalidate();
}

GraphObject::~GraphObject()
{
  Invalidate();
  mDisplay.Remove( this );
}

GraphObject&
GraphObject::Invalidate()
{
  mDisplay.InvalidateRect( mBoundingRect );
  return *this;
}

GraphObject&
GraphObject::SetObjectRect( const GUI::Rect& inRect )
{
  GUI::Rect r = inRect;
  Normalize( r );
  if( mObjectRect != r )
  {
    int changedFlags = Position;
    bool resized = mObjectRect.Width() != r.Width()
                 || mObjectRect.Height() != r.Height();
    if( resized )
      changedFlags |= Size;

    Invalidate();
    mObjectRect = r;
    mBoundingRect = mDisplay.NormalizedToPixelCoords( r );
    mRectSet = true;
    Change( changedFlags );
  }
  return *this;
}

GUI::Rect
GraphObject::BoundingRect() const
{
  return mDisplay.PixelToNormalizedCoords( mBoundingRect );
}

void
GraphObject::Paint()
{
  if( mRectSet && mVisible )
  {
    DrawContext dc =
    {
      mDisplay.Context().handle,
      { 0, 0, 0, 0 }
    };
    dc.rect = mBoundingRect;
    SaveDC( dc );
    OnPaint( dc );
    RestoreDC( dc );
  }
}

void
GraphObject::Change( int inWhich )
{
  if( mRectSet )
  {
    Invalidate();
    DrawContext dc =
    {
      mDisplay.Context().handle,
      { 0, 0, 0, 0 }
    };
    dc.rect = mDisplay.NormalizedToPixelCoords( mObjectRect );

    int considered = 0;

    if( inWhich & Position )
      OnMove( dc );
    considered |= Position;

    if( inWhich & Size )
      OnResize( dc );
    considered |= Size;

    if( inWhich & ~considered )
      OnChange( dc );

    mBoundingRect = dc.rect;
    Normalize( mBoundingRect );
    Invalidate();
  }
}

bool
GraphObject::Click( const GUI::Point& p )
{
  Rect r = mDisplay.PixelToNormalizedCoords( mBoundingRect );
  return PointInRect( p, r ) && OnClick( p );
}

void
GraphObject::SaveDC( DrawContext& ioDC )
{
#if USE_QT
  ioDC.handle.painter->save();
#endif
}

void
GraphObject::RestoreDC( DrawContext& ioDC )
{
#if USE_QT
  ioDC.handle.painter->restore();
#endif
}

