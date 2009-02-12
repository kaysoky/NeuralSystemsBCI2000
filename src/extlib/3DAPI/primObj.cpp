////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: primObj is a base class for all objects in the 3D environment.
//   Two classes are derived from primObj:
//     sceneObj representing 3D scene objects, and
//     overlayObj representing objects in the 2D overlay.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "glheaders.h"
#include "primObj.h"
#include "buffers.h"
#include "Scene.h"

using namespace std;

primObj::primObj( Scene& inScene )
: mScene( inScene ),
  mVisible( true ),
  mDrawingOrder( 0 )
{
  mScene.Add( this );
}

primObj::~primObj()
{
  mScene.Remove( this );
}


void
overlayObj::render() const
{
  if( getVisible() )
  {
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
    glBindTexture( GL_TEXTURE_2D, NULL );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    int viewPort[4] = { -1, -1, -1, -1 };
    glGetIntegerv( GL_VIEWPORT, viewPort );
    glOrtho( 0, viewPort[2], 0, viewPort[3], -1, 1 );
    glMatrixMode( GL_MODELVIEW );

    onRender();

    glEnable( GL_LIGHTING );
    glEnable( GL_DEPTH_TEST );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
  }
}


sceneObj::sceneObj( Scene& inScene )
: primObj( inScene ),
  mBrightness(1),
  mTransparency(0),
  mTextureHandle(NULL),
  mRColor(1),
  mBColor(1),
  mGColor(1)
{
}


// Rendering
void
sceneObj::render() const
{
  if( getVisible() )
  {
    // apply world transform
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glMultMatrixf( getWorldTransform() );
    glEnable( GL_NORMALIZE );

    // bind texture
    if( mTextureHandle != NULL )  //check to see if texture date loaded successfully
    {
      glEnable( GL_TEXTURE_2D );
      glBindTexture( GL_TEXTURE_2D, mTextureHandle );
    }
    else
    {
      glDisable( GL_TEXTURE_2D );
    }

    //transparency and brightness
    GLfloat b = clamp( getBrightness() ),
            t = clamp( getTransparency() );

    glColor4f( mRColor * b, mGColor * b, mBColor * b, 1 - t );
    glEnable( GL_BLEND ); // Turn Blending On
    // Blending Function For Translucency Based On Source Alpha Value
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    onRender();

    //disable the transparency
    glDisable( GL_BLEND );      // Turn Blending Off
    glDisable( GL_TEXTURE_2D ); // Turn the 2D texture off
    // restore previous transformation matrix
    glPopMatrix();
  }
}

void
sceneObj::onInitialize()
{
  mTextureHandle = buffers::loadTexture(mTextureFile);
}

void
sceneObj::onCleanup()
{
  mTextureHandle = NULL;
  buffers::releaseTexture(mTextureFile);
}

