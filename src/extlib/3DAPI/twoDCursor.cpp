///////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: A 2D cursor displayed on top of the 2D overlay.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "twoDCursor.h"
#include <cmath>

using namespace std;

const cNumVertices = 35;

void
twoDCursor::onRender() const
{
  glDisable( GL_TEXTURE_2D );
  glTranslatef( mOrigin.x, mOrigin.y, 0 );
  glColor3f( mRColor, mGColor, mBColor );
  glBegin( GL_POLYGON );
  for( int i = 0; i < cNumVertices; ++i )
  {
      float angle = i * 2 * M_PI / cNumVertices,
            x = mRadius * ::cos( angle ),
            y = mRadius * ::sin( angle );
      glVertex2f( x, y );
  }//for
  glEnd();
}


