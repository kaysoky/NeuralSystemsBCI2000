////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: This "threeDText" class resembles the 3D text in the 3D
//   environment.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "threeDText.h"
#include "buffers.h"

using namespace std;

void
threeDText::onInitialize()
{
  mFontID = buffers::loadFont3D( mFont );
  sceneObj::onInitialize();
}

void
threeDText::onCleanup()
{
  buffers::releaseFont3D( mFont );
  mFontID = NULL;
  sceneObj::onCleanup();
}

void
threeDText::onRender() const
{
  if( !mCaption.empty() )
  {  //print the 3D text
    const GLYPHMETRICSFLOAT* gmf = buffers::getFontData3D(mFontID);
    if( gmf != NULL )
    {
      GLfloat height = gmf['X'].gmfBlackBoxY,
              width = 0;
      //resize the font
      GLfloat ratio = mFontSize / height;
      glScalef( ratio, ratio, ratio );
      for( size_t i = 0; i < mCaption.length(); ++i )
        width += gmf[ mCaption[i] ].gmfCellIncX;
      // Center our text on the screen
      glTranslatef( -width/2, -height/2, 0 );

      glPushAttrib( GL_LIST_BIT );  // Pushes the display list bits
      glListBase( mFontID );        // Sets the base character to 0
      glCallLists( mCaption.length(), GL_UNSIGNED_BYTE, mCaption.data() ); // Draws the display list text
      glPopAttrib();                // Pops the display list bits
    }
  }
}


