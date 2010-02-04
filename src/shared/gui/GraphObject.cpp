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
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GraphObject.h"
#include "GraphDisplay.h"

using namespace GUI;

GraphObject::GraphObject( GraphDisplay& display, int zOrder )
: mDisplay( display ),
  mRectSet( false ),
  mVisible( true ),
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
    OnPaint( dc );
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


