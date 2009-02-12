///////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: This "twoDOverlay" class resembles the 2D overlay on top of the
//   3D environment.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "twoDOverlay.h"
#include "glheaders.h"
#include "buffers.h"

void
twoDOverlay::onInitialize()
{
  mPictureTexture = buffers::loadTexture( mPictureFile );
  mTranslucencyTexture = buffers::loadTexture( mTranslucencyFile );
  overlayObj::onInitialize();
}

void
twoDOverlay::onCleanup()
{
  buffers::releaseTexture( mPictureFile );
  mPictureTexture = NULL;
  buffers::releaseTexture( mTranslucencyFile );
  mTranslucencyTexture = NULL;
  overlayObj::onCleanup();
}

void
twoDOverlay::drawQuad( const int* inRect )
{
  glBegin( GL_QUADS );
  // Display the top left point of the 2D image
  glTexCoord2f( 0, 1 );
  glVertex2f( inRect[0], inRect[1] );
  // Display the bottom left point of the 2D image
  glTexCoord2f( 0, 0 );
  glVertex2f( inRect[0], inRect[1] + inRect[3] );
  // Display the bottom right point of the 2D image
  glTexCoord2f( 1, 0 );
  glVertex2f( inRect[0] + inRect[2], inRect[1] + inRect[3] );
  // Display the top right point of the 2D image
  glTexCoord2f( 1, 1 );
  glVertex2f( inRect[0] + inRect[2], inRect[1] );
  // Stop drawing
  glEnd();
}

void
twoDOverlay::onRender() const
{
  int viewPort[4] = { -1, -1, -1, -1 };
  // Push the current projection matrix
  glGetIntegerv( GL_VIEWPORT, viewPort );
  if( mTranslucencyTexture )
  {
    glColor4f( 1, 1, 1, 1 );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glBlendFunc( GL_ZERO, GL_SRC_COLOR );
    //Bind the mask texture to our new 2D quad
    glBindTexture( GL_TEXTURE_2D, mTranslucencyTexture );
    // Display a 2D quad with the translucency mask
    drawQuad( viewPort );
  }
  if( mPictureTexture )
  {
    glColor4f( mBrightness, mBrightness, mBrightness, 1 );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glBlendFunc( GL_ONE, GL_ONE );
    // Bind the scope texture to this next 2D quad
    glBindTexture( GL_TEXTURE_2D, mPictureTexture );
    // Display a 2D quad with our scope texture
    drawQuad( viewPort );
  }
  glDisable( GL_BLEND );
}


