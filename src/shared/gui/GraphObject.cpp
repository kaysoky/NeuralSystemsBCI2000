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
#include "PCHIncludes.h"
#pragma hdrstop

#include "GraphObject.h"
#include "GraphDisplay.h"

using namespace GUI;

static Rect sNullRect = { 0, 0, 0, 0 };

GraphObject::GraphObject( GraphDisplay& display, float zOrder )
: mDisplay( display ),
  mVisible( true ),
  mRectSet( false ),
  mZOrder( zOrder ),
  mAspectRatioMode( AspectRatioModes::AdjustNone )
{
  mUserSpecifiedDisplayRect = sNullRect;
  mActualDisplayRect = sNullRect;
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
  mDisplay.InvalidateRect( mActualDisplayRect );
  return *this;
}

GraphObject&
GraphObject::SetDisplayRect( const GUI::Rect& inRect )
{
  if( mUserSpecifiedDisplayRect != inRect )
  {
    int changedFlags = Position;
    bool resized = mUserSpecifiedDisplayRect.Width() != inRect.Width()
                 || mUserSpecifiedDisplayRect.Height() != inRect.Height();
    if( resized )
      changedFlags |= Size;

    Invalidate();
    mUserSpecifiedDisplayRect = inRect;
    mActualDisplayRect = sNullRect;
    mRectSet = true;
    Change( changedFlags );
  }
  return *this;
}

GUI::Rect
GraphObject::DisplayRect() const
{
  return mDisplay.PixelToNormalizedCoords( mActualDisplayRect );
}

void
GraphObject::Paint()
{
  if( mRectSet && mVisible )
  {
    DrawContext dc =
    {
      mDisplay.Context().handle,
      mActualDisplayRect
    };
#ifndef __BORLANDC__
    dc.handle.painter->save();
#endif // __BORLANDC__
    OnPaint( dc );
#ifndef __BORLANDC__
    dc.handle.painter->restore();
#endif // __BORLANDC__
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
      mDisplay.NormalizedToPixelCoords( mUserSpecifiedDisplayRect )
    };
    int considered = 0;

    if( inWhich & Position )
      OnMove( dc );
    considered |= Position;

    if( inWhich & Size )
      OnResize( dc );
    considered |= Size;

    if( inWhich & ~considered )
      OnChange( dc );

    mActualDisplayRect = dc.rect;
    Invalidate();
  }
}

bool
GraphObject::Click( const Point& p )
{
  Rect r = mDisplay.PixelToNormalizedCoords( mActualDisplayRect );
  return PointInRect( p, r ) && OnClick( p );
}

