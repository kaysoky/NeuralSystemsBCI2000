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

GraphObject::GraphObject( GraphDisplay& display, int zOrder )
: mDisplay( display ),
  mVisible( true ),
  mRectSet( false ),
  mZOrder( zOrder ),
  mAspectRatioMode( AspectRatioModes::AdjustNone )
{
  Rect rect = { 0, 0, 0, 0 };
  mDisplayRect = rect;
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
  mDisplay.InvalidateRect( mDisplayRect );
  return *this;
}

GraphObject&
GraphObject::SetDisplayRect( const GUI::Rect& inRect )
{
  Invalidate();
  mDisplayRect = inRect;
  mRectSet = true;
  Change();
  return *this;
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
    dc.rect = mDisplay.NormalizedToPixelCoords( mDisplayRect );
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
GraphObject::Change()
{
  if( mRectSet )
  {
    Invalidate();
    DrawContext dc =
    {
      mDisplay.Context().handle,
      { 0, 0, 0, 0 }
    };
    dc.rect = mDisplay.NormalizedToPixelCoords( mDisplayRect );
    OnChange( dc );
    mDisplayRect = mDisplay.PixelToNormalizedCoords( dc.rect );
    Invalidate();
  }
}


